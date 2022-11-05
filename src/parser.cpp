// TODO:     if (l.tok.tok == K_Static_assert) return consume_static_assert();
// TODO:     zero extend: getelementptr
// TODO:     JSON Dumper, Ast Console Dumper, SVG Dumper
struct Type_info {
    CType ty;
    Location loc;
};
struct Label_Info {
  label_t idx;
  uint8_t flags;
  Label_Info(): flags{LBL_UNDEFINED} {}
};
// bit enums for Variable_Info::tags
constexpr uint8_t 
  GARBAGE = 0, // just declared
  USED = 0x1, // true if variable is used
  ASSIGNED = 0x2; // true if assigned or initialized
struct Variable_Info {
    CType ty;
    Location loc;
    llvm::Constant *val;
    unsigned tags: 2;
};
struct Parser : public DiagnosticHelper {
  Lexer l;
  struct Sema { // Semantics processor
    CType currentfunctionRet = nullptr, currentInitTy = nullptr, currentCase = nullptr;
    IdentRef pfunc = nullptr;           // current function name: `__func__`
    FunctionAndBlockScope<Variable_Info, 50> typedefs;
    BlockScope<Type_info, 20> tags;
    SmallVector<Token, 5> tokens_cache;
    uint32_t currentAlign = 0;          // current align(bytes)
    bool type_error = false; // type error
  } sema;
  struct JumpBuilder {
    DenseMap<IdentRef, Label_Info> labels{}; // named labels
    unsigned cur = 0;
    llvm::SmallSet<label_t, 12> used_breaks{};
    label_t topBreak = INVALID_LABEL, topContinue = INVALID_LABEL;
    unsigned createLabel() {
      return cur++;
    }
    Label_Info &lookupLabel(IdentRef Name) {
      return labels.insert(std::make_pair(Name, Label_Info())).first->second;
    }
  } jumper;
  IRGen &irgen;
  Stmt InsertPt = nullptr;
  bool sreachable;
  Stmt unreachable_reason = nullptr;
  Expr intzero, intone, cfalse;
  Parser(SourceMgr &SM, xcc_context &context, IRGen &irgen)
      : DiagnosticHelper{context}, l(SM, *this, context), irgen{irgen},
      intzero {
        wrap(context.getInt(), 
        ConstantInt::get(irgen.ctx, APInt::getZero(context.getInt()->getBitWidth())))
      },
      intone {
        wrap(context.getInt(), 
        ConstantInt::get(irgen.ctx, APInt(context.getInt()->getBitWidth(), 1)))
      },
      cfalse {wrap(context.typecache.b, ConstantInt::getFalse(irgen.ctx))}
      {}
  template <typename T> auto getsizeof(T a) { 
      return irgen.getsizeof(a); 
  }
  template <typename T> auto getAlignof(T a) { 
      return irgen.getAlignof(a); 
  }
  Expr binop(Expr a, BinOp op, Expr b, CType ty) {
    // construct a binary operator
    return ENEW(BinExpr) {.loc = a->loc, .ty = ty, .lhs = a, .bop = op, .rhs = b};
  }
  Expr unary(Expr e, UnaryOp op, CType ty) {
    // construct a unary operator
    return ENEW(UnaryExpr) {.loc = e->loc, .ty = ty, .uoperand = e, .uop = op};
  }

  ArenaAllocator &getAllocator() { 
    return context.getAllocator(); 
  }
  CType gettypedef(IdentRef s) {
    auto it = sema.typedefs.getSym(s);
    if (it && it->ty->tags & TYTYPEDEF)
      return it->ty;
    return nullptr;
  }
  label_t getLabel(IdentRef Name) {
    Label_Info &ref = jumper.lookupLabel(Name);
    switch (ref.flags) {
    case LBL_UNDEFINED: // first label, undefined
      ref.flags = LBL_FORWARD;
      ref.idx = jumper.createLabel();
      break;
    case LBL_FORWARD: // case: stll use undefined label
      break;
    case LBL_DECLARED: // defined, and used => ok!
      ref.flags = LBL_OK;
      break;
    default:
      llvm_unreachable("");
    }
    return ref.idx;
  }
  label_t putLable(IdentRef Name) {
    Label_Info &ref = jumper.lookupLabel(Name);
    switch (ref.flags) {
    case LBL_UNDEFINED: // not used, declared
      ref.flags = LBL_DECLARED;
      ref.idx = jumper.createLabel();
      break;
    case LBL_FORWARD: // used, and defined => ok!
      ref.flags = LBL_OK;
      break;
    case LBL_DECLARED: // declared => declared twice!
      type_error(getLoc(), "duplicate label: %I", Name);
      break;
    default:
      llvm_unreachable("");
    }
    return ref.idx;
  }

  CType gettagByName(IdentRef Name, enum CTypeKind expected, Location loc) {
    CType result;
    size_t idx;
    auto r = sema.tags.getSym(Name, idx);
    if (r) {
      if (r->ty->k != expected)
        type_error(getLoc(), "%s %I is not a %s", expected, Name, get_type_name_str(expected));
      result = r->ty;
    } else {
      result = TNEW(IncompleteType) {.align = 0, .tag = expected, .name = Name};
      Type_info info = Type_info {.ty = result, .loc = loc};
      insertStmt(SNEW(DeclStmt) {
        .loc = loc, 
        .decl_idx = sema.tags.putSym(Name, info),
        .decl_ty = result
      });
    }
    return result;
  }
  size_t puttag(IdentRef Name, CType ty, Location loc, enum CTypeKind k) {
    size_t prev;
    bool found = false;
    if (Name) {
      auto old = sema.tags.getSymInCurrentScope(Name, prev);
      if (old) {
        if (old->ty->k != TYINCOMPLETE) {
          type_error("%s %I redefined", get_type_name_str(k), Name);
        }
        else if (old->ty->tag != k)
          type_error("%s %I redeclared with different as different kind: %s", get_type_name_str(k), Name, get_type_name_str(old->ty->tag));
        old->ty = ty;
        old->loc = loc;
        insertStmt(SNEW(UpdateForwardDeclStmt) {
          .loc = loc,
          .prev_idx = prev,
          .now = ty,
        });
        found = true;
      }
    }
    if (!found) {
      insertStmt(SNEW(DeclStmt) {
        .loc = loc,
        .decl_idx = prev,
        .decl_ty = ty
      });
    }
    return sema.tags.putSym(Name, Type_info {.ty = ty, .loc = loc});
  }
  void putenum(IdentRef Name, uint64_t val, Location loc) {
    if (sema.typedefs.containsInCurrentScope(Name))
      type_error(getLoc(), "%I redeclared", Name);
    sema.typedefs.putSym(Name,
      Variable_Info {
        .ty = context.getConstInt(),
        .loc = loc,
        .val = ConstantInt::get(cast<llvm::IntegerType>(irgen.types[x32]), val),
        .tags = ASSIGNED | USED, // enums are used by default ignore warnings
      }
    );
  }
  // function
  size_t putsymtype2(IdentRef Name, CType yt) {
    Location loc = getLoc();
    CType base = yt->ret;
    if (base->k == TYARRAY)
      type_error(loc, "function cannot return array");
    if (base->tags & TYREGISTER)
      warning(loc, "'register' in function has no effect");
    if (base->tags & TYTHREAD_LOCAL)
      warning(loc, "'_Thread_local' in function has no effect");
    if (base->tags & (TYCONST | TYRESTRICT | TYVOLATILE))
      warning(loc, "type qualifiers ignored in function");
    size_t idx;
    auto it = sema.typedefs.getSymInCurrentScope(Name, idx);
    if (it) {
      if (!compatible(yt, it->ty)) {
        type_error(loc, "conflicting types for function declaration %I", Name);
      } else if (!(it->ty->ret->tags & TYSTATIC) && base->tags & TYSTATIC) {
        type_error(loc, "static declaration of %I follows non-static declaration", Name);
        note(it->loc, "previous declaration of %I was here", Name);
      } else {
        it->ty->tags |= it->ty->ret->tags & (TYSTATIC);
      }
    } else {
      idx = sema.typedefs.putSym(Name, Variable_Info {.ty = yt, .loc = loc, .tags = ASSIGNED}); // function never undefined
    }
    return idx;
  }
  // typedef, variable
  size_t putsymtype(IdentRef Name, CType yt) {
    Location loc = getLoc();
    if (yt->k == TYFUNCTION)
      return putsymtype2(Name, yt);
    size_t idx;
    auto it = sema.typedefs.getSymInCurrentScope(Name, idx);
    if (it) {
      if (isTopLevel() || (yt->tags & (TYEXTERN | TYSTATIC))) {
        CType old = it->ty;
        bool err = true;
        constexpr auto q = TYATOMIC | TYCONST | TYRESTRICT | TYVOLATILE;
        if ((yt->tags & TYSTATIC) && !(old->tags & TYSTATIC))
          type_error(loc, "static declaration case %I follows non-static declaration", Name);
        else if ((old->tags & TYSTATIC) && !(yt->tags & TYSTATIC))
          type_error(loc, "non-static declaration case %I follows static declaration", Name);
        else if ((yt->tags & TYTHREAD_LOCAL) && !(old->tags & TYTHREAD_LOCAL))
          type_error(loc, "thread-local declaration case %I follows non-thread-local declaration", Name);
        else if ((old->tags & TYTHREAD_LOCAL) && !(yt->tags & TYTHREAD_LOCAL))
          type_error(loc, "non-thread-local declaration case %I follows thread-local declaration", Name);
        else if ((yt->tags & q) != (old->tags & q))
          type_error(loc, "conflicting type qualifiers for %I", Name);
        else if (!compatible(old, yt))
          type_error(loc, "conflicting types for %I", Name);
        else
          err = false;
        if (err)
          note("previous declaration case %I with type %T", Name, old);
      } else {
        type_error(loc, "%I redeclared", Name);
      }
      it->ty = yt;
    } else {
      idx = sema.typedefs.putSym(Name, Variable_Info {.ty = yt, .loc = loc, .tags = GARBAGE});
    }
    return idx;
  }
  bool istype(TokenV a) {
    if (is_declaration_specifier(a.tok))
      return true;
    switch (a.tok) {
    case Kstruct:
    case Kenum:
    case Kunion:
    case K_Alignas:
      return true;
    default:
      break;
    }
    if (a.tok == TIdentifier)
      return gettypedef(a.s) != nullptr;
    return false;
  }
  Expr make_cast(Expr from, CastOp op, CType to) {
    return ENEW(CastExpr){
        .loc = from->loc, .ty = to, .castop = op, .castval = from};
  }
  Expr intcast(Expr e, CType to) {
    if ((to->tags & (TYINT8 | TYINT16 | TYINT32 | TYINT64 | TYINT128 | TYUINT8 | TYUINT16 | TYUINT32 | TYUINT64 | TYUINT128)) && (e->ty->tags & (TYINT8 | TYINT16 | TYINT32 | TYINT64 | TYINT128 | TYUINT8 | TYUINT16 | TYUINT32 | TYUINT64 | TYUINT128 | TYBOOL))) {
      if (to->tags == e->ty->tags) {
        return ENEW(CastExpr){.loc = e->loc, .ty = to, .castop = CastOp::BitCast, .castval = e};
      }
      if (intRank(to->tags) > intRank(e->ty->tags)) {
        return make_cast(e, (to->isSigned() && !(e->ty->tags & TYBOOL)) ? SExt : ZExt, to);
      }
      return make_cast(e, Trunc, to);
    }
    return type_error(getLoc(), "cannot cast %E(has type %T) to %T", e, e->ty, to), nullptr;
  }
  static bool type_equal(CType a, CType b) {
    if (a->k != b->k)
      return false;
    else {
      switch (a->k) {
      case TYPRIM:
        return a->tags == b->tags;
      case TYPOINTER:
        return type_equal(a->p, b->p);
      case TYENUM:
      case TYSTRUCT:
      case TYUNION:
        return a == b;
      case TYINCOMPLETE:
        return a->name == b->name;
      default:
        return false;
      }
    }
  }
  Expr castto(Expr e, CType to) {
    Location loc = getLoc();
    if (type_equal(e->ty, to))
      return e;
    if (e->ty->tags & TYVOID)
      return type_error(loc, "cannot cast 'void' expression to type %T", to), e;
    if (to->k == TYINCOMPLETE || e->ty->k == TYINCOMPLETE)
      return type_error(loc, "cannot cast to incomplete type: %T", to), e;
    if (to->k == TYSTRUCT || e->ty->k == TYSTRUCT || to->k == TYUNION ||
        e->ty->k == TYUNION)
      return type_error(loc, "cannot cast between different struct/unions"), e;
    if (e->ty->k == TYENUM)
      return castto(make_cast(e, BitCast, context.getInt()), to);
    if (to->k == TYENUM)
      return castto(castto(e, context.getInt()), to);
    if (!(e->ty->isScalar() && to->isScalar()))
      return type_error(loc, "cast uoperand shall have scalar type"), e;
    if (to->tags & TYBOOL) {
      // simplify 'boolean(a) zext to int(b)' to 'a'
      if (e->k == ECast && e->castop == ZExt && e->castval->ty->tags & TYBOOL) {
        return e->castval;
      }
      Expr zero = wrap(e->ty, ConstantInt::get(irgen.ctx, APInt::getZero(e->ty->getBitWidth())));
      return binop(e, NE, zero, context.typecache.b);
    }
    if ((to->tags & (TYFLOAT | TYDOUBLE)) && e->ty->k == TYPOINTER)
      return type_error(loc, "A floating type shall not be converted to any pointer type"), e;
    if ((e->ty->tags & (TYFLOAT | TYDOUBLE)) && to->k == TYPOINTER)
      return type_error(loc, "A floating type shall not be converted to any pointer type"), e;
    if (e->ty->k == TYPOINTER && to->k == TYPOINTER)
      return make_cast(e, BitCast, to);
    if ((e->ty->tags & TYDOUBLE) && (to->tags & TYFLOAT))
      return make_cast(e, FPTrunc, to);
    if ((e->ty->tags & TYFLOAT) && (to->tags & TYDOUBLE))
      return make_cast(e, FPExt, to);
    if ((e->ty->tags & (TYFLOAT | TYDOUBLE)) && (to->tags & TYF128))
      return make_cast(e, FPExt, to);
    if ((e->ty->tags & TYF128) && (to->tags & (TYFLOAT | TYDOUBLE)))
      return make_cast(e, FPTrunc, to);
    if (e->ty->tags & (TYINT8 | TYINT16 | TYINT32 | TYINT64 | TYINT128 | TYUINT8 |
                       TYUINT16 | TYUINT32 | TYUINT64 | TYUINT128 | TYBOOL)) {
      if (to->k == TYPOINTER) {
        if (e->k == EConstant) {
          auto CI = cast<ConstantInt>(e->C);
          if (CI->isZero()) // A interger constant expression with the value 0 is a *null pointer constant*
            return wrap(to, llvm::ConstantPointerNull::get(cast<llvm::PointerType>(irgen.types[xptr])), e->loc);
          return wrap(to, llvm::ConstantExpr::getIntToPtr(CI, irgen.wrap2(to)), e->loc);
        }
        return make_cast(e, IntToPtr, to);
      }
      if (isFloating(to)) {
        if (e->k == EConstant) {
          if (e->ty->isSigned()) {
            return wrap(to, llvm::ConstantExpr::getSIToFP(e->C, irgen.wrap2(to)), e->loc);
          }
          return wrap(to, llvm::ConstantExpr::getUIToFP(e->C, irgen.wrap2(to)), e->loc);
        }
        return make_cast(e, e->ty->isSigned() ? SIToFP : UIToFP, to);
      }
    } else if (to->tags & (TYINT8 | TYINT16 | TYINT32 | TYINT64 | TYINT128 | TYUINT8 |
                           TYUINT16 | TYUINT32 | TYUINT64 | TYUINT128)) {
      if (e->ty->k == TYPOINTER) {
        if (e->k == EConstant) {
          return wrap(to, llvm::ConstantExpr::getPtrToInt(e->C, irgen.wrap2(to)), e->loc);
        }
        return make_cast(e, PtrToInt, to);
      }
      if (isFloating(e->ty)) {
        if (e->k == EConstant) {
          if (to->isSigned())
            return wrap(to, llvm::ConstantExpr::getFPToSI(e->C, irgen.wrap2(to)), e->loc);
          return wrap(to, llvm::ConstantExpr::getFPToUI(e->C, irgen.wrap2(to)), e->loc);
        }
        return make_cast(e, to->isSigned() ? FPToSI : FPToUI, to);
      }
    }
    return intcast(e, to);
  }
  bool isTopLevel() { 
    return sema.currentfunctionRet == nullptr; 
  }
  void enterBlock() { 
    sema.typedefs.push();
    sema.tags.push();
  }
  void leaveBlock() {
    const auto &T = sema.typedefs;
    for (auto it = T.current_block();it != T.end();++it) {
      if (!(it->info.tags & USED)) {
        if (it->info.tags & ASSIGNED)
          warning(it->info.loc, "%s %I is un-used after assignment", it->info.ty->tags & TYPARAM ? "parameter" : "variable", it->sym);
        else
          warning(it->info.loc, "variable %I is declared but not used", it->sym);
      }
    }
    sema.typedefs.pop();
    sema.tags.pop();
  }
  void leaveBlock2() {
    for (const auto &it: sema.typedefs) {
      if (it.info.ty->tags & TYTYPEDEF)
        continue;
      if (it.info.ty->k == TYFUNCTION) {
        if (it.info.ty->ret->tags & TYSTATIC)
          warning(it.info.loc, "static function %I' declared but not used", it.sym);
      } else if (it.info.ty->tags & TYSTATIC) {
          warning(it.info.loc, "static variable %I declared but not used", it.sym);
      }
    }
  }
  void to(Expr &e, uint32_t tag) {
    if (e->ty->tags != tag)
      e = castto(e, TNEW(PrimType){.align = 0, .tags = tag});
  }
  void integer_promotions(Expr &e) {
    if (e->ty->k == TYBITFIELD ||
        e->ty->tags & (TYBOOL | TYINT8 | TYUINT8 | TYINT16 | TYUINT16))
      to(e, TYINT);
  }
  void conv(Expr &a, Expr &b) {
    uint32_t at = a->ty->tags;
    uint32_t bt = b->ty->tags;
    if (a->ty->k != TYPRIM || b->ty->k != TYPRIM)
      return;
    if (at & TYLONGDOUBLE)
      return to(b, TYLONGDOUBLE);
    if (bt & TYLONGDOUBLE)
      return to(a, TYLONGDOUBLE);
    if (at & TYDOUBLE)
      return to(b, TYDOUBLE);
    if (bt & TYDOUBLE)
      return to(a, TYDOUBLE);
    if (at & TYFLOAT)
      return to(b, TYFLOAT);
    if (bt & TYFLOAT)
      return to(a, TYFLOAT);
    integer_promotions(a);
    integer_promotions(b);
    if (intRank(a->ty->tags) == intRank(b->ty->tags))
      return;
    auto sizeofa = getsizeof(a->ty);
    auto sizeofb = getsizeof(b->ty);
    bool isaunsigned = (at & ty_unsigned) != 0;
    bool isbunsigned = (bt & ty_unsigned) != 0;
    // if operand has the same sign
    if (isaunsigned == isbunsigned)
      return sizeofa > sizeofb ? to(b, at) : to(a, bt);
    if (isaunsigned && (sizeofa > sizeofb))
      return to(b, at);
    if (isbunsigned && (sizeofb > sizeofa))
      return to(a, bt);
    if (!isaunsigned && sizeofa > sizeofb)
      return to(b, at);
    if (!isbunsigned && sizeofb > sizeofa)
      return to(a, bt);
    return isaunsigned ? to(b, at) : to(a, bt);
  }
  void default_argument_promotions(Expr &e) {
    if (e->ty->k == TYENUM || e->ty->k == TYUNION || e->ty->k == TYSTRUCT ||
        e->ty->k == TYPOINTER)
      return;
    if (e->ty->tags & TYFLOAT)
      e = castto(e, context.typecache.fdoublety);
    integer_promotions(e);
  }
  bool checkInteger(CType ty) {
    return ty->k == TYPRIM && !(ty->k & (TYVOID | TYDOUBLE | TYFLOAT));
  }
  bool checkInteger(Expr e) { return checkInteger(e->ty); }
  void checkInteger(Expr a, Expr b) {
    if (!(checkInteger(a->ty) && checkInteger(b->ty)))
      type_error(getLoc(), "integer types expected");
  }
  static int checkArithmetic(CType ty) { return ty->k == TYPRIM; }
  void checkArithmetic(Expr a, Expr b) {
    if (!(checkArithmetic(a->ty) && checkArithmetic(b->ty)))
      type_error(getLoc(), "arithmetic types expected");
  }
  void checkSpec(Expr &a, Expr &b) {
    if (a->ty->k != b->ty->k)
      return type_error(getLoc(), "bad operands to binary expression:\n    operands type mismatch: %E(has type %T) and %E(has type %T)", a, b);
    if (!(a->ty->isScalar() && b->ty->isScalar()))
      return type_error(getLoc(), "scalar types expected");
    conv(a, b);
  }
  bool issimple(Expr e) {
    switch (e->k) {
      case EVar:
      case EString:
      case EStruct:
      case EArray:
        return true;
      default: 
        return false;
    }
  }
  bool isAssignOp(enum BinOp op) {
    switch (op) {
      case Assign:
      case AtomicrmwAdd:
      case AtomicrmwSub:
      case AtomicrmwAnd:
      case AtomicrmwOr:
      case AtomicrmwXor:
        return true;
      default:
        return false;
    }
  }
  Expr comma(Expr a, Expr b) {
    return binop(simplify(a), Comma, simplify(b), b->ty);
  }
  Expr simplify(const Expr e) {
    switch (e->k) {
        case EBin:
          if (isAssignOp(e->bop) || e->bop == LogicalAnd || e->bop == LogicalOr)
            return e;
          if (issimple(e->rhs)) {
            warning(e->rhs->loc, "unused binary expression result in right-hand side (%E)", e->rhs);
            note("simplify %E to %E", e, e->lhs);
            return simplify(e->lhs);
          }
          if (issimple(e->lhs)) {
            warning(e->rhs->loc, "unused binary expression result in left-hand side (%E)", e->rhs);
            note("simplify %E to %E", e, e->rhs);
            return simplify(e->rhs);
          }
          return comma(simplify(e->lhs), simplify(e->rhs));
        case EUnary:
          if (e->uop != Dereference) {
            warning(e->loc, "unused unary expression result");
            note("simplify %E to %E", e, e->uoperand);
            return simplify(e->uoperand);
          }
          return e;
        case EVoid:
            return simplify(e->castval);
        case ECondition:
            // if the cond is simple, we should simplify it in constant folding
            if (issimple(e->cleft) && issimple(e->cright)) {
              warning("unused conditional-expression left-hand side and right-hand result");
              note("simplify %E to %E", e, e->cond);
              return simplify(e->cond);
            }
        case ECast:
            warning(e->loc, "unused cast expression, replace '(type)x' with 'x'");
            return simplify(e->castval);
        case ECall:
            return e;
        case ESubscript:
            if (issimple(e->right)) {
              warning("unused array substract, replace 'a[b]' with a");
              return simplify(e->left);
            }
            warning("unused subscript expression");
            return comma(e->left, e->right);
        case EPostFix:
        case EArray:
        case EStruct:
        case EString:
        case EMemberAccess:
        case EVar:
        case EConstant:
        case EArrToAddress:
            return e;
    }
    llvm_unreachable("bad expr kind");
  }
  void make_add(Expr &result, Expr &r) {
    if (isFloating(result->ty)) {
      conv(result, r);
      if (result->k == EConstant && r->k == EConstant) {
        result = wrap(r->ty, llvm::ConstantExpr::getAdd(result->C, r->C), r->loc);
        return;
      }
      result = binop(result, FAdd, r, r->ty);
      return;
    }
    if (result->ty->k == TYPOINTER) {
      if (!checkInteger(r->ty))
        type_error(getLoc(), "integer expected");

      result = binop(result, SAddP, r, result->ty);
      return;
    }
    checkSpec(result, r);
    if (result->k == EConstant && r->k == EConstant) {
      result = wrap(r->ty, llvm::ConstantExpr::getAdd(result->C, r->C, false, r->ty->isSigned()), r->loc);
      return;
    }
    result = binop(result, r->ty->isSigned() ? SAdd : UAdd, r, r->ty);
  }
  void make_sub(Expr &result, Expr &r) {
    if (isFloating(result->ty)) {
      if (result->k == EConstant && r->k == EConstant) {
        result = wrap(r->ty, llvm::ConstantExpr::getSub(result->C, r->C), r->loc);
        return;
      }
      conv(result, r);
      result = binop(result, FSub, r, r->ty);
      return;
    }
    if (result->ty->k == TYPOINTER) {
      if (r->ty->k == TYPOINTER) {
        // two pointer substraction
        // https://stackoverflow.com/questions/65748155/what-is-the-motivation-for-ptrdiff-t
        //                if ((!(result->ty->p->tags & TYVOID)) ||
        //                (r->ty->p->tags & TYVOID))
        //                    return type_error(getLoc(), "cannot substract on a
        //                    pointer to void");
        if (!compatible(result->ty->p, r->ty->p))
          warning(getLoc(), "incompatible type when substract two pointers");
        CType ty = context.getIntPtr();
        result = binop(make_cast(result, PtrToInt, ty), PtrDiff,
                       make_cast(r, PtrToInt, ty), ty);
        return;
      }
      if (!checkInteger(r->ty))
        type_error(getLoc(), "integer expected");
      result = binop(result, SAddP, unary(r, r->ty->isSigned() ? SNeg : UNeg, r->ty), result->ty);
      return;
    }
    checkSpec(result, r);
    if (result->k == EConstant && r->k == EConstant) {
      result = wrap(r->ty, llvm::ConstantExpr::getSub(result->C, r->C, false, r->ty->isSigned()), r->loc);
      return;
    }
    result = binop(result, r->ty->isSigned() ? SSub : USub, r, r->ty);
  }
  void make_shl(Expr &result, Expr &r) {
    checkInteger(result, r);
    integer_promotions(result);
    integer_promotions(r);
    if (r->k != EConstant) goto NOT_CONSTANT;
    if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
      auto width = result->ty->getBitWidth();
      const APInt &shift = CI->getValue();
      if (r->ty->isSigned() && shift.isNegative()) {
        warning(r->loc, "negative shift count is undefined");
        goto BAD;
      }
      if (shift.uge(width)) {
        warning(result->loc, "shift count(%A) >= width of type(%z)", &shift, (size_t)width);
        goto BAD;
      }
      if (result->k != EConstant) goto NOT_CONSTANT;
      {
        const APInt &obj = cast<ConstantInt>(result->C)->getValue();
        bool overflow = false;
        if (result->ty->isSigned() && obj.isNegative())
          warning(result->loc, "shifting a negative signed value is undefined");
        result = wrap(result->ty, ConstantInt::get(irgen.ctx, result->ty->isSigned() ? obj.sshl_ov(shift, overflow) : obj << shift), result->loc);
        if (overflow)
          warning(result->loc, "shift-left overflow, result is %A", &cast<ConstantInt>(result->C)->getValue());
        return;
      }
      BAD:
      result = wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getZero(width)), result->loc);
      return;
    }
    if (result->k != EConstant) goto NOT_CONSTANT;
    result = wrap(r->ty, llvm::ConstantExpr::getShl(result->C, r->C), r->loc);
    return;
    NOT_CONSTANT:
    result = binop(result, Shl, r, result->ty);
  }
  void make_shr(Expr &result, Expr &r) {
    checkInteger(result, r);
    integer_promotions(result);
    integer_promotions(r);
    if (r->k != EConstant) goto NOT_CONSTANT;
    if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
      auto width = result->ty->getBitWidth();
      const APInt &shift = CI->getValue();
      if (r->ty->isSigned() && shift.isNegative()) {
        warning(r->loc, "negative shift count is undefined");
        goto BAD;
      }
      if (shift.uge(width)) {
        warning(result->loc, "shift count(%A) >= width of type(%z)", &shift, (size_t)width);
        goto BAD;
      }
      if (result->k != EConstant) goto NOT_CONSTANT;
      {
        APInt obj = cast<ConstantInt>(result->C)->getValue();
        if (result->ty->isSigned())
          obj.ashrInPlace((unsigned)shift.getZExtValue());
        else
          obj.lshrInPlace((unsigned)shift.getZExtValue());
        result = wrap(result->ty, ConstantInt::get(irgen.ctx, obj), result->loc);
        return;
      }
      BAD:
      result = wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getZero(width)), result->loc);
      return;
    }
    if (result->k != EConstant) goto NOT_CONSTANT;
    result = wrap(r->ty, llvm::ConstantExpr::getShl(result->C, r->C), r->loc);
    return;
    NOT_CONSTANT:
    result = binop(result, result->ty->isSigned() ? AShr : Shr, r, result->ty);
  }
  void make_bitop(Expr &result, Expr &r, BinOp op) {
    checkInteger(result, r);
    conv(result, r);
    if (result->k == EConstant) {
      if (const auto CI = dyn_cast<ConstantInt>(result->C)) {
        if (CI->isZero())
          goto ZERO;
        if (CI->getValue().isAllOnes() && op != Xor)
          goto ONE;
        if (r->k == EConstant) {
          if (const auto CI2 = dyn_cast<ConstantInt>(r->C)) {
            APInt val(CI->getValue());
            switch (op) {
              case And: val &= CI2->getValue(); break;
              case Or: val |= CI2->getValue(); break;
              case Xor: val ^= CI2->getValue(); break;
              default: llvm_unreachable("");
            }
            result = wrap(result->ty, ConstantInt::get(irgen.ctx, val), result->loc);
            return;
          }
        }
      }
    }
    else if (r->k == EConstant) {
      if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
        if (CI->isZero())
          goto ZERO;
        if (CI->getValue().isAllOnes() && op != Xor)
          goto ONE;
      }
    }
    result = binop(result, op, r, result->ty);
    return;
    ZERO:
    {
      const char *msg;
      if (op == Or)
        msg = "'bitwise or' to zero is always itself";
      else if (op == And)
        msg = "'bitwise and' to to zero is always zero";
      else
        msg = "'bitwise xor' to zero is always itself";
      warning(result->loc, msg);
      result = wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getZero(result->ty->getBitWidth())), result->loc);
      return;
    }
    ONE:
    {
      const StringRef allOnes = "all ones(all bits set, -1)";
      if (op == Or) {
        warning(result->loc, "'bitsize or' to %R is always all ones", allOnes);
        result = result = wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getAllOnes(result->ty->getBitWidth())), result->loc);
        return;
      }
      warning(result->loc, "'bitsize and' to %R is always itself", allOnes);
    }
  }
  void make_mul(Expr &result, Expr &r) {
    checkArithmetic(result, r);
    conv(result, r);
    if (result->k != EConstant) goto NOT_CONSTANT;
    if (const auto lhs = dyn_cast<ConstantInt>(result->C)) {
      if (lhs->isZero()) goto ZERO;
      if (r->k != EConstant) goto NOT_CONSTANT;
      if (const auto rhs = dyn_cast<ConstantInt>(r->C)) {
        bool overflow = false;
        if (rhs->isZero()) goto ZERO;
        result = wrap(r->ty, ConstantInt::get(irgen.ctx, r->ty->isSigned() ? lhs->getValue().smul_ov(rhs->getValue(), overflow) : lhs->getValue().umul_ov(rhs->getValue(), overflow)), result->loc);
        if (overflow)
          warning(result->loc, "multiplication overflow");
        return;
      }
    }
    else if (const auto lhs = dyn_cast<ConstantFP>(result->C)) {
      if (r->k != EConstant) goto NOT_CONSTANT;
      if (const auto rhs = dyn_cast<ConstantFP>(result->C)) {
        APFloat F = lhs->getValue();
        handleOpStatus(F.multiply(rhs->getValue(), APFloat::rmNearestTiesToEven));
        result = wrap(r->ty, ConstantFP::get(irgen.ctx, F), result->loc);
        return;
      }
      result = wrap(r->ty, llvm::ConstantExpr::getMul(result->C, r->C), result->loc);
      return;
    }
    NOT_CONSTANT:
    result = binop(result, isFloating(r->ty) ? FMul : (r->ty->isSigned() ? SMul : UMul), r, r->ty);
    return;
    ZERO:
    warning(result->loc, "multiplying by zero is always zero (zero-product property)");
  }
  void make_div(Expr &result, Expr &r) {
    checkArithmetic(result, r);
    conv(result, r);
    if (r->k != EConstant) goto NOT_CONSTANT;
    if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
      if (CI->getValue().isZero()) {
        warning(result->loc, "integer division by zero is undefined");
        result = wrap(r->ty, llvm::UndefValue::get(CI->getType()), result->loc);
        return;
      }
      if (result->k != EConstant) goto NOT_CONSTANT;
      result = wrap(r->ty, llvm::ConstantExpr::get(r->ty->isSigned() ? llvm::Instruction::SDiv : llvm::Instruction::UDiv, result->C, r->C), result->loc);
      return;
    }
    else if (const auto CFP = dyn_cast<ConstantFP>(r->C)) {
      if (CFP->isZero()) {
        warning(result->loc, "floating division by zero is undefined");
        result = wrap(r->ty, llvm::UndefValue::get(CFP->getType()), result->loc);
        return;
      }
      if (result->k != EConstant) goto NOT_CONSTANT;
      result = wrap(r->ty, llvm::ConstantExpr::get(llvm::Instruction::FDiv, result->C, r->C), result->loc);
      return;
    }
    NOT_CONSTANT:
    result = binop(result, isFloating(r->ty) ? FDiv : (r->ty->isSigned() ? SDiv : UDiv), r, r->ty);
  }
  void handleOpStatus(APFloat::opStatus status) {

  }
  void make_rem(Expr &result, Expr &r) {
    checkInteger(result, r);
    conv(result, r);
    if (result->k == EConstant && r->k == EConstant) {
      if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
        if (CI->getValue().isZero()) {
          warning("integer remainder by zero is undefined");
          result = wrap(r->ty, llvm::UndefValue::get(CI->getType()), result->loc);
          return;
        }
        result = wrap(r->ty, llvm::ConstantExpr::get(r->ty->isSigned() ? llvm::Instruction::SRem : llvm::Instruction::URem, result->C, r->C), result->loc);
        return;
      }
      else if (const auto CFP = dyn_cast<ConstantFP>(r->C)) {
        if (CFP->isZero()) {
          warning("integer remainder by zero is undefined");
          result = wrap(r->ty, llvm::UndefValue::get(CFP->getType()), result->loc);
          return;
        }
        result = wrap(r->ty, llvm::ConstantExpr::get(llvm::Instruction::FRem, result->C, r->C), result->loc);
        return;
      }
    }
    result = binop(result, isFloating(r->ty) ? FRem : (r->ty->isSigned() ? SRem : URem), r, r->ty);
  }
  static BinOp get_relational_expression_op(Token tok, bool isFloating, bool isSigned) {
    switch (tok) {
      case TLe: return isFloating ? FLE : (isSigned ? SLE : ULE);
      case TLt: return isFloating ? FLT : (isSigned ? SLT : ULT);
      case TGt: return isFloating ? FGT : (isSigned ? SGT : UGT);
      case TGe: return isFloating ? FGE : (isSigned ? SGE : UGE);
      case TNe: return isFloating ? FNE: NE;
      case TEq: return isFloating ? FEQ: EQ;
      default: llvm_unreachable("");
    }
  }
  void make_cmp(Expr &result, Token tok, bool isEq = false) {
    Expr r;
    consume();
    if (!(r = isEq ? relational_expression() : shift_expression()))
      return;
    checkSpec(result, r);
    if (result->k == EConstant && r->k == EConstant) {
      if (const auto CI = dyn_cast<ConstantInt>(result->C)) {
        auto CI2 = dyn_cast<ConstantInt>(r->C);
        const APInt &A1 = CI->getValue();
        const APInt &A2 = CI2->getValue();
        int status;
        if (result->ty->isSigned()) {
          if (A1.isSingleWord()) {
            auto x = A1.getZExtValue();
            auto y = A2.getZExtValue();
            status = x < y ? -1 : (x == y) ? 0 : -1;
          } else {
            status = APInt::tcCompare(A1.getRawData(), A2.getRawData(), A1.getNumWords());
          }
        } else {
          if (A1.isSingleWord()) {
            int64_t x = llvm::SignExtend64(A1.getZExtValue(), A1.getBitWidth());
            int64_t y = llvm::SignExtend64(A2.getZExtValue(), A2.getBitWidth());
            status = x < y ? -1 : (x == y) ? 0 : -1;
          } else {
            bool lhsNeg = A1.isNegative();
            bool rhsNeg = A2.isNegative();
            status = lhsNeg != rhsNeg ?
              lhsNeg ? -1 : 1 :
              APInt::tcCompare(A1.getRawData(), A2.getRawData(), A1.getNumWords());
          }
        }
        bool B;
        switch (tok) {
          case TGt: B = status > 0; break;
          case TGe: B = status >= 0; break;
          case TEq: B = status == 0; break;
          case TNe: B = status != 0; break;
          case TLt: B = status < 0; break;
          case TLe: B = status <= 0; break;
          default: llvm_unreachable("bad operator to make_cmp");
        }
        result = wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt(context.getInt()->getBitWidth(), (uint64_t)B)), result->loc);
        return;
      }
      if (const auto CFP = dyn_cast<ConstantFP>(result->C)) {
        const auto CFP2 = dyn_cast<ConstantFP>(r->C);
        bool B;
        APFloat::cmpResult status = CFP->getValue().compare(CFP2->getValue());
        constexpr APFloat::cmpResult 
          cmpGreaterThan = APFloat::cmpGreaterThan,
          cmpEqual = APFloat::cmpEqual,
          cmpLessThan = APFloat::cmpLessThan;
        switch (tok) {
          case TGt: B = status == cmpGreaterThan; break;
          case TGe: B = status == cmpEqual || status == cmpGreaterThan; break;
          case TEq: B = status == cmpEqual; break;
          case TNe: B = status != cmpEqual; break;
          case TLt: B = status == cmpLessThan || status == cmpEqual; break;
          case TLe: B = status == cmpLessThan; break;
          default: llvm_unreachable("bad operator to make_cmp");
        }
        result = wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt(context.getInt()->getBitWidth(), (uint64_t)B)), result->loc);
        return;
      }
    }
    result = boolToInt(binop(result, get_relational_expression_op(tok, isFloating(result->ty), result->ty->isSigned()), r, context.typecache.b));
  }
  Expr boolToInt(Expr e) {
    return ENEW(CastExpr) {.loc = e->loc, .ty = context.getInt(), .castop = ZExt, .castval = e};
  }
  enum DeclaratorFlags {
    Direct = 0,   // declarator with name
    Abstract = 1, // declarator without name
    Function = 2 // function parameter-type-list = Direct or Abstract
  };
  CType declaration_specifiers() { 
    return specifier_qualifier_list(); 
  }
  Declator abstract_decorator(CType base, enum DeclaratorFlags flags) {
    // abstract decorator has no name
    // for example: `static int        ()(int, char)`
    //               base-type      abstact-decorator
    return declarator(base, flags);
  }
  void consume() {
    // eat token from preprocessor
    l.cpp();
    if (l.tok.tok >= TIdentifier)
      // make preprocessor identfiers to normal identfier
      l.tok.tok = TIdentifier;
  }
  Location getLoc() { 
    return l.getLoc(); 
  }
  Expr constant_expression() { 
    return conditional_expression(); 
  }
  bool addTag(CType &ty, Token theTok) {
    // add a tag to type
    switch (theTok) {
    case Kinline:
      ty->tags |= TYINLINE;
      break;
    case K_Noreturn:
      ty->tags |= TYNORETURN;
      break;
    case Kextern:
      ty->tags |= TYEXTERN;
      break;
    case Kstatic:
      ty->tags |= TYSTATIC;
      break;
    case K_Thread_local:
      ty->tags |= TYTHREAD_LOCAL;
      break;
    case Kregister:
      ty->tags |= TYREGISTER;
      break;
    case Krestrict:
      ty->tags |= TYRESTRICT;
      break;
    case Kvolatile:
      ty->tags |= TYVOLATILE;
      break;
    case Ktypedef:
      ty->tags |= TYTYPEDEF;
      break;
    case Kconst:
      ty->tags |= TYCONST;
      break;
    case K_Atomic:
      ty->tags |= TYATOMIC;
      break;
    default:
      return false;
    }
    return true;
  }
  CType merge_types(ArrayRef<Token> ts, Location loc) {
    // merge many token to a type
    //
    // for example:
    //
    //   `long long int const` => const long long
    unsigned l = 0, s = 0, f = 0, d = 0, i = 0, c = 0, v = 0, numsigned = 0,
             numunsigned = 0, su;
    CType result;

    result = TNEW(PrimType){.align = 0, .tags = 0};
    SmallVector<Token, 5> b;
    for (const auto t : ts) {
      if (!addTag(result, t))
        b.push_back(t);
    }
    if (b.empty()) { // no type
      warning(loc, "default type to `int`");
      result->tags |= TYINT;
      return result;
    }
    if (b.size() == 1) { // one type
      result->tags |= ([](Token theTok) {
        switch (theTok) {
        case Kchar:
          return TYCHAR;
        case Kint:
          return TYINT;
        case Kvoid:
          return TYVOID;
        case Klong:
          return TYLONG;
        case Ksigned:
          return TYINT;
        case Kunsigned:
          return TYUINT;
        case Kshort:
          return TYSHORT;
        case Kdouble:
          return TYDOUBLE;
        case Kfloat:
          return TYFLOAT;
        case K_Bool:
          return TYBOOL;
        default:
          llvm_unreachable("");
        }
      })(b.front());
      return result;
    }
    for (const auto tok : b) {
      switch (tok) {
      case Ksigned:
        numsigned++;
        break;
      case Kunsigned:
        numunsigned++;
        break;
      case Klong:
        l++;
        break;
      case Kshort:
        s++;
        break;
      case Kint:
        i++;
        break;
      case Kdouble:
        d++;
        break;
      case Kfloat:
        f++;
        break;
      case Kvoid:
        v++;
        break;
      case Kchar:
        c++;
        break;
      default:
        break;
      }
    }
    su = numsigned + numunsigned;
    if (su >= 2)
      return type_error(loc, "duplicate signed/unsigned"), nullptr;
    if (c > 1)
      return type_error(loc, "too many `char`s"), nullptr;
    if (i > 1)
      return type_error(loc, "too many `int`s"), nullptr;
    if (f > 0)
      return type_error(loc, "`float` cannot combine with other types"),
             nullptr;
    if (v > 0)
      return type_error(loc, "`void` cannot combine with other types"), nullptr;
    if (l >= 3)
      return type_error(loc, "too many `long`s, max is `long long`"), nullptr;
    if (s >= 2)
      return s == 2 ? type_error(loc, "duplicate 'short' declaration specifier")
                    : type_error(loc, "too many `short`s, max is `short`"),
             nullptr;
    if (d > 1)
      return type_error(loc, "too many `double`s"), nullptr;
    if (d) {
      if (d != 1)
        return type_error(loc, "too many `long` for `double`"), nullptr;
      if (b.size() != (1 + l))
        return type_error(loc, "extra `double` declaration specifier"), nullptr;
      result->tags |= (d == 1 ? TYLONGDOUBLE : TYDOUBLE);
      return result;
    }
    if (s) { // short+int
      if ((s + i + su) != b.size())
        return type_error(loc, "extra `short` declaration specifier"), nullptr;
      result->tags |= (numunsigned ? TYUSHORT : TYSHORT);
      return result;
    }
    if (l) { // long+int
      if ((l + i + su) != b.size())
        return type_error(loc, "extra `long` declaration specifier"), nullptr;
      switch (l) {
      case 1:
        result->tags |= numunsigned ? TYULONG : TYLONG;
        break;
      case 2:
        result->tags |= numunsigned ? TYULONGLONG : TYLONGLONG;
        break;
      default:
        llvm_unreachable("too many longs");
      }
      return result;
    }
    if (c) {
      if ((c + su) != b.size())
        return type_error(loc, "extra `char` declaration specifier"), nullptr;
      result->tags |= numunsigned ? TYUCHAR : TYCHAR;
      return result;
    }
    if (i) {
      if ((i + su) != b.size())
        return type_error(loc, "extra `int` declaration specifier"), nullptr;
      result->tags |= numsigned ? TYUINT : TYINT;
      return result;
    }
    return type_error(loc, "bad type specifier"), nullptr;
  }
  void checkSemicolon() {
    if (l.tok.tok != TSemicolon)
      return warning(getLoc(), "missing ';'");
    consume();
  }
  void type_qualifier_list(CType &ty) {
    // parse many type qualifiers, add to type
    for (;;)
      switch (l.tok.tok) {
      case Kconst:
        ty->tags |= TYCONST, consume();
        continue;
      case Krestrict:
        ty->tags |= TYRESTRICT, consume();
        continue;
      case Kvolatile:
        ty->tags |= TYVOLATILE, consume();
        continue;
      case K_Atomic:
        ty->tags |= TYATOMIC, consume();
        continue;
      default:
        return;
      }
  }
  void more() {
    while (is_declaration_specifier(l.tok.tok))
      sema.tokens_cache.push_back(l.tok.tok), consume();
  }
  void read_enum_sepcs(CType &c, Token sepc) {
    // TODO: ...
  }
  void read_struct_union_sepcs(CType &c, Token sepc) {
    // TODO: ...
  }

  CType handle_typedef(CType ty) {
    bool is_redefine = false;
    CType result = context.clone(ty);
    consume();
    while (is_declaration_specifier(l.tok.tok))
      sema.tokens_cache.push_back(l.tok.tok), consume();
    if (sema.tokens_cache.size()) {
      for (size_t i = 0; i < sema.tokens_cache.size(); ++i) {
        switch (i) {
        case Ktypedef:
          break;
        default:
          if (!addTag(result, sema.tokens_cache[i]))
            is_redefine = true;
        }
      }
    }
    if (is_redefine) {
      if (!compatible(result, ty))
        type_error("typedef redefinition with different types: %T vs %T", result, ty);
    } else {
      result->tags &= (~TYTYPEDEF);
    }
    return result;
  }
  CType specifier_qualifier_list() {
    Location loc = getLoc();
    // specfier-qualifier-list: parse many type specfiers and type qualifiers
    CType result;
    sema.tokens_cache.clear();
    while (is_declaration_specifier(l.tok.tok)) {
      switch (l.tok.tok) {
      case Kenum:
        result = enum_decl();
        more();
        for (const auto tok : sema.tokens_cache) {
          if (tok == Ktypedef) {
            result->tags |= TYTYPEDEF;
            continue;
          }
          read_enum_sepcs(result, tok);
        }
        return result;
      case Kunion:
      case Kstruct:
        result = struct_union(l.tok.tok);
        more();
        for (const auto tok: sema.tokens_cache) {
          if (tok == Ktypedef) {
            result->tags |= TYTYPEDEF;
            continue;
          }
          read_struct_union_sepcs(result, tok);
        }
        return result;
      case K_Atomic:
        consume();
        if (l.tok.tok == TLbracket) {
          consume();
          if (!(result = type_name()))
            return expect(getLoc(), "type-name"), nullptr;
          if (l.tok.tok != TRbracket)
            return expectRB(getLoc()), nullptr;
          consume();
          more();
          if (sema.tokens_cache.size())
            warning(getLoc(), "atomic-type-specifier cannot combine with other types");
          return result;
        }
        LLVM_FALLTHROUGH;
      default:
        sema.tokens_cache.push_back(l.tok.tok), consume();
      }
    }
    if (l.tok.tok == TIdentifier) {
      result = gettypedef(l.tok.s);
      if (result)
        return more(), handle_typedef(result);
    }
    return merge_types(sema.tokens_cache, loc);
  }
  Declator declarator(CType base, enum DeclaratorFlags flags = Direct) {
    // take a base type, return the final type and name
    //
    // for example: `static   int     foo`
    //
    //                base-type    decorator
    CType ty = base;
    while (l.tok.tok == TMul)
      consume(), ty = context.getPointerType(ty), type_qualifier_list(ty);
    return direct_declarator(ty, flags);
  }
  Expr initializer_list() {
    Expr e, result;
    int m;
    if (l.tok.tok != TLcurlyBracket) {
      if (!sema.currentInitTy)
        // when 'excess elements in initializer-list', 'sema.currentInitTy' is
        // nullptr
        return assignment_expression();
      auto k = sema.currentInitTy->k;
      if (k != TYPRIM && k != TYPOINTER && k != TYENUM)
        return type_error(getLoc(), "expect bracket initializer"), nullptr;
      if (!(e = assignment_expression()))
        return nullptr;
      return castto(e, sema.currentInitTy);
    }
    result = sema.currentInitTy->k == TYSTRUCT
                 ? ENEW(StructExpr){.ty = sema.currentInitTy, .arr2 = xvector<Expr>::get()}
                 : ENEW(ArrayExpr){.ty = sema.currentInitTy,.arr = xvector<Expr>::get()};
    result->loc = getLoc();
    consume();
    if (sema.currentInitTy->k == TYARRAY) {
      if (sema.currentInitTy->hassize) {
        m = sema.currentInitTy->arrsize;
      } else {
        result->ty->hassize = true, result->ty->arrsize = 0, m = -1;
      }
    } else if (sema.currentInitTy->k == TYSTRUCT) {
      m = sema.currentInitTy->selems.size();
    } else {
      // braces around scalar initializer
      m = 1;
    }
    for (unsigned i = 0;; i++) {
      CType ty;
      if (l.tok.tok == TRcurlyBracket) {
        consume();
        break;
      }
      if (sema.currentInitTy->k == TYSTRUCT) {
        ty = i < (unsigned)m ? sema.currentInitTy->selems[i].ty : nullptr;
      } else if (sema.currentInitTy->k == TYARRAY) {
        ty = sema.currentInitTy->arrtype;
      } else {
        ty = sema.currentInitTy;
      }
      {
        CType o = sema.currentInitTy;
        sema.currentInitTy = ty;
        e = initializer_list();
        sema.currentInitTy = o;
      }
      if (!e)
        return nullptr;
      if (m == -1) {
        result->arr.push_back(e), result->ty->arrsize++;
      } else if (i < (unsigned)m) {
        result->arr.push_back(e);
      } else {
        warning(getLoc(), "excess elements in initializer-list");
      }
      if (l.tok.tok == TComma)
        consume();
    }
    auto k = sema.currentInitTy->k;
    if (k == TYPRIM || k == TYPOINTER || k == TYENUM)
      result = result->arr.front();
    return result;
  }

  Declator direct_declarator(CType base, enum DeclaratorFlags flags = Direct) {
    switch (l.tok.tok) {
    case TIdentifier: {
      IdentRef name;
      if (flags == Abstract)
        return Declator();
      name = l.tok.s, consume();
      return direct_declarator_end(base, name);
    }
    case TLbracket: {
      // int (*fn_ptr)(void);
      // int (*)(void);
      // int ((*arr)[5])[5];
      // int ((*)[5])[5];
      Declator st, st2;
      CType dummy = reinterpret_cast<CType>(getAllocator().Allocate(
          ctype_max_size,
          alignof(std::max_align_t))); // make a dummy type, large enough to
                                       // hold any
      dummy->k = TYPRIM;
      dummy->tags = TYINT;
      dummy->align = 0;
      consume();
      st = declarator(dummy, flags);
      if (!st.ty)
        return Declator();
      if (l.tok.tok != TRbracket)
        warning(getLoc(), "missing ')'");
      else
        consume();
      st2 = direct_declarator_end(base, st.name);
      if (!st2.ty)
        return Declator();
      memcpy(reinterpret_cast<void *>(dummy),
             reinterpret_cast<const void *>(st2.ty), ctype_size_map[st2.ty->k]);
      return st;
    }
    default:
      if (flags == Direct)
        return Declator();
      return direct_declarator_end(base, nullptr);
    }
  }
  Declator direct_declarator_end(CType base, IdentRef name) {
    switch (l.tok.tok) {
    case TLSquareBrackets: {
      CType ty = TNEW(ArrayType){
          .vla = nullptr, .arrtype = base, .hassize = false, .arrsize = 0};
      consume();
      if (l.tok.tok == TMul) {
        consume();
        if (l.tok.tok != TRSquareBrackets)
          return expect(getLoc(), "]"), Declator();
        return Declator(name, ty);
      }
      if (l.tok.tok == Kstatic)
        consume(), type_qualifier_list(ty);
      else {
        type_qualifier_list(ty);
        if (l.tok.tok == Kstatic)
          consume();
      }
      if (l.tok.tok != TRSquareBrackets) {
        Expr e;
        bool ok;
        Location cloc = getLoc();
        if (!(e = assignment_expression()))
          return Declator();
        if (!checkInteger(e->ty))
          return type_error(getLoc(), "size of array has non-integer type %T",  e->ty), Declator();
        ty->hassize = true;
        ty->arrsize = try_eval(e, cloc, ok);
        if (!ok) {
          ty->vla = e;
        }
        if (l.tok.tok != TRSquareBrackets)
          return expect(getLoc(), "]"), Declator();
      }
      consume();
      return direct_declarator_end(ty, name);
    }
    case TLbracket: 
    {
      CType ty = TNEW(FunctionType) {.ret = base, .params = xvector<Param>::get(), .isVarArg = false};
      consume();
      if (l.tok.tok != TRbracket) {
        if (!parameter_type_list(ty->params, ty->isVarArg))
          return Declator();
        if (l.tok.tok != TRbracket)
          return expectRB(getLoc()), Declator();
      } else {
        ty->isVarArg = true;
      }
      consume();
      return direct_declarator_end(ty, name);
    }
    default:
      return Declator(name, base);
    }
  }
  Declator struct_declarator(CType base) {
    Declator d;

    if (l.tok.tok == TColon) {
      Expr e;
      unsigned bitsize;
      consume();
      Location cloc = getLoc();
      if (!(e = constant_expression()))
        return Declator();
      bitsize = force_eval(e, cloc);
      return Declator(nullptr, TNEW(BitfieldType) {.bittype = base, .bitsize = bitsize});
    }
    d = declarator(base, Direct);
    if (!d.ty)
      return Declator();
    if (l.tok.tok == TColon) {
      unsigned bitsize;
      Expr e;
      consume();
      Location cloc = getLoc();
      if (!(e = constant_expression()))
        return Declator();
      bitsize = force_eval(e, cloc);
      return Declator(nullptr, TNEW(BitfieldType){.bittype = base, .bitsize = bitsize});
    }
    return Declator(d.name, d.ty);
  }
  CType struct_union(Token tok) {
    // parse a struct or union, return it
    // for example:  `struct Foo`
    //
    //               `struct { ... }`
    //
    //               `struct Foo { ... }`
    Location loc = getLoc();
    consume(); // eat struct/union
    IdentRef Name = nullptr;
    if (l.tok.tok == TIdentifier) {
      Location loc2 = getLoc();
      Name = l.tok.s;
      consume();
      if (l.tok.tok != TLcurlyBracket) // struct Foo
        return gettagByName(Name, tok == Kstruct ? TYSTRUCT : TYUNION, loc2);
    } else if (l.tok.tok != TLcurlyBracket) {
      return parse_error(getLoc(), "expect '{' for start anonymous struct/union"), nullptr;
    }
    CType result = TNEW(StructType) {.sname = Name, .selems = xvector<Declator>::get()};
    consume();
    if (l.tok.tok != TRcurlyBracket) {
      for (;;) {
        CType base;
        if (l.tok.tok == K_Static_assert) {
          if (!consume_static_assert())
            return nullptr;
          if (l.tok.tok == TRcurlyBracket)
            break;
          continue;
        }
        if (!(base = specifier_qualifier_list()))
          return expect(getLoc(), "specifier-qualifier-list"), nullptr;
        if (l.tok.tok == TSemicolon) {
          consume();
          continue;
        } else {
          for (;;) {
            Declator e = struct_declarator(base);
            if (!e.ty)
              return parse_error(getLoc(), "expect struct-declarator"), nullptr;
            if (e.name) {
              for (const auto &p : result->selems) {
                if (p.name == e.name) {
                  type_error(getLoc(), "duplicate member %I", e.name);
                  break;
                }
              }
            }
            result->selems.push_back(e);
            if (l.tok.tok == TComma)
              consume();
            else {
              checkSemicolon();
              break;
            }
            if (l.tok.tok == TRcurlyBracket) {
              consume();
              break;
            }
          }
        }
      }
    } else
      consume();
    result->sidx = puttag(Name, result, loc, tok == Kstruct ? TYSTRUCT : TYUNION);
    return result;
  }
  CType enum_decl() {
    // parse a enum, return it
    // for example:
    //
    //      `enum State`
    //
    //      `enum { ... }`
    //
    //      `enum State { ... }`
    Location loc = getLoc();
    IdentRef Name = nullptr;
    consume(); // eat enum
    if (l.tok.tok == TIdentifier) {
      Location loc2 = getLoc();
      Name = l.tok.s;
      consume();
      if (l.tok.tok != TLcurlyBracket) // struct Foo
        return gettagByName(Name, TYENUM, loc2);
    } else if (l.tok.tok != TLcurlyBracket)
      return parse_error(loc, "expect '{' for start anonymous enum"), nullptr;
    CType result = TNEW(EnumType) {.ename = Name};
    if (!irgen.options.g)
      result->eelems = xvector<EnumPair>::get();
    consume();
    for (uint64_t c = 0;; c++) {
      if (l.tok.tok != TIdentifier)
        break;
      IdentRef s = l.tok.s;
      consume();
      if (l.tok.tok == TAssign) {
        Expr e;
        consume();
        if (!(e = constant_expression()))
          return nullptr;
        if (e->k == EConstant) {
          const APInt &I = cast<ConstantInt>(e->C)->getValue();
          if (I.getActiveBits() > 32) {
            warning("enum constant exceeds 32 bit");
          }
          c = I.getLimitedValue();
        }
      }
      putenum(s, c, loc);
      if (irgen.options.g)
        result->eelems.push_back(EnumPair {.name = s, .val = c});
      if (l.tok.tok == TComma)
        consume();
      else
        break;
    }
    if (l.tok.tok != TRcurlyBracket)
      parse_error(getLoc(), "expect '}'");
    consume();
    result->eidx = puttag(Name, result, loc, TYENUM);
    return result;
  }
  bool parameter_type_list(xvector<Param> &params, bool &isVararg) {
    Location loc = getLoc();
    bool ok = true;
    assert(params.empty());
    enterBlock(); // function prototype scope
    for (unsigned i = 1;; i++) {
      if (l.tok.tok == TEllipsis) {
        consume(), isVararg = true;
        break;
      }
      if (l.tok.tok == TRbracket)
        break;
      if (!istype(l.tok.tok) && l.tok.tok != TIdentifier) // TODO: old style
        return type_error(getLoc(), "expect a type or old-style variable name"), false;
      CType base = declaration_specifiers();
      if (!base)
        return expect(getLoc(), "declaration-specifiers"), false;
      Declator nt = abstract_decorator(base, Function);
      if (!nt.ty)
        return expect(getLoc(), "abstract-decorator"), false;
      params.push_back(nt);
      if (nt.ty->k == TYINCOMPLETE)
        type_error(getLoc(), "parameter %u has imcomplete type %T", i, nt.ty), ok = false;
      nt.ty->tags |= TYPARAM;
      if (nt.name) {
        if (sema.typedefs.getSymInCurrentScope(nt.name))
          type_error("duplicate parameter %I", nt.name);
        sema.typedefs.putSym(nt.name, Variable_Info {.ty = nt.ty, .loc = getLoc(), .tags = ASSIGNED | USED});
      }
      if (l.tok.tok == TComma)
        consume();
    }
    bool zero = false;
    for (size_t i = 0; i < params.size(); ++i) {
      if (i == 0 && (params[0].ty->tags & TYVOID))
        zero = true;
      if (params[i].ty->k == TYARRAY)
        params[i].ty = context.getPointerType(params[i].ty->arrtype);
    }
    if (zero) {
      if (params.size() > 1)
        warning(loc, "'void' must be the only parameter");
      params.clear();
    }
    leaveBlock();
    return ok;
  }
  void checkAlign(uint32_t a) {
    if (a & (a - 1))
      type_error(getLoc(), "requested alignment is not a power of 2");
  }
  uint64_t force_eval(Expr e, Location cloc) {
      if (e->k != EConstant) {
          type_error(cloc, "not a constant expression: %E", e);
          return 0;
      }
      if (auto CI = dyn_cast<ConstantInt>(e->C)) {
          if (CI->getValue().getActiveBits() > 64)
              warning(cloc, "integer constant expression larger exceeds 64 bit, the result is truncated");
          return CI->getValue().getLimitedValue();
      }
      type_error("not a integer constant: %E", e);
      return 0;
  }
  uint64_t try_eval(Expr e, Location cloc, bool &ok) {
    if (e->k == EConstant) {
      if (auto CI = dyn_cast<ConstantInt>(e->C)) {
          if (CI->getValue().getActiveBits() > 64)
              warning(cloc, "integer constant expression larger exceeds 64 bit, the result is truncated");
          return CI->getValue().getLimitedValue();
      }
    }
    return 0;
  }
  bool try_eval_as_bool(Expr e, bool &res) {
    if (e->k == EConstant) {
      if (auto CI = dyn_cast<ConstantInt>(e->C)) {
          res = !CI->isZero();
          return true;
      }
    }
    return false;
  }
  bool parse_alignas() {
    consume();
    if (l.tok.tok != TLbracket)
      return expectLB(getLoc()), false;
    consume();
    if (istype(l.tok.tok)) {
      uint32_t a;
      CType ty = type_name();
      if (!ty)
        return false;
      a = getAlignof(ty);
      if (a == 0)
        type_error(getLoc(), "zero alignment is not valid");
      else {
        checkAlign(a);
        sema.currentAlign = a;
      }
    } else {
      uint32_t a;
      Location cloc = getLoc();
      Expr e = expression();
      if (!e)
        return false;
      a = force_eval(e, cloc);
      if (e->ty->isSigned() && (int64_t)a <= 0)
        type_error(getLoc(), "alignment %u too small", (unsigned)a);
      else {
        checkAlign(a);
        sema.currentAlign = a;
      }
    }
    if (l.tok.tok != TRbracket)
      return expectRB(getLoc()), false;
    consume();
    return true;
  }
  Stmt parse_asm() {
    Stmt result;
    consume(); // eat asm
    if (l.tok.tok != TLbracket)
      return expectLB(getLoc()), nullptr;
    consume(); // eat '('
    if (l.tok.tok != TStringLit)
      return expect(getLoc(), "string literal"), nullptr;
    result = SNEW(AsmStmt){.loc = getLoc(), .asms = l.tok.str};
    consume(); // eat string
    if (l.tok.tok != TRbracket)
      return expectRB(getLoc()), nullptr;
    consume(); // eat ')'
    return result;
  }
  bool consume_static_assert() {
    uint32_t ok;
    Location loc = getLoc();
    consume();
    if (l.tok.tok != TLbracket)
      return expectLB(getLoc()), false;
    consume();
    Expr e = constant_expression();
    if (!e)
      return false;
    ok = force_eval(e, loc);
    if (l.tok.tok == TRbracket) { // no message
      if (ok == 0)
        parse_error(loc, "%s", "static assert failed!");
      consume();
    } else if (l.tok.tok == TComma) {
      consume();
      if (l.tok.tok != TStringLit)
        return expect(loc, "string literal in static assert"), false;
      if (!ok)
        parse_error(loc, "static assert failed: %s", l.tok.str.data());
      consume();
      if (l.tok.tok != TRbracket)
        return expectRB(getLoc()), false;
      consume();
    } else
      return expect(getLoc(), "',' or ')'"), false;
    return checkSemicolon(), true;
  }
  void clearKnownConstantVariables() {
    for (auto it = sema.typedefs.current_function();it != sema.typedefs.end();++it) {
      if ((it->info.ty->tags & TYCONST) == 0)
        it->info.val = nullptr;
    }
  }
  bool assignable(Expr e, Variable_Info *&info) {
    if (e->k == EVar) {
      Variable_Info &var_info = sema.typedefs.getSym(e->sval);
      var_info.tags |= ASSIGNED;
      info = &var_info;
      return true;
    }
    if (e->ty->k == TYPOINTER) {
      if ((e->ty->tags & TYLVALUE) && e->ty->p->k == TYARRAY)
        return type_error(getLoc(), "array is not assignable"), false;
      return true;
    }
    if (e->ty->tags & TYLVALUE)
      return true;
    return type_error(getLoc(), "expression is not assignable"), false;
  }
  void declaration() {
    // parse declaration or function definition
    CType base;
    Stmt result;
    Location loc = getLoc();
    sema.currentAlign = 0;
    if (l.tok.tok == TSemicolon)
      return;
    if (!(base = declaration_specifiers()))
      return expect(loc, "declaration-specifiers");
    if (sema.currentAlign) {
      size_t m = getAlignof(base);
      if (sema.currentAlign < m) {
        type_error(loc, "requested alignment is less than minimum alignment of %Z for type %T", m, base);
      } else {
        base->align = sema.currentAlign;
      }
    }
    if (l.tok.tok == TSemicolon) {
      consume();
      if (base->align)
        warning(loc, "'_Alignas' can only used in variables");
      return insertStmt(SNEW(DeclOnlyStmt) {.loc = loc, .decl = base});
    }
    result = SNEW(VarDeclStmt) {.loc = loc, .vars = xvector<VarDecl>::get()};
    for (;;) {
      Location loc2 = getLoc();
      Declator st = declarator(base, Direct);
      if (!st.ty)
        return;
      if (l.tok.tok == TLcurlyBracket) {
        if (st.ty->k != TYFUNCTION)
          return type_error(loc2, "unexpected function definition");
        // function definition
        if (st.name->second.getToken() == PP_main) {
          if (irgen.options.libtrace) {
            if (st.ty->params.size() < 2) {
              type_error("main should be at lease two arguments(argc, argv) when using libtrace.");              
            }
          }
          if (st.ty->params.size()) {
            if (!(st.ty->params.front().ty->tags & TYINT)) {
              type_error("first parameter of main is not 'int'");
            }
            if (st.ty->params.size() >= 2) {
              if (!(
                  st.ty->params[1].ty->k == TYPOINTER && 
                  st.ty->params[1].ty->p->k == TYPOINTER && 
                  st.ty->params[1].ty->p->p->tags & (TYCHAR | TYUCHAR))
                ) {
                type_error("second parameter of main is not 'char**'");
              }
            }
          }
        }
        size_t idx = putsymtype2(st.name, st.ty);
        sema.pfunc = st.name;
        if (!isTopLevel())
          return parse_error(loc2, "function definition is not allowed here"), note("function can only declared in global scope");
        Stmt res = SNEW(FunctionStmt) {
          .loc = loc,
          .func_idx = idx,
          .funcname = st.name, 
          .functy = st.ty,
          .args = xvector<size_t>::get_with_length(st.ty->params.size())
        };
        {
          llvm::SaveAndRestore<CType> saved_ret(sema.currentfunctionRet, st.ty->ret);
          res->funcbody = function_body(st.ty->params, res->args, loc);
        }
        res->numLabels = jumper.cur;
        jumper.cur = 0;
        sreachable = true;
        return insertStmt(res);
      }
      noralizeType(st.ty);
#if 0
      logtime();
      print_cdecl(st.name->getKey(), st.ty, llvm::errs(), true);
#endif
      size_t idx = putsymtype(st.name, st.ty);

      result->vars.push_back(VarDecl {.name = st.name, .ty = st.ty, .init = nullptr, .idx = idx});
      if (st.ty->tags & TYINLINE)
        warning(loc2, "inline can only used in function declaration");
      if (!(st.ty->tags & TYEXTERN)) {
        if ((st.ty->tags & TYVOID) && !(st.ty->tags & TYTYPEDEF))
          return type_error(loc2, "variable %I declared void", st.name);
        if (st.ty->k == TYINCOMPLETE)
          return type_error(loc2, "variable %I has imcomplete type %T", st.name);
      }
      if (l.tok.tok == TAssign) {
        Expr init;
        auto &var_info = sema.typedefs.getSym(idx);
        var_info.tags |= ASSIGNED;
        if (st.ty->k == TYARRAY && st.ty->vla) {
          return type_error(loc2, "variable length array may not be initialized");
        } else if (st.ty->k == TYFUNCTION)
          return type_error(loc2, "function may not be initialized");
        if (st.ty->tags & TYTYPEDEF)
          return type_error(loc2, "'typedef' may not be initialized");
        consume();
        {
          CType old = sema.currentInitTy;
          sema.currentInitTy = st.ty;
          init = initializer_list();
          sema.currentInitTy = old;
        }
        if (!init)
          return expect(loc2, "initializer-list");
        result->vars.back().init = init;
        if (init->k == EConstant && (isTopLevel() ? bool(st.ty->tags & TYCONST) : true)) {
            var_info.val = init->C;
        }
        if (isTopLevel() && !isConstant(init)) {
          type_error(loc2, "initializer element is not constant");
          note("global variable requires constant initializer");
        }
      } else {
        if (st.ty->k == TYARRAY) {
          if (st.ty->vla && isTopLevel())
            type_error(loc2, "variable length array declaration not allowed at file scope");
          else if (st.ty->hassize == false && !st.ty->vla && (st.ty->tag & TYEXTERN)) {
            if (isTopLevel()) {
              warning(loc2, "array %I assumed to have one element", st.name);
              st.ty->hassize = true, st.ty->arrsize = 1;
            } else {
              type_error(loc2, "array size missing in %I", st.name);
            }
          }
        }
      }
      if (l.tok.tok == TComma) {
        consume();
      } else if (l.tok.tok == TSemicolon) {
        consume();
        break;
      }
    }
    return insertStmt(result);
  }
  Expr cast_expression() {
    if (l.tok.tok == TLbracket) {
      consume();
      if (istype(l.tok.tok)) {
        CType ty;
        Expr e;
        if (!(ty = type_name()))
          return expect(getLoc(), "type-name"), nullptr;
        if (l.tok.tok != TRbracket)
          return expectRB(getLoc()), nullptr;
        consume();
        if (l.tok.tok == TLcurlyBracket) {
          Expr result;
          CType old = sema.currentInitTy;
          sema.currentInitTy = ty;
          result = initializer_list();
          sema.currentInitTy = old;
          return result;
        }
        if (!(e = cast_expression()))
          return nullptr;
        if (ty->tags & TYVOID)
          return ENEW(VoidExpr){
              .loc = e->loc, .ty = context.typecache.v, .voidexpr = e};
        return castto(e, ty);
      }
      l.tokenq.push_back(l.tok);
      l.tok = TLbracket;
    }
    return unary_expression();
  }
  CType type_name() {
    // parse a type-name(abstract, has no name)
    //
    // for example:
    //
    //    sizeof(type-name)
    CType base;
    if (!(base = declaration_specifiers()) || l.tok.tok == TRbracket)
      return base;
    return abstract_decorator(base, Abstract).ty;
  }
  Expr unary_expression() {
    Location loc = getLoc();
    Token tok = l.tok.tok;
    switch (tok) {
      case TNot: 
      {
        Expr e;
        consume();
        if (!(e = cast_expression()))
          return nullptr;
        valid_condition(e, true);
        return boolToInt(unary(e, LogicalNot, context.getInt()));
      }
    case TMul: 
    {
        Expr e;
        CType ty;
        consume();
        if (!(e = cast_expression()))
          return nullptr;
        if (e->ty->k != TYPOINTER)
          return type_error(getLoc(), "pointer expected"), nullptr;
        ty = context.clone(e->ty->p);
        ty->tags |= TYLVALUE;
        return unary(e, Dereference, ty);
    }
    case TBitNot: 
    {
      Expr e;
      consume();
      if (!(e = cast_expression()))
        return nullptr;
      if (!checkInteger(e->ty))
        return type_error(getLoc(), "integer type expected"), nullptr;
      integer_promotions(e);
      if (e->k == EConstant) // fold simple bitwise-not, e.g., ~0ULL
        return wrap(e->ty, llvm::ConstantExpr::getNot(e->C));
      return unary(e, Not, e->ty);;
    }
    case TBitAnd: 
    {
      Expr e;
      consume();
      if (!(e = expression()))
        return nullptr;
      if (e->k == EString)
        return e->ty->tags &= ~TYLVALUE, e;
      if (e->k == EArrToAddress)
        return e->ty = context.getPointerType(e->voidexpr->ty), e;
      if (e->ty->k == TYBITFIELD)
        return type_error(getLoc(), "cannot take address of bit-field"), getIntZero();
      if (e->ty->tags & TYREGISTER)
        return type_error(getLoc(), "take address of register variable"), getIntZero();
      if (e->k == EUnary && e->uop == AddressOf && e->ty->p->k == TYFUNCTION)
        return e->ty->tags &= ~TYLVALUE, e;
      Variable_Info *info = nullptr;
      if (!assignable(e, info))
        return getIntZero();
      if (info)
        info->val = nullptr;
      return unary(e, AddressOf, context.getPointerType(e->ty));
    }
    case TDash: 
    {
      Expr e;
      consume();
      if (!(e = unary_expression()))
        return nullptr;
      if (!checkArithmetic(e->ty))
        return type_error(getLoc(), "arithmetic type expected"), nullptr;
      integer_promotions(e);
      if (e->k == EConstant) // fold simple negate numbers, e.g, -10
        return wrap(e->ty, e->ty->isSigned() ? llvm::ConstantExpr::getNSWNeg(e->C) : llvm::ConstantExpr::getNeg(e->C), e->loc);
      return unary(e, e->ty->isSigned() ? SNeg : UNeg, e->ty);
    }
    case TAdd:
    {
      Expr e;
      consume();
      if (!(e = unary_expression()))
        return nullptr;
      if (!checkArithmetic(e->ty))
        return type_error(getLoc(), "arithmetic type expected"), nullptr;
      integer_promotions(e);
      return e;
    }
    case TAddAdd:
    case TSubSub: 
    {
      Expr e;
      Variable_Info *info = nullptr;
      consume();
      if (!(e = unary_expression()))
        return nullptr;
      if (!assignable(e, info))
        return nullptr;
      if (info)
        info->val = nullptr;
      CType ty = e->ty->k == TYPOINTER ? context.getSize_t() : e->ty;
      Expr obj = e;
      Expr one = wrap(ty, ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), 1)), e->loc);
      (tok == TAddAdd) ? make_add(e, one) : make_sub(e, one);
      return binop(obj, Assign, e, e->ty);
    }
    case Ksizeof: 
    {
      Expr e;
      consume();
      if (l.tok.tok == TLbracket) {
        consume();
        if (istype(l.tok.tok)) {
          CType ty = type_name();
          if (!ty)
            return expect(getLoc(), "type-name or expression"), nullptr;
          if (l.tok.tok != TRbracket)
            return expectRB(getLoc()), nullptr;
          consume();
          return wrap(context.getSize_t(),  ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getsizeof(ty))), loc);
        }
        if (!(e = unary_expression()))
          return nullptr;
        if (l.tok.tok != TRbracket)
          return expectRB(getLoc()), nullptr;
        consume();
        return wrap(context.getSize_t(), ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getsizeof(e))), loc);
      }
      if (!(e = unary_expression()))
        return nullptr;
      return wrap(context.getSize_t(),  ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getsizeof(e))), loc);
    }
    case K_Alignof: 
    {
      Expr result;
      consume();
      if (l.tok.tok != TLbracket)
        return expectLB(getLoc()), nullptr;
      consume();
      if (istype(l.tok.tok)) {
        CType ty = type_name();
        if (!ty)
          return expect(getLoc(), "type-name"), nullptr;
        result = wrap(context.getSize_t(), ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getAlignof(ty))), loc);
      } else {
        Expr e = constant_expression();
        if (!e)
          return nullptr;
        result = wrap(context.getSize_t(), ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getAlignof(e))), loc);
      }
      if (l.tok.tok != TRbracket)
        return expectRB(getLoc()), nullptr;
      consume();
      return result;
    }
    default:
      return postfix_expression();
    }
}
static bool alwaysFitsInto64Bits(unsigned Radix, unsigned NumDigits) {
    switch (Radix) {
        case 2:
            return NumDigits <= 64;
        case 8:
            return NumDigits <= 64 / 3;
        case 10:
            return NumDigits <= 19;
        case 16:
            return NumDigits <= 64 / 4;
        default:
            llvm_unreachable("impossible Radix");
    }
}

// the input must be null-terminated
Expr parse_pp_number(const xstring str) {
    Location loc = getLoc();
    const unsigned char *DigitsBegin = nullptr;
    bool isFPConstant = false;
    uint64_t radix;
    const unsigned char *s = reinterpret_cast<const unsigned char*>(str.data());
    if (str.size() == 2) {
        if (!llvm::isDigit(str.front()))
            return lex_error(loc, "expect one digit"), nullptr;
        return wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt(32, static_cast<uint64_t>(*s) - '0')));
    }
    if (*s == '0') {
        s++;
        switch (*s) {
            case '.':
                DigitsBegin = s;
                goto common;
            case 'X':
            case 'x':
                ++s;
                if (!(llvm::isHexDigit(*s) || *s == '.'))
                    return lex_error(loc, "expect hexdecimal digits"), nullptr;
                DigitsBegin = s;
                radix = 16;
                do 
                    s++;
                while (llvm::isHexDigit(*s));
                goto common;
            case 'b':
            case 'B':
                ++s;
                if (*s != '0' && *s != '1')
                    return lex_error(loc, "expect binary digits"), nullptr;
                DigitsBegin = s;
                radix = 2;
                do 
                    s++;
                while (*s == '0' || *s == '1');
                goto common;
            default:
                if (*s < '0' || *s > '7')
                    return lex_error(loc, "expect octal digits"), nullptr;
                DigitsBegin = s;
                radix = 8;
                do 
                    s++;
                while (*s >= '0' && *s <= '7');
                goto common;
        }
    } else {
        radix = 10;
        DigitsBegin = s;
        while (llvm::isDigit(*s))
            s++;
        common:
        if (*s != '\0') {
            if (llvm::isHexDigit(*s) && *s != 'E' && *s != 'e')
                return lex_error(loc, "invalid digit '%c'", *s), nullptr;
            if (*s == '.') {
                if (radix == 2)
                  return lex_error(loc, "binary literal cannot be floating constant"), nullptr;
                isFPConstant = true;
                do
                    s++;
                while (llvm::isDigit(*s));
            }
            if (*s == 'E' || *s == 'e') {
                if (radix == 16)
                    return lex_error(loc, "hexdecimal constant requires 'p' or 'P' exponent"), nullptr;
                goto READ_EXP;
            }
            if (*s == 'P' || *s == 'p') {
                if (radix != 16)
                    return lex_error(loc, "exponent 'p' and 'p' can only used in hexdecimal constant"), nullptr;
                READ_EXP:
                isFPConstant = true;
                ++s;
                if (*s == '+' || *s == '-') s++;
                if (!llvm::isDigit(*s))
                    return lex_error(loc, "expect exponent digits"), nullptr;
                do 
                    s++;
                while (llvm::isDigit(*s));
            }
        }
    }
    struct Suffixs {
        bool isUnsigned: 1,
             isLongLong: 1,
             isLong: 1,
             isFloat: 1,
             isFloat128: 1,
             HasSize: 1,
             isImaginary: 1;
    } su{};
    const unsigned char *SuffixBegin = s;
    for (;;) {
        unsigned char c1;
        switch ((c1 = *s++)) {
            case '\0': goto NEXT;
            case 'f':
            case 'F':
                if (!isFPConstant || su.HasSize) break;
                su.HasSize = su.isFloat = true;
                continue;
            case 'u':
            case 'U':
                if (isFPConstant || su.isUnsigned) break;
                su.isUnsigned = true;
                continue;
            case 'l':
            case 'L':
                if (su.HasSize) break;
                su.HasSize = true;
                if (c1 == *s) {
                    if (isFPConstant) break;
                    su.isLongLong = true;
                    ++s;
                } else 
                    su.isLong = true;
                continue;
            case 'i':
            case 'I':
            case 'j':
            case 'J':
                if (su.isImaginary) break;
                su.isImaginary = true;
                continue;
            case 'q':
            case 'Q':
                if (!isFPConstant || su.HasSize) break;
                su.isFloat128 = true;
                continue;
        }
        lex_error(loc, "invalid suffix '%c' in %s constant", c1, isFPConstant ? "floating" : "integer");
        break;
    }
    NEXT:
    const uintptr_t NumDigits = (uintptr_t)SuffixBegin - (uintptr_t)DigitsBegin;
    if (isFPConstant) {
        StringRef fstr(str.data(), std::min(
            (uintptr_t)str.size() - 1,
            (uintptr_t)SuffixBegin - (uintptr_t)str.data()
        ));
        CType ty = context.typecache.fdoublety;
        const llvm::fltSemantics *Format = &llvm::APFloat::IEEEdouble();
        if (su.isFloat128) {
            ty = context.typecache.f128ty;
            Format = &APFloat::IEEEquad();
        } else if (su.isFloat) {
            Format = &APFloat::IEEEsingle();
            ty = context.typecache.ffloatty;
        }
        APFloat F(*Format);
        auto it = F.convertFromString(fstr, APFloat::rmNearestTiesToEven);
        if (auto err = it.takeError()) {
            std::string msg = llvm::toString(std::move(err));
            lex_error(loc, "error parsing floating literal: %R", StringRef(msg));
        } else {
            auto status = *it;
            SmallString<20> buffer;
            if ((status & APFloat::opOverflow) ||
                ((status & APFloat::opUnderflow) && F.isZero())) {
                const char *diag;
                if (status & APFloat::opOverflow) {
                    APFloat::getLargest(*Format).toString(buffer);
                    diag = "floating point constant overflow: %R";
                } else {
                    APFloat::getSmallest(*Format).toString(buffer);
                    diag = "float point constant opUnderflow: %R";
                }
                lex_error(loc, diag, buffer.str());
            }
        }
        return wrap(ty, ConstantFP::get(irgen.ctx, F), loc);
    }
    bool overflow = false;
    xint128_t bigVal = xint128_t::getZero();
    if (alwaysFitsInto64Bits(radix, NumDigits)) {
        uint64_t N = 0;
        for (auto Ptr = DigitsBegin; Ptr < SuffixBegin; ++Ptr)
            N = N * radix + llvm::hexDigitValue(*Ptr);
        bigVal.setLow(N);
    } else {
        for (auto Ptr = DigitsBegin; Ptr < SuffixBegin; ++Ptr) {
            overflow |= umul_overflow(bigVal, radix, bigVal);
            overflow |= uadd_overflow(bigVal, llvm::hexDigitValue(*Ptr));
        }
        if (overflow)
          warning(loc, "integer literal is too large to be represented in any integer type(exceed 128 bits)");
    }
    CType ty = context.getInt();
    uint64_t H = bigVal.high(),
             L = bigVal.low();
    if (su.isLongLong) {
        ty = su.isUnsigned ? context.getULongLong() : context.getLongLong();
        if (bigVal >> ty->getBitWidth())
          warning("integer constant is too large with 'll' suffix");
    } else if (su.isLong) {
        ty = su.isUnsigned ? context.getULong() : context.getLong();
        if (bigVal >> ty->getBitWidth())
          warning("integer constant is too large with 'l' suffix");
    } else {
        if (H) {
          /* 128 bits */
            if ((int64_t)H < 0)
              ty = context.typecache.u128;
            else
              ty = context.typecache.i128;
        } else {
          /* 64 bits */
          if ((int64_t)L < 0) /* test MSB */
            ty = context.typecache.u64;
          else {
            if (L >> 32) { /* more than 32 bits ? */
              ty = context.typecache.i64;
            } else { /* 32 bits */
              if ((int32_t)L < 0) /* test MSB */
                ty = context.typecache.u32;
              else
                ty = context.typecache.i32;
            }
          }
        }
    }
    if (H)
      return wrap(ty, ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), {H, L})));
    return wrap(ty, ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), L)));
}
  Expr wrap(CType ty, llvm::Constant *C, Location loc = Location()) {
    return ENEW(ConstantExpr) {.loc = loc, .ty = ty, .C = C};
  }
  Expr getIntZero() {
    return intzero;
  }
  Expr getIntOne() {
    return intone;
  }
  Expr primary_expression() {
    // primary expressions:
    //      constant
    //      `(` expression `)`
    //      identfier
    Expr result;
    Location loc = getLoc();
    switch (l.tok.tok) {
    case TCharLit: 
    {
      CType ty;
      switch (l.tok.itag) {
      case Iint: // Iint: 'c'
        ty = context.getInt();
        break;
      case Ilong: // Ilong: u'c'
        ty = context.typecache.u16;
        break;
      case Iulong: // Iulong: U'c'
        ty = context.typecache.u32;
        break;
      case Ilonglong: // Ilonglong: L'c'
        ty = context.getWchar();
        break;
      default:
        llvm_unreachable("");
      }
      result = wrap(
        ty, 
        ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), l.tok.i)), 
        loc);
      consume();
    } break;
    case TStringLit: 
    {
      xstring s = l.tok.str;
      auto enc = l.tok.enc;
      consume();
      while (l.tok.tok == TStringLit) {
        s.push_back(l.tok.str);
        if (l.tok.enc != enc)
          type_error(getLoc(), "unsupported non-standard concatenation of string literals for UTF-%u and UTF-%u", (unsigned)enc, (unsigned)l.tok.enc);
        consume();
      }
      switch (enc) {
      case 8:
        result = ENEW(ArrToAddressExpr) {.loc = loc, .ty = context.typecache.strty,.arr3 = ENEW(StringExpr){ .ty = TNEW(ArrayType){.arrtype = context.typecache.i8,.hassize = true,.arrsize = (unsigned)s.size()},.str = s,.is_constant = true}};
        break;
      default:
        llvm_unreachable("bad encoding");
      }
    } break;
    case PPNumber: 
    {
      result = parse_pp_number(l.tok.str);
      if (result)
        result->loc = getLoc();
      else
        result = getIntZero();
      consume();
    } break;
    case TIdentifier: 
    {
      if (l.want_expr)
        result = getIntZero();
      else if (l.tok.s->second.getToken() == PP__func__) {
        CType ty = TNEW(ArrayType) {.arrtype = context.typecache.i8, .hassize = true, .arrsize = (unsigned)sema.pfunc->getKeyLength()};
        result = ENEW(ArrToAddressExpr) {.loc = loc, .ty = context.typecache.strty, .arr3 = ENEW(StringExpr){.ty = ty, .str = xstring::get(sema.pfunc->getKey()), .is_constant = true}};
      } else {
        IdentRef sym = l.tok.s;
        size_t idx;
        Variable_Info *it = sema.typedefs.getSym(l.tok.s, idx);
        consume();
        if (!it)
          return type_error(loc, "use of undeclared identifier %I", sym), getIntZero();
        if (it->ty->tags & TYTYPEDEF)
          return type_error("typedefs are not allowed here %I", sym), getIntZero();
        it->tags |= USED;
        CType ty = context.clone(it->ty);
        // lvalue conversions
        ty->tags &= ~(TYCONST | TYRESTRICT | TYVOLATILE | TYATOMIC | TYREGISTER | TYTHREAD_LOCAL | TYEXTERN | TYSTATIC | TYNORETURN | TYINLINE | TYPARAM);
        ty->tags |= TYLVALUE;
        if (it->val) 
          return wrap(ty, it->val, loc);
//        if (!(it->tags & ASSIGNED))
//          warning("use of variable %I has garbage value now");
        switch (ty->k) {
          case TYFUNCTION:
            result = unary(ENEW(VarExpr) {.loc = loc, .ty = ty, .sval = idx}, AddressOf, context.getPointerType(ty));
            break;
          case TYARRAY:
            result = ENEW(ArrToAddressExpr) {.loc = loc, .ty = context.getPointerType(ty->arrtype), .arr3 = ENEW(VarExpr) {.loc = loc, .ty = ty, .sval = idx}};
            break;
          default:
            result = ENEW(VarExpr) {.loc = loc, .ty = ty, .sval = idx};
        }
      }
    } break;
    case TLbracket: 
    {
      consume();
      if (!(result = expression()))
        return nullptr;
      if (l.tok.tok != TRbracket)
        return expectRB(getLoc()), nullptr;
      consume();
    } break;
    case K_Generic: 
    {
      Expr test, defaults, e;
      CType testty, tname;
      consume();
      if (l.tok.tok != TLbracket)
        return expectLB(loc), nullptr;
      consume();
      if (!(test = assignment_expression()))
        return nullptr;
      if (l.tok.tok != TComma)
        return expect(loc, "','"), nullptr;
      consume();
      for (testty = test->ty;;) {
        tname = nullptr;
        if (l.tok.tok == Kdefault) {
          if (defaults)
            return type_error(loc, "more then one default case in Generic expression"), nullptr;
          consume();
        } else if (!(tname = type_name()))
          return expect(loc, "type-name"), nullptr;
        if (l.tok.tok != TColon)
          return expect(loc, "':'"), nullptr;
        consume();
        if (!(e = assignment_expression()))
          return nullptr;
        if (!tname)
          defaults = e;
        else if (compatible(tname, testty)) {
          if (result)
            return type_error(loc,"more then one compatible types in Generic expression"), nullptr;
          result = e;
        }
        switch (l.tok.tok) {
        case TComma:
          consume();
          continue;
        case TRbracket:
          consume();
          goto GENERIC_END;
        default:
          return expect(loc, "','' or ')'"), nullptr;
        }
      }
    GENERIC_END:
      if (result)
        return result;
      if (defaults)
        return defaults;
      return type_error(loc, "Generic expression(has type %T) not compatible with any generic association type", testty), nullptr;
    } break;
    default:
      return parse_error(loc, "unexpected token in primary_expression: %s\n", show(l.tok.tok)), consume(), nullptr;
    }
    return result;
  }
  Expr postfix_expression() {
    bool isarrow;
    PostFixOp isadd;
    Expr result = primary_expression();
    if (!result)
      return nullptr;
    for (;;) {
      switch (l.tok.tok) {
      case TSubSub:
        isadd = PostfixDecrement;
        goto ADD;
      case TAddAdd: 
      {
        isadd = PostfixIncrement;
      ADD:
      {
        Variable_Info *info = nullptr;
        if (!assignable(result, info))
          return nullptr;
        consume();
        if (info)
          info->val = nullptr;
        if (result->k == EConstant) {
          if (const auto CI = dyn_cast<ConstantInt>(result->C)) {
            APInt I = CI->getValue();
            if (isadd == PostfixIncrement) 
              info->val = ConstantInt::get(irgen.ctx, ++I);
            else
              info->val = ConstantInt::get(irgen.ctx, --I);
          }
          else if (const auto CFP = dyn_cast<ConstantFP>(result->C)) {
            APFloat F = CFP->getValue();
            if (isadd == PostfixIncrement) 
              F.add(APFloat(F.getSemantics(), 1), APFloat::rmNearestTiesToEven);
            else
              F.subtract(APFloat(F.getSemantics(), 1), APFloat::rmNearestTiesToEven);
            info->val = ConstantFP::get(irgen.ctx, F);
          }
        }
        result = ENEW(PostFixExpr) {.loc = result->loc, .ty = result->ty, .pop = isadd, .poperand = result};
      }
      }
        continue;
      case TArrow:
        isarrow = false;
        goto DOT;
      case TDot: {
        isarrow = true;
      DOT:
        bool isLvalue = false;
        consume();
        if (l.tok.tok != TIdentifier)
          return expect(getLoc(), "identifier"), nullptr;
        if (isarrow) {
          if (result->ty->k != TYPOINTER)
            return type_error(getLoc(),"pointer member access('->') requires a pointer"), nullptr;
          result = unary(result, Dereference, result->ty->p);
          isLvalue = true;
        }
        if (result->ty->k != TYSTRUCT && result->ty->k != TYUNION)
          return type_error(getLoc(), "member access is not struct or union"),
                 nullptr;

        for (size_t i = 0; i < result->ty->selems.size(); ++i) {
          Declator pair = result->ty->selems[i];
          if (l.tok.s == pair.name) {
            result = ENEW(MemberAccessExpr){.loc = result->loc,.ty = pair.ty,.obj = result,.idx = (uint32_t)i};
            if (isLvalue) {
              result->ty = context.clone(result->ty);
              result->ty->tags |= TYLVALUE;
            }
            consume();
            goto CONTINUE;
          }
        }
        return type_error(getLoc(), "struct/union %I has no member %I", result->ty->sname, l.tok.s), nullptr;
      CONTINUE:;
      } break;
      case TLbracket: // function call
      {
        CType ty = (result->k == EUnary && result->uop == AddressOf) ? result->ty->p : ((result->ty->k == TYPOINTER) ? result->ty->p : result->ty);
        Expr f = result;
        result = ENEW(CallExpr){.loc = result->loc, .ty = ty->ret, .callfunc = f, .callargs = xvector<Expr>::get()};
        if (ty->k != TYFUNCTION)
          return type_error(getLoc(), "expect function or function pointer, but the expression has type %T",ty), nullptr;
        consume();
        if (l.tok.tok == TRbracket)
          consume();
        else {
          for (;;) {
            Expr e = assignment_expression();
            if (!e)
              return nullptr;
            result->callargs.push_back(e);
            if (l.tok.tok == TComma)
              consume();
            else if (l.tok.tok == TRbracket) {
              consume();
              break;
            }
          }
        }
        auto &params = ty->params;
        if (ty->isVarArg) {
          if (result->callargs.size() < params.size())
            return type_error(
                       getLoc(),
                       "too few arguments to variable argument function"),
                   note("at lease %z arguments needed",
                        (size_t)(params.size() + 1)),
                   nullptr;
          for (size_t j = 0; j < params.size(); ++j) {
            Expr e = castto(result->callargs[j], params[j].ty);
            if (!e)
              break;
            result->callargs[j] = e;
          }
          for (size_t j = params.size(); j < result->callargs.size(); ++j) {
            Expr e = result->callargs[j];
            default_argument_promotions(e);
            if (!e)
              break;
            result->callargs[j] = e;
          }
        } else {
          if (params.size() != result->callargs.size())
            return type_error(getLoc(), "expect %z parameters, %z arguments provided", (size_t)params.size(), (size_t)result->callargs.size()),
                   nullptr;
          for (size_t j = 0; j < result->callargs.size(); ++j) {
            Expr e = castto(result->callargs[j], params[j].ty);
            if (!e)
              break;
            result->callargs[j] = e;
          }
        }
      } break;
      case TLSquareBrackets: // array subscript
      {
        Expr rhs;
        if (result->ty->k != TYPOINTER)
          return type_error(getLoc(), "array subscript is not a pointer"), nullptr;
        consume();
        if (!(rhs = expression()))
          return nullptr;
        if (l.tok.tok != TRSquareBrackets)
          warning(getLoc(), "missing ']'");
        else
          consume();
        CType ty = result->ty->p;
        ty->tags |= TYLVALUE;
        result = ENEW(SubscriptExpr){
            .loc = result->loc, .ty = ty, .left = result, .right = rhs};
      } break;
      default:
        return result;
      }
    }
  }
  void tryContinue() {
    if (jumper.topContinue == INVALID_LABEL)
      type_error(getLoc(), "%s", "connot continue: no outer for/while/do-while");
  }
  Expr getBool(bool b) {
    return b ? getIntOne() : getIntZero();
  }
  void foldBool(Expr &e, bool reverse = false) {
    llvm::Constant *C = e->C;
    switch (C->getValueID()) {
      case llvm::Value::ConstantIntVal:
      {
        const auto CI = cast<ConstantInt>(C);
        e = getBool(reverse ? CI->isZero() : !CI->isZero());
      } break;
      case llvm::Value::ConstantFPVal:
      {
        const auto CFP = cast<ConstantFP>(C);
        e = getBool(reverse ? CFP->isZero() : CFP->getValue().isNonZero());
      } break;
      case llvm::Value::UndefValueVal:
      case llvm::Value::PoisonValueVal:
      {
        /* TODO: undefined value is true/false ? */
      } break;
      case llvm::Value::ConstantPointerNullVal:
      {
        e = getBool(reverse);
      } break;
      default: break;
    }
  }
  void valid_condition(Expr &e, bool reverse = false) {
    if (!e->ty->isScalar()) {
      type_error("conditions requires a scalar expression");
    }
    switch (e->k) {
      case EUnary:
        switch (e->uop) {
          case AddressOf:
          warning("the address of %E will always evaluate as 'true'", e->uoperand);
          e = reverse ?  getIntZero() : getIntOne();
          return;
          case UNeg:
          case SNeg:
          case FNeg:
          warning("unary operator %s in conditions can be removed", show(e->uop));
          e = e->uoperand;
          break;
          default: break;
        }
      default: break;
    }
    if (e->k == EConstant)
      foldBool(e, reverse);
  }
  Expr Bexpression() {
    Expr e;
    consume();
    if (l.tok.tok != TLbracket)
      return expectLB(getLoc()), nullptr;
    consume();
    if (!(e = expression()))
      return nullptr;
    valid_condition(e);
    if (l.tok.tok != TRbracket)
      return expectRB(getLoc()), nullptr;
    consume();
    return e;
  }
  void setInsertPoint(Stmt I) {
    InsertPt = I;
  }
  Stmt getInsertPoint() {
    return InsertPt;
  }
  void clearInsertPoint() {
    InsertPt = nullptr;
  }
  void jumpIfTrue(Expr test, label_t dst) {
    label_t thenBB = jumper.createLabel();
    insertStmt(SNEW(CondJumpStmt) {.loc = Location::make_invalid(), .test = test, .T = dst, .F = thenBB});
    return insertLabel(thenBB);
  }
  void jumpIfFalse(Expr test, label_t dst) {
    label_t thenBB = jumper.createLabel();
    insertStmt(SNEW(CondJumpStmt) {.loc = Location::make_invalid(), .test = test, .T = thenBB, .F = dst});
    return insertLabel(thenBB);
  }
  void insertBr(label_t L, Location loc = Location::make_invalid()) {
    clearKnownConstantVariables();
    return insertStmt(SNEW(GotoStmt) {.loc = loc, .location = L});
  }
  void insertLabel(label_t L, IdentRef Name = nullptr) {
    sreachable = true;
    clearKnownConstantVariables();
    return insertStmtInternal(SNEW(LabelStmt) {.loc = Location::make_invalid(), .label = L, .labelName = Name});
  }
  void insertStmtInternal(Stmt s) {
      InsertPt->next = s;
      InsertPt = s;
  }
  void insertStmt(Stmt s) {
    if (s) {
      if (sreachable) {
        insertStmtInternal(s);
        if (isTerminator(s->k)) {
            unreachable_reason = s;
            sreachable = false;
        }
      } else {
        if (unreachable_reason && s->loc.isValid()) {
          printf("this = %u\n", s->k);
          warning(s->loc, "this statement is unreachable");
          note(unreachable_reason->loc, "after this *terminator* statement is unreachable");
          unreachable_reason = nullptr;
        }
      }
    }
  }
  void insertStmtEnd() {
    InsertPt->next = nullptr;
  }
  void statement() {
    // parse a statement
    Location loc = getLoc();
    switch (l.tok.tok) {
    case TSemicolon:
      return consume();
    case K__asm__:
      return parse_asm(), checkSemicolon();
    case TLcurlyBracket:
      return compound_statement();
    /*case Kcase: {
      Expr e, c;
      Stmt body;
      consume();
      if (!(e = constant_expression()))
        return expect(loc, "constant-expression"), nullptr;
      if (l.tok.tok != TColon)
        return expect(loc, ":"), nullptr;
      consume();
      if (!(body = statement()))
        return nullptr;
      if (!(c = castto(e, sema.currentCase)))
        return nullptr;
      return SNEW(CaseStmt){.loc = loc, .case_stmt = body, .case_expr = c};
    }
    case Kdefault: {
      Stmt s;
      consume();
      if (l.tok.tok != TColon)
        return expect(loc, ":"), nullptr;
      consume();
      if (!(s = statement()))
        return nullptr;
      return SNEW(DefaultStmt){.loc = loc, .default_stmt = s};
    }*/
    case Kgoto: 
    {
      consume();
      if (l.tok.tok != TIdentifier)
        return expect(loc, "identifier");
      IdentRef Name = l.tok.s;
      label_t L = getLabel(Name);
      consume();
      checkSemicolon();
      return insertBr(L, loc);
    }
    case Kcontinue:
      tryContinue();
      consume();
      checkSemicolon();
      return insertBr(jumper.topContinue, loc);
    case Kbreak: 
    {
      consume();
      checkSemicolon();
      if (jumper.topBreak == INVALID_LABEL)
        type_error(getLoc(), "%s", "connot break: no outer for/while/switch/do-while");
      else {
        if (sreachable)
          jumper.used_breaks.insert(jumper.topBreak);
        return insertBr(jumper.topBreak, loc);
      }
      return;
    }
    case Kreturn: 
    {
      Expr e, r;
      consume();
      if (l.tok.tok == TSemicolon) {
        consume();
        if (sema.currentfunctionRet->tags & TYVOID)
          return insertStmt(SNEW(ReturnStmt) {.loc = loc, .ret = nullptr});

        return warning(loc, "function should not return void in a function return %T", sema.currentfunctionRet),
          note("%s", "A return statement without an expression shall only appear in a function whose return type is void"),
          insertStmt(SNEW(ReturnStmt) {.loc = loc, .ret = 
            wrap(sema.currentfunctionRet, llvm::UndefValue::get(irgen.wrap2(sema.currentfunctionRet)), loc)
          });
      }
      if (!(e = expression()))
        return;
      checkSemicolon();
      if (sema.currentfunctionRet->tags & TYVOID)
        return warning(loc,"function should return a value in a function return void"),
               note("A return statement with an expression shall not appear in a function whose return type is void"),
               insertStmt(SNEW(ReturnStmt) {.loc = loc, .ret = nullptr});
      // warn_if_bad_cast(e, sema.currentfunctionRet, " returning '" & $e->ty &
      // "' from a function with return type '" & $sema.currentfunctionRet &
      // "'")
      if (!(r = castto(e, sema.currentfunctionRet)))
        return;
      return insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = r});
    }
    case Kwhile:
    case Kswitch: {
      Expr test;
      Token tok = l.tok.tok;
      if (!(test = Bexpression()))
        return;
      if (tok == Kswitch) {
        integer_promotions(test);
        statement();
        llvm_unreachable("");
        return;
      }
      if (test->k == EConstant) {
        if (auto CI = dyn_cast<ConstantInt>(test->C)) {
          bool isFalse = CI->isZero();
          auto II = getInsertPoint();
          statement();
          if (isFalse)
            setInsertPoint(II);
          return;
        }
      }
      label_t BODY = jumper.createLabel();
      label_t CMP = jumper.createLabel();
      label_t LEAVE = jumper.createLabel();
      llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
      llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
      insertBr(CMP);
      insertLabel(BODY);
      statement();
      insertLabel(CMP);
      clearKnownConstantVariables();
      insertStmt(SNEW(CondJumpStmt) {.loc = Location::make_invalid(), .test = test, .T = BODY, .F = LEAVE});
      return insertLabel(LEAVE);
    }
    case Kfor: 
    {
      Expr cond = nullptr, forincl = nullptr;
      consume();
      enterBlock();
      if (l.tok.tok != TLbracket)
        return expectLB(getLoc());
      consume();
      if (istype(l.tok.tok)) {
        declaration();
      } else {
        if (l.tok.tok != TSemicolon) {
          Expr ex = expression();
          if (!ex)
            return;
          ex = simplify(ex);
          if (issimple(ex)) 
            warning(loc, "expression in for-clause-1(for loop initialization) has no effect");
          else 
            insertStmt(SNEW(ExprStmt) {.loc = ex->loc, .exprbody = ex});
          if (l.tok.tok != TSemicolon)
            warning(getLoc(), "missing ';' after for-clause-1 expression");
        }
        consume();
      }
      if (l.tok.tok != TSemicolon) {
        if (!(cond = expression()))
          return;
        valid_condition(cond);
        if (l.tok.tok != TSemicolon)
          expect(loc, "';'");
      }
      consume();
      if (l.tok.tok != TRbracket) {
        Location loc3 = getLoc();
        if (!(forincl = expression()))
          return;
        forincl = simplify(forincl);
        if (issimple(forincl)) {
          warning(loc3, "expression in expression-3(for loop increment) has no effect");
          forincl = nullptr;
        }
      }
      consume();
      label_t CMP = jumper.createLabel();
      label_t LEAVE = jumper.createLabel();
      llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
      llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
      insertLabel(CMP);
      if (cond) {
        if (cond->k == EConstant)
          if (auto CI = dyn_cast<ConstantInt>(cond->C))
            if (!CI->isZero()) goto FOR_NEXT; // if the condition is true, we don't have to jump!
        jumpIfFalse(cond, LEAVE);
      }
FOR_NEXT:
      if (forincl)
        insertStmt(SNEW(ExprStmt) {.loc = forincl->loc, .exprbody = forincl});
      statement();
      insertBr(CMP);
      insertLabel(LEAVE);
      leaveBlock();
      return;
    }
    case Kdo: 
    {
      Expr test;
      label_t CMP = jumper.createLabel();
      label_t LEAVE = jumper.createLabel();
      llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
      llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
      insertLabel(CMP);
      consume();
      statement();
      if (l.tok.tok != Kwhile)
        return expect(loc, "'while'");
      consume();
      if (l.tok.tok != TLbracket)
        return expectLB(loc);
      consume();
      if (!(test = Bexpression()))
        return;
      checkSemicolon();
      if (test->k == EConstant) {
        if (auto CI = dyn_cast<ConstantInt>(test->C)) {
          bool isFalse = CI->isZero();
          if (!isFalse)
            insertBr(CMP); // jump to CMP if condition is true
          return insertLabel(LEAVE);;
        }
      }
      jumpIfTrue(test, CMP);
      insertLabel(LEAVE);
    }
    case Kif: 
    {
      Expr test;
      if (!(test = Bexpression()))
        return;
      if (test->k == EConstant) {
        if (auto CI = dyn_cast<ConstantInt>(test->C)) {
          bool isFalse = CI->isZero();
          auto II = getInsertPoint();
          statement();
          if (isFalse)
            setInsertPoint(II); // skip if body if condition is false
          if (l.tok.tok == Kelse) {
            consume();
            II = getInsertPoint();
            statement();
            if (!isFalse)
              setInsertPoint(II); // skip else body if condition if true
          }
          return;
        }
      }
      label_t IF_END = jumper.createLabel();
      jumpIfFalse(test, IF_END);
      statement();
      if (l.tok.tok == Kelse) {
        label_t END = jumper.createLabel();
        consume();
        insertBr(END);
        insertLabel(IF_END);
        statement();
        insertLabel(END);
        return;
      }
      return insertLabel(IF_END);
    }
    case Kelse:
      return parse_error(loc, "'else' without a previous 'if'"), consume();
    case TIdentifier: 
    {
      TokenV tok = l.tok;
      consume();
      if (l.tok.tok == TColon) { // labeled-statement
        consume();
        label_t L = putLable(tok.s);
        insertLabel(L, tok.s);
        return statement();
      }
      l.tokenq.push_back(l.tok), l.tok = tok;
      // goto default!
    }
    default:
      break;
    }
    Expr e = expression();
    if (!e)
      return;
    e = simplify(e);
    if (issimple(e)) {
      warning(loc, "expression statement has no effect");
    }
    checkSemicolon();
    return insertStmt(SNEW(ExprStmt){.loc = loc, .exprbody = e});
  }
  Expr multiplicative_expression() {
    Expr r, result = cast_expression();
    if (!result)
      return nullptr;
    for (;;)
      switch (l.tok.tok) {
      case TMul:
        consume();
        if (!(r = cast_expression()))
          return nullptr;
        make_mul(result, r);
        continue;
      case TSlash:
        consume();
        if (!(r = cast_expression()))
          return nullptr;
        make_div(result, r);
        continue;
      case TPercent:
        consume();
        if (!(r = cast_expression()))
          return nullptr;
        make_rem(result, r);
        continue;
      default:
        return result;
      }
  }
  Expr additive_expression() {
    Expr r, result = multiplicative_expression();
    if (!result)
      return nullptr;
    for (;;)
      if (l.tok.tok == TAdd) {
        consume();
        if (!(r = multiplicative_expression()))
          return nullptr;
        make_add(result, r);
      } else if (l.tok.tok == TDash) {
        consume();
        if (!(r = multiplicative_expression()))
          return nullptr;
        make_sub(result, r);
      } else
        return result;
  }
  Expr shift_expression() {
    Expr r, result = additive_expression();
    if (!result)
      return nullptr;
    for (;;)
      if (l.tok.tok == Tshl) {
        consume();
        if (!(r = additive_expression()))
          return nullptr;
        make_shl(result, r);
      } else if (l.tok.tok == Tshr) {
        consume();
        if (!(r = additive_expression()))
          return nullptr;
        make_shr(result, r);
      } else
        return result;
  }
  Expr relational_expression() {
    Expr result = shift_expression();
    if (!result)
      return nullptr;
    for (;;) {
      switch (l.tok.tok) {
      case TLt:
      case TLe:
      case TGt:
      case TGe:
        make_cmp(result, l.tok.tok);
      default:
        return result;
      }
    }
  }
  Expr equality_expression() {
    Expr result = relational_expression();
    if (!result)
      return nullptr;
    for (;;) {
      switch (l.tok.tok) {
        case TEq:
        case TNe:
          make_cmp(result, l.tok.tok, true);
        default: 
          return result;
      }
    }
  }
  Expr AND_expression() {
    Expr r, result = equality_expression();
    if (!result)
      return nullptr;
    for (;;)
      if (l.tok.tok == TBitAnd) {
        consume();
        if (!(r = equality_expression()))
          return nullptr;
        make_bitop(result, r, And);
      } else
        return result;
  }
  Expr exclusive_OR_expression() {
    Expr r, result = AND_expression();
    if (!result)
      return nullptr;
    for (;;) {
      if (l.tok.tok == TXor) {
        consume();
        if (!(r = AND_expression()))
          return nullptr;
        make_bitop(result, r, Xor);
      } else
        return result;
    }
  }
  Expr inclusive_OR_expression() {
    Expr r, result = exclusive_OR_expression();
    if (!result)
      return nullptr;
    for (;;) {
      if (l.tok.tok == TBitOr) {
        consume();
        if (!(r = exclusive_OR_expression()))
          return nullptr;
        make_bitop(result, r, Or);
      } else
        return result;
    }
  }
  Expr logical_AND_expression() {
    Expr r, result = inclusive_OR_expression();
    if (!result)
      return nullptr;
    for (;;) {
      if (l.tok.tok == TLogicalAnd) {
        consume();
        if (!(r = inclusive_OR_expression()))
          return nullptr;
        valid_condition(result);
        valid_condition(r);
        result = boolToInt(binop(result, LogicalAnd, r, context.getInt()));
      } else
        return result;
    }
  }
  Expr logical_OR_expression() {
    Expr r, result = logical_AND_expression();
    if (!result)
      return nullptr;
    for (;;) {
      if (l.tok.tok == TLogicalOr) {
        consume();
        r = logical_AND_expression();
        if (!r)
          return nullptr;
        valid_condition(result);
        valid_condition(r);
        result = boolToInt(binop(result, LogicalOr, r, context.getInt()));
      } else
        return result;
    }
  }
  Expr expression() {
    // parse a expression
    Expr r, result = assignment_expression();
    if (!result)
      return nullptr;
    for (;;) {
      if (l.tok.tok == TComma) {
        consume();
        if (!(r = assignment_expression()))
          return nullptr;
        if (issimple(result)) {
          warning("left-hand side of comma expression has no effect, replace with right-hand side");
          result = r;
        }
        else {
          result = binop(result, Comma, r, result->ty);
        }
      } else
        return result;
    }
  }
  Expr conditional_expression(Expr start) {
    Expr rhs, lhs;
    if (!(lhs = logical_OR_expression()))
      return nullptr;
    if (l.tok.tok != TColon) {
      if (start)
        parse_error(getLoc(), "missing ':'");
      return lhs;
    }
    consume();
    if (!(rhs = conditional_expression()))
      return nullptr;
    if (!compatible(lhs->ty, rhs->ty))
      return type_error(getLoc(), "incompatible type for conditional-expression: the left is %T, the right is %T", lhs->ty, rhs->ty), nullptr;
    conv(lhs, rhs);
    valid_condition(start);
    return ENEW(ConditionExpr) {.loc = start->loc, .ty = lhs->ty, .cond = start, .cleft = lhs, .cright = rhs};
  }
  Expr conditional_expression() {
    Expr e = logical_OR_expression();
    if (!e)
      return nullptr;
    if (l.tok.tok == TQuestionMark)
      return conditional_expression(e);
    return e;
  }
  bool is_assigment_op(Token tok) {
    switch (tok) {
    case TAssign:
    case TAsignAdd:
    case TAsignSub:
    case TAsignMul:
    case TAsignDiv:
    case TAsignRem:
    case TAsignShl:
    case TAsignShr:
    case TAsignBitAnd:
    case TAsignBitOr:
    case TAsignBitXor:
      return true;
    default:
      return false;
    }
  }
  void make_assign(Token tok, Expr &lhs, Expr &rhs) {
    switch (tok) {
    case TAssign:
      return;
    case TAsignAdd:
      return make_add(lhs, rhs);
    case TAsignSub:
      return make_sub(lhs, rhs);
    case TAsignMul:
      return make_mul(lhs, rhs);
    case TAsignDiv:
      return make_div(lhs, rhs);
    case TAsignRem:
      return make_rem(lhs, rhs);
    case TAsignShl:
      return make_shl(lhs, rhs);
    case TAsignShr:
      return make_shr(lhs, rhs);
    case TAsignBitAnd:
      return make_bitop(lhs, rhs, And);
    case TAsignBitOr:
      return make_bitop(lhs, rhs, Or);
    case TAsignBitXor:
      return make_bitop(lhs, rhs, Xor);
    default:
      llvm_unreachable("bad assignment operator!");
    }
  }
  Expr assignment_expression() {
    Expr result = logical_OR_expression();
    if (!result)
      return nullptr;
    Token tok = l.tok.tok;
    if (tok == TQuestionMark)
      return consume(), conditional_expression(result);
    if (is_assigment_op(tok)) {
      Expr e, rhs;
      Variable_Info *info = nullptr;
      if (!assignable(result, info))
        return nullptr;
      consume();
      if (!(e = assignment_expression()))
        return nullptr;
      if (result->ty->tags & TYATOMIC) {
        BinOp op = getAtomicrmwOp(tok);
        if (op) {
          if (!(rhs = castto(e, result->ty)))
            return nullptr;
          return binop(result, op, rhs, result->ty);
        }
      }
      // make `a += b` to `a = a + b`
      make_assign(tok, result, e);
      if (!(rhs = castto(e, result->ty)))
        return nullptr;
      if (rhs->k == EConstant) 
        info->val = rhs->C;
      return binop(result, Assign, rhs, result->ty);
    } else
      return result;
  }
  Stmt translation_unit() {
    Stmt head = SNEW(HeadStmt) {.loc = getLoc()};
    setInsertPoint(head);
    sreachable = true;
    while (l.tok.tok != TEOF) {
      if (l.tok.tok == Kasm)
        parse_asm(), checkSemicolon();
      else 
        declaration();
    }
    insertStmtEnd();
    return head;
  }
  Stmt run(size_t &num_typedefs, size_t &num_tags) {
    Stmt ast;
    consume();
    enterBlock(); // the global scope!
    ast = translation_unit();
    leaveBlock2();
    num_typedefs = sema.typedefs.maxSyms;
    num_tags = sema.tags.maxSyms;
    return ast;
  }
  void compound_statement() {
    consume();
    enterBlock();
    NullStmt nullstmt {.k = SHead}; // make a dummy statement in stack, remove it when scope leaves
    Stmt head = SNEW(CompoundStmt) {.loc = getLoc(), .inner = reinterpret_cast<Stmt>(&nullstmt)};
    {
      llvm::SaveAndRestore<Stmt> saved_ip(InsertPt, head->inner);
      bool old = sreachable;
      eat_compound_statement();
      insertStmtEnd();
      sreachable = old;
    }
    head->inner = head->inner->next;
    leaveBlock();
    consume();
    insertStmt(head);
  }
  void eat_compound_statement() {
    while (l.tok.tok != TRcurlyBracket) {
      if (istype(l.tok.tok)) 
        declaration();
      else 
        statement();
    }
  }
  Stmt function_body(const xvector<Param> params, xvector<size_t> &args, Location loc) {
    consume();
    Stmt head = SNEW(HeadStmt) {.loc = loc};
    sema.typedefs.push_function();
    enterBlock();
    assert(jumper.cur == 0 && "labels scope should be empty in start of function");
    for (size_t i = 0;i < params.size();++i) {
      args[i] = sema.typedefs.putSym(params[i].name, Variable_Info {.ty = params[i].ty, .loc = loc, .tags = ASSIGNED});
    }
    {
      llvm::SaveAndRestore<Stmt> saved_ip(InsertPt, head);
      eat_compound_statement();
      if (!isTerminator(getInsertPoint()->k)) {
        Stmt s = getInsertPoint();
        Location loc2 = getLoc();
        if (sema.pfunc->second.getToken() == PP_main) {
          insertStmt(SNEW(ReturnStmt) {.loc = loc2, .ret = getIntZero()});
        } else {
          if (!(sema.currentfunctionRet->tags & TYVOID)) {
            // if this label never reaches (never break to it)
            if (s->k != SLabel || (!s->labelName && !jumper.used_breaks.contains(s->label))) {
                warning(loc2, "control reaches end of non-void function");
                note(loc, "in the definition of function %I", sema.pfunc);
            }
            // control reaches end of non-void function
            // non-void function does not return a value
            // no return statement in function returning non-void
          }
        }
      }
      insertStmtEnd();
    }
    for (const auto &it: jumper.labels) {
      IdentRef name = it.first;
      switch (it.second.flags) {
      case LBL_FORWARD:
        type_error(loc, "use of undeclared label: %I", name);
        break;
      case LBL_DECLARED:
        warning(loc, "unused label: %I", name);
        break;
      case LBL_OK:
        dbgprint("push label into function: %s\n", name->getKey().data());
        break;
      default: llvm_unreachable("");
      }
    }
    jumper.used_breaks.clear();
    leaveBlock();
    sema.typedefs.pop_function();
    consume();
    return head;
  }

}; // end Parser
