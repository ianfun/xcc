// TODO:     if (l.tok.tok == K_Static_assert) return consume_static_assert();
// TODO:     zero extend: getelementptr
// TODO:     JSON Dumper, Ast Console Dumper, SVG Dumper
struct Parser : public DiagnosticHelper {
  Lexer l;
  struct Sema { // Semantics processor
    CType currentfunctionRet = nullptr, currentInitTy = nullptr, currentCase = nullptr;
    IdentRef pfunc = nullptr;           // current function name: `__func__`
    SmallVector<BScope, 8> scopes{};    // block scope
    SmallVector<Token, 5> tokens_cache;
    size_t scopeTop = 0;
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
  Parser(SourceMgr &SM, xcc_context &context, IRGen &irgen)
      : DiagnosticHelper{context}, l(SM, *this, context), irgen{irgen} {}
  template <typename T> auto getsizeof(T a) { return irgen.getsizeof(a); }
  template <typename T> auto getAlignof(T a) { return irgen.getAlignof(a); }
  Expr binop(Expr a, BinOp op, Expr b, CType ty) {
    // construct a binary operator
    return ENEW(BinExpr){.loc = a->loc, .ty = ty, .lhs = a, .bop = op, .rhs = b};
  }
  Expr unary(Expr e, UnaryOp op, CType ty) {
    // construct a unary operator
    return ENEW(UnaryExpr){.loc = e->loc, .ty = ty, .uoperand = e, .uop = op};
  }
  ArenaAllocator &getAllocator() { return context.getAllocator(); }
  CType getTag(IdentRef s) {
    for (size_t i = sema.scopeTop; i != (size_t)-1; i--) {
      auto &Table = sema.scopes[i].tags;
      const auto it = Table.find(s);
      if (it != Table.end()) {
        return it->second.ty;
      }
    }
    return nullptr;
  }
  CType gettypedef(IdentRef s) {
    for (size_t i = sema.scopeTop; i != (size_t)-1; i--) {
      auto &Table = sema.scopes[i].typedefs;
      const auto it = Table.find(s);
      if (it != Table.end()) {
        it->second.tags |= INFO_USED;
        return it->second.ty;
      }
    }
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
  CType gettagByName(IdentRef name, enum CTypeKind expected) {
    auto r = getTag(name);
    if (r) {
      if (r->k != expected)
        return type_error(getLoc(), "%s %I is not a %s", expected, name, get_type_name_str(expected)), nullptr;
      return r;
    }
    return TNEW(IncompleteType){.align = 0, .tag = expected, .name = name};
  }
  void puttag(CType ty, Location &loc, enum CTypeKind k) {
    auto it = sema.scopes[sema.scopeTop].tags.insert(
        std::make_pair(ty->sname, Type_info{.ty = ty, .loc = loc}));
    if (it.second)
      type_error(getLoc(), "%s %I aleady defined", get_type_name_str(k), ty->sname, ty);
  }
  void putenum(IdentRef name, uintmax_t val) {
    noEnum(name);
    noTypeDef(name);
    sema.scopes[sema.scopeTop].enums[name] = val;
  }
  void noTypeDef(IdentRef name) {
    auto &Table = sema.scopes[sema.scopeTop].typedefs;
    if (Table.find(name) != Table.end())
      type_error(getLoc(), "%I redeclared", name);
  }
  void noEnum(IdentRef name) {
    auto &Table = sema.scopes[sema.scopeTop].enums;
    if (Table.find(name) != Table.end())
      type_error(getLoc(), "%I redeclared", name);
  }
  // function
  void putsymtype2(IdentRef name, CType yt) {
    Location loc = getLoc();
    auto &Table = sema.scopes[sema.scopeTop].typedefs;
    CType base = yt->ret;
    noEnum(name);
    if (base->k == TYARRAY)
      type_error(loc, "function cannot return array");
    if (base->tags & TYREGISTER)
      warning(loc, "'register' in function has no effect");
    if (base->tags & TYTHREAD_LOCAL)
      warning(loc, "'_Thread_local' in function has no effect");
    if (base->tags & (TYCONST | TYRESTRICT | TYVOLATILE))
      warning(loc, "type qualifiers ignored in function");
    const auto it = Table.find(name);
    if (it != Table.end()) {
      if (!compatible(yt, it->second.ty)) {
        type_error(loc, "conflicting types for function declaration %I", name);
      } else if (!(it->second.ty->ret->tags & TYSTATIC) && base->tags & TYSTATIC) {
        type_error(loc, "static declaration of %I follows non-static declaration", name);
        note(it->second.loc, "previous declaration of %I was here", name);
      } else {
        yt->tags |= it->second.ty->ret->tags & (TYSTATIC);
      }
    }
    Table[name] = Variable_Info{.ty = yt, .loc = getLoc()};
  }
  // typedef, variable
  void putsymtype(IdentRef name, CType yt) {
    noEnum(name);
    if (yt->k == TYFUNCTION)
      return putsymtype2(name, yt);
    auto &Table = sema.scopes[sema.scopeTop].typedefs;
    auto it = Table.find(name);
    if (it != Table.end()) {
      Location loc = getLoc();
      if (isTopLevel() || (yt->tags & (TYEXTERN | TYSTATIC))) {
        CType old = it->second.ty;
        bool err = true;
        constexpr auto q = TYATOMIC | TYCONST | TYRESTRICT | TYVOLATILE;
        if ((yt->tags & TYSTATIC) && !(old->tags & TYSTATIC))
          type_error(loc, "static declaration case %I follows non-static declaration", name);
        else if ((old->tags & TYSTATIC) && !(yt->tags & TYSTATIC))
          type_error(loc, "non-static declaration case %I follows static declaration", name);
        else if ((yt->tags & TYTHREAD_LOCAL) && !(old->tags & TYTHREAD_LOCAL))
          type_error(loc, "thread-local declaration case %I follows non-thread-local declaration", name);
        else if ((old->tags & TYTHREAD_LOCAL) && !(yt->tags & TYTHREAD_LOCAL))
          type_error(loc, "non-thread-local declaration case %I follows thread-local declaration", name);
        else if ((yt->tags & q) != (old->tags & q))
          type_error(loc, "conflicting type qualifiers for %I", name);
        else if (!compatible(old, yt))
          type_error(loc, "conflicting types for %I", name);
        else
          err = false;
        if (err)
          note("previous declaration case %I with type %T", name, old);
      } else {
        type_error(loc, "%I redeclared", name);
      }
    }
    Table[name] = Variable_Info{.ty = yt, .loc = getLoc()};
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
      return type_error(loc, "cannot cast 'void' expression to type %T", to),
             nullptr;
    if (to->k == TYINCOMPLETE || e->ty->k == TYINCOMPLETE)
      return type_error(loc, "cannot cast to incomplete type: %T", to), nullptr;
    if (to->k == TYSTRUCT || e->ty->k == TYSTRUCT || to->k == TYUNION ||
        e->ty->k == TYUNION)
      return type_error(loc, "cannot cast between different struct/unions"),
             nullptr;
    if (e->ty->k == TYENUM)
      return castto(make_cast(e, BitCast, context.getInt()), to);
    if (to->k == TYENUM)
      return castto(castto(e, context.getInt()), to);
    if (!(e->ty->isScalar() && to->isScalar()))
      return type_error(loc, "cast uoperand shall have scalar type"), nullptr;
    if (to->tags & TYBOOL)
      return binop(e, NE, ENEW(DefaultExpr){.loc = e->loc, .ty = e->ty},
                   context.typecache.b);
    if ((to->tags & (TYFLOAT | TYDOUBLE)) && e->ty->k == TYPOINTER)
      return type_error(
                 loc,
                 "A floating type shall not be converted to any pointer type"),
             nullptr;
    if ((e->ty->tags & (TYFLOAT | TYDOUBLE)) && to->k == TYPOINTER)
      return type_error(
                 loc,
                 "A floating type shall not be converted to any pointer type"),
             nullptr;
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
    if (e->ty->tags & (TYINT8 | TYINT16 | TYINT32 | TYINT64 | TYUINT8 |
                       TYUINT16 | TYUINT32 | TYUINT64 | TYBOOL)) {
      if (to->k == TYPOINTER)
        return make_cast(e, IntToPtr, to);
      if (isFloating(to))
        return make_cast(e, e->ty->isSigned() ? SIToFP : UIToFP, to);
    } else if (to->tags & (TYINT8 | TYINT16 | TYINT32 | TYINT64 | TYINT128 | TYUINT8 |
                           TYUINT16 | TYUINT32 | TYUINT64 | TYUINT128)) {
      if (e->ty->k == TYPOINTER)
        return make_cast(e, PtrToInt, to);
      if (isFloating(e->ty))
        return make_cast(e, to->isSigned() ? FPToSI : FPToUI, to);
    }
    return intcast(e, to);
  }
  bool isTopLevel() { return !sema.scopeTop; }
  void enterBlock() {
    sema.scopeTop++;
    if (sema.scopeTop >= sema.scopes.size())
      sema.scopes.push_back(BScope());
  }
  void leaveBlock() {
    auto &Table = sema.scopes[sema.scopeTop--];

    for (const auto &it : Table.typedefs) {
      if (!(it.second.tags & INFO_USED)) {
        if (it.second.ty->tags & TYPARAM)
          warning(it.second.loc, "unused parameter %I", it.first);
        else
          warning(it.second.loc, "unused variable %I", it.first);
      }
    }
    Table.typedefs.clear();
    Table.tags.clear();
    Table.enums.clear();
  }
  void leaveBlock2() {
    auto &Table = sema.scopes.front();
    for (const auto &it : Table.typedefs) {
      if (it.second.ty->tags & TYTYPEDEF)
        continue;
      if (!(it.second.tags & INFO_USED)) {
        if (it.second.ty->k == TYFUNCTION) {
          if (it.second.ty->ret->tags & TYSTATIC)
            warning(it.second.loc,
                    "static function %I' defined/declared but not used",
                    it.first);
        } else if (it.second.ty->tags & TYSTATIC) {
          warning(it.second.loc,
                  "static variable %I defined/declared but not used", it.first);
        }
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
  void checkScalar(Expr a, Expr b) {
    if (!(a->ty->isScalar() && b->ty->isScalar()))
      type_error(getLoc(), "scalar types expected");
  }
  static int checkArithmetic(CType ty) { return ty->k == TYPRIM; }
  void checkArithmetic(Expr a, Expr b) {
    if (!(checkArithmetic(a->ty) && checkArithmetic(b->ty)))
      type_error(getLoc(), "arithmetic types expected");
  }
  void checkSpec(Expr &a, Expr &b) {
    if (a->ty->k != b->ty->k)
      return type_error(
          getLoc(),
          "operands type mismatch: %E(has type %T) and %E(has type %T)", a, b);
    checkScalar(a, b);
    conv(a, b);
  }
  void make_add(Expr &result, Expr &r) {
    if (isFloating(result->ty)) {
      result = binop(result, FAdd, r, r->ty);
      return conv(result, r);
    }
    if (result->ty->k == TYPOINTER) {
      if (!checkInteger(r->ty))
        type_error(getLoc(), "integer expected");
      result = binop(result, SAddP, r, result->ty);
      return;
    }
    checkSpec(result, r);
    result = binop(result, r->ty->isSigned() ? SAdd : UAdd, r, r->ty);
  }
  void make_sub(Expr &result, Expr &r) {
    if (isFloating(result->ty)) {
      result = binop(result, FSub, r, r->ty);
      return conv(result, r);
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
      result =
          binop(result, SAddP, unary(r, r->ty->isSigned() ? SNeg : UNeg, r->ty),
                result->ty);
      return;
    }
    checkSpec(result, r);
    result = binop(result, r->ty->isSigned() ? SSub : USub, r, r->ty);
  }
  void make_shl(Expr &result, Expr &r) {
    checkInteger(result, r);
    integer_promotions(result);
    integer_promotions(r);
    result = binop(result, Shl, r, result->ty);
  }

  void make_shr(Expr &result, Expr &r) {
    checkInteger(result, r);
    integer_promotions(result);
    integer_promotions(r);
    result = binop(result, result->ty->isSigned() ? AShr : Shr, r, result->ty);
  }
  void make_bitop(Expr &result, Expr &r, BinOp op) {
    checkInteger(result, r);
    conv(result, r);
    result = binop(result, op, r, result->ty);
  }
  void make_mul(Expr &result, Expr &r) {
    checkArithmetic(result, r);
    conv(result, r);
    result = binop(result,
                   isFloating(r->ty) ? FMul : (r->ty->isSigned() ? SMul : UMul),
                   r, r->ty);
  }

  void make_div(Expr &result, Expr &r) {
    checkArithmetic(result, r);
    conv(result, r);
    result = binop(result,
                   isFloating(r->ty) ? FDiv : (r->ty->isSigned() ? SDiv : UDiv),
                   r, r->ty);
  }
  void make_rem(Expr &result, Expr &r) {
    checkInteger(result, r);
    conv(result, r);
    result = binop(result,
                   isFloating(r->ty) ? FRem : (r->ty->isSigned() ? SRem : URem),
                   r, r->ty);
  }
  Expr boolToInt(Expr e) {
    return ENEW(CastExpr){
        .loc = e->loc, .ty = context.getInt(), .castop = ZExt, .castval = e};
  }
  enum DeclaratorFlags {
    Direct,   // declarator with name
    Abstract, // declarator without name
    Function  // function parameter-type-list
  };
  CType declaration_specifiers() { return specifier_qualifier_list(); }
  NameTypePair abstract_decorator(CType base, enum DeclaratorFlags flags) {
    // abstract decorator has no name
    // for example: `static int        ()(int, char)`
    //               base-type      abstact-decorator
    return declarator(base, flags);
  }
  void consume() {
    // eat token from preprocesser
    l.cpp();
    if (l.tok.tok >= TIdentifier)
      l.tok.tok = TIdentifier;
    // TODO: builtin lookup, keyword lookup
  }
  inline Location getLoc() { return l.getLoc(); }
  inline Expr constant_expression() { return conditional_expression(); }
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
      dbgprint("typedef redefinition founded\n");
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
      if (result && (result->tags & TYTYPEDEF))
        return more(), handle_typedef(result);
    }
    return merge_types(sema.tokens_cache, loc);
  }
  NameTypePair declarator(CType base, enum DeclaratorFlags flags = Direct) {
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
                 ? ENEW(StructExpr){.ty = sema.currentInitTy,
                                    .arr2 = xvector<Expr>::get()}
                 : ENEW(ArrayExpr){.ty = sema.currentInitTy,
                                   .arr = xvector<Expr>::get()};
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

  NameTypePair direct_declarator(CType base,
                                 enum DeclaratorFlags flags = Direct) {
    switch (l.tok.tok) {
    case TIdentifier: {
      IdentRef name;
      if (flags == Abstract)
        return NameTypePair();
      name = l.tok.s, consume();
      return direct_declarator_end(base, name);
    }
    case TLbracket: {
      // int (*fn_ptr)(void);
      // int (*)(void);
      // int ((*arr)[5])[5];
      // int ((*)[5])[5];
      NameTypePair st, st2;
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
        return NameTypePair();
      if (l.tok.tok != TRbracket)
        warning(getLoc(), "missing ')'");
      else
        consume();
      st2 = direct_declarator_end(base, st.name);
      if (!st2.ty)
        return NameTypePair();
      memcpy(reinterpret_cast<void *>(dummy),
             reinterpret_cast<const void *>(st2.ty), ctype_size_map[st2.ty->k]);
      return st;
    }
    default:
      if (flags == Direct)
        return NameTypePair();
      return direct_declarator_end(base, nullptr);
    }
  }
  NameTypePair direct_declarator_end(CType base, IdentRef name) {
    switch (l.tok.tok) {
    case TLSquareBrackets: {
      CType ty = TNEW(ArrayType){
          .vla = nullptr, .arrtype = base, .hassize = false, .arrsize = 0};
      consume();
      if (l.tok.tok == TMul) {
        consume();
        if (l.tok.tok != TRSquareBrackets)
          return expect(getLoc(), "]"), NameTypePair();
        return NameTypePair(name, ty);
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
        if (!(e = assignment_expression()))
          return NameTypePair();
        if (!checkInteger(e->ty))
          return type_error(getLoc(), "size of array has non-integer type %T",
                            e->ty),
                 NameTypePair();
        ty->hassize = true;
        ty->arrsize = l.evaluator.withQuiet(e);
        if (l.evaluator.error) {
          l.evaluator.error = false;
          ty->vla = e;
          ty->arrsize = 0;
        }
        if (l.tok.tok != TRSquareBrackets)
          return expect(getLoc(), "]"), NameTypePair();
      }
      consume();
      return direct_declarator_end(ty, name);
    }
    case TLbracket: {
      CType ty = TNEW(FunctionType){.ret = base,
                                    .params = xvector<NameTypePair>::get(),
                                    .isVarArg = false};
      consume();
      if (l.tok.tok != TRbracket) {
        if (!parameter_type_list(ty->params, ty->isVarArg))
          return NameTypePair();
        if (l.tok.tok != TRbracket)
          return expectRB(getLoc()), NameTypePair();
      } else {
        ty->isVarArg = true;
      }
      consume();
      return direct_declarator_end(ty, name);
    }
    default:
      return NameTypePair(name, base);
    }
  }
  NameTypePair struct_declarator(CType base) {
    NameTypePair d;

    if (l.tok.tok == TColon) {
      Expr e;
      unsigned bitsize;
      consume();
      if (!(e = constant_expression()))
        return NameTypePair();
      bitsize = l.evaluator.withLoc(e, getLoc());
      l.evaluator.error = false;
      return NameTypePair(
          nullptr, TNEW(BitfieldType){.bittype = base, .bitsize = bitsize});
    }
    d = declarator(base, Direct);
    if (!d.ty)
      return NameTypePair();
    if (l.tok.tok == TColon) {
      unsigned bitsize;
      Expr e;
      consume();
      if (!(e = constant_expression()))
        return NameTypePair();
      bitsize = l.evaluator.withQuiet(e);
      if (l.evaluator.error)
        return NameTypePair();
      return NameTypePair(
          nullptr, TNEW(BitfieldType){.bittype = base, .bitsize = bitsize});
    }
    return NameTypePair(d.name, d.ty);
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
    IdentRef name = nullptr;
    if (l.tok.tok == TIdentifier) {
      name = l.tok.s;
      consume();
      if (l.tok.tok != TLcurlyBracket) // struct Foo
        return gettagByName(name, tok == Kstruct ? TYSTRUCT : TYUNION);
    } else if (l.tok.tok != TLcurlyBracket) {
      return parse_error(getLoc(),
                         "expect '{' for start anonymous struct/union"),
             nullptr;
    }
    CType result =
        tok == Kstruct
            ? TNEW(StructType){.sname = name,
                               .selems = xvector<NameTypePair>::get()}
            : TNEW(UnionType){.uname = name,
                              .uelems = xvector<NameTypePair>::get()};
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
            NameTypePair e = struct_declarator(base);
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
    if (result->sname)
      puttag(result, loc, tok == Kstruct ? TYSTRUCT : TYUNION);
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
    IdentRef name = nullptr;
    consume(); // eat enum
    if (l.tok.tok == TIdentifier) {
      name = l.tok.s;
      consume();
      if (l.tok.tok != TLcurlyBracket) // struct Foo
        return gettagByName(name, TYENUM);
    } else if (l.tok.tok != TLcurlyBracket)
      return parse_error(loc, "expect '{' for start anonymous enum"), nullptr;
    CType result =
        TNEW(EnumType){.ename = name, .eelems = xvector<EnumPair>::get()};
    consume();
    for (uintmax_t c = 0;; c++) {
      if (l.tok.tok != TIdentifier)
        break;
      IdentRef s = l.tok.s;
      consume();
      if (l.tok.tok == TAssign) {
        Expr e;
        consume();
        if (!(e = constant_expression()))
          return nullptr;
        c = l.evaluator.withLoc(e, getLoc());
        if (l.evaluator.error)
          return l.evaluator.error = false, nullptr;
      }
      putenum(s, c);
      result->eelems.push_back(EnumPair{.name = s, .val = c});
      if (l.tok.tok == TComma)
        consume();
      else
        break;
    }
    if (l.tok.tok != TRcurlyBracket)
      parse_error(getLoc(), "expect '}'");
    consume();
    if (result->ename)
      puttag(result, loc, TYENUM);
    return result;
  }
  bool parameter_type_list(xvector<NameTypePair> &params, bool &isVararg) {
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
        return type_error(getLoc(), "expect a type or old-style variable name"),
               false;
      // TODO: gettypedef old style
      CType base = declaration_specifiers();
      if (!base)
        return expect(getLoc(), "declaration-specifiers"), false;
      NameTypePair nt = abstract_decorator(base, Function);
      if (!nt.ty)
        return expect(getLoc(), "abstract-decorator"), false;
      params.push_back(nt);
      if (nt.ty->k == TYINCOMPLETE)
        type_error(getLoc(), "parameter %u has imcomplete type '", i, nt.ty),
            ok = false;
      nt.ty->tags |= TYPARAM;
      if (nt.name)
        sema.scopes[sema.scopeTop].typedefs[nt.name] =
            Variable_Info{.ty = nt.ty, .tags = INFO_USED, .loc = getLoc()};
      if (l.tok.tok == TComma)
        consume();
    }
    bool zero = false;
    for (size_t i = 0; i < params.size(); ++i) {
      if (i == 0 && (params.front().ty->tags & TYVOID))
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
      Expr e = expression();
      if (!e)
        return false;
      a = l.evaluator.withLoc(e, getLoc());
      if (l.evaluator.error) 
        return (l.evaluator.error = false);

      if (e->ty->isSigned() && a <= 0)
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
    ok = l.evaluator.withLoc(e, loc);
    if (l.evaluator.error)
      return (l.evaluator.error = false);
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
  bool assignable(Expr e) {
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
      return doinsertStmt(SNEW(DeclOnlyStmt) {.loc = loc, .decl = base});
    }
    result = SNEW(VarDeclStmt){.loc = loc, .vars = xvector<VarDecl>::get()};
    for (;;) {
      Location loc2 = getLoc();
      NameTypePair st = declarator(base, Direct);
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
        putsymtype2(st.name, st.ty);
        sema.pfunc = st.name;
        if (!isTopLevel())
          return parse_error(loc2, "function definition is not allowed here"), note("function can only declared in global scope");
        Stmt res = SNEW(FunctionStmt){.loc = loc, .funcname = st.name, .functy = st.ty};
        {
          llvm::SaveAndRestore<CType> saved_ret(sema.currentfunctionRet, st.ty->ret);
          res->funcbody = function_body(st.ty->params, loc);
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
      putsymtype(st.name, st.ty);

      result->vars.push_back(
          VarDecl{.name = st.name, .ty = st.ty, .init = nullptr});
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
        if (st.ty->k == TYARRAY && st.ty->vla) {
          return type_error(loc2, "variable length array may not be initialized");
        } else if (st.ty->k == TYFUNCTION)
          return type_error(loc2, "function may not be initialized");
        if (st.ty->tags & TYTYPEDEF)
          return type_error(loc2, "'typedef' may not be initialized");
        if (st.ty->tags & TYEXTERN) {
          type_error(loc2, "'extern' variables may not be initialized");
          if (!isTopLevel())
            note(loc2, "place initializer after 'extern' declaration to fix this");
          return;
        }
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
        if (isTopLevel() && !isConstant(init)) {
          type_error(loc2, "initializer element is not constant");
          note("global variable requires constant initializer");
        }
      } else {
        if (st.ty->k == TYARRAY) {
          if (st.ty->vla && isTopLevel())
            type_error(
                loc2,
                "variable length array declaration not allowed at file scope");
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
    return doinsertStmt(result);
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
    case TNot: {
      Expr e;
      consume();
      if (!(e = cast_expression()))
        return nullptr;
      if (!e->ty->isScalar())
        return type_error(getLoc(), "scalar expected"), nullptr;
      return boolToInt(unary(e, LogicalNot, context.getInt()));
    }
    case TMul: {
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
    case TBitNot: {
      Expr e;
      consume();
      if (!(e = cast_expression()))
        return nullptr;
      if (!checkInteger(e->ty))
        return type_error(getLoc(), "integer type expected"), nullptr;
      e = unary(e, Not, e->ty);
      integer_promotions(e);
      return e;
    }
    case TBitAnd: {
      Expr e;
      consume();
      if (!(e = expression()))
        return nullptr;
      if (!(e->ty->tags & TYLVALUE))
        return type_error(getLoc(), "cannot take the address of an rvalue"),
               nullptr;
      if (e->k == EString)
        return e->ty->tags &= ~TYLVALUE, e;
      if (e->k == EArrToAddress)
        return e->ty = context.getPointerType(e->voidexpr->ty), e;
      if (e->ty->k == TYBITFIELD)
        return type_error(getLoc(), "cannot take address of bit-field"),
               nullptr;
      if (e->ty->tags & TYREGISTER)
        warning(getLoc(), "take address of register variable");
      if (e->k == EUnary && e->uop == AddressOf && e->ty->p->k == TYFUNCTION)
        return e->ty->tags &= ~TYLVALUE, e;
      return unary(e, AddressOf, context.getPointerType(e->ty));
    }
    case TDash: {
      Expr e;
      consume();
      if (!(e = unary_expression()))
        return nullptr;
      if (!checkArithmetic(e->ty))
        return type_error(getLoc(), "arithmetic type expected"), nullptr;
      e = unary(e, e->ty->isSigned() ? SNeg : UNeg, e->ty);
      return integer_promotions(e), e;
    }
    case TAdd: {
      Expr e;
      consume();
      if (!(e = unary_expression()))
        return nullptr;
      if (!checkArithmetic(e->ty))
        return type_error(getLoc(), "arithmetic type expected"), nullptr;
      return integer_promotions(e), e;
    }
    case TAddAdd:
    case TSubSub: {
      Expr e;
      consume();
      if (!(e = unary_expression()))
        return nullptr;
      if (!assignable(e))
        return nullptr;
      Expr e2 = context.clone(e);
      CType ty = e->ty->k == TYPOINTER ? context.getSize_t() : e->ty;
      Expr one = ENEW(IntLitExpr) {
          .loc = e->loc,
          .ty = ty,
          .ival = APInt(ty->getBitWidth(), 1)};
      (tok == TAddAdd) ? make_add(e, one) : make_sub(e, one);
      return binop(e2, Assign, e, e->ty);
    }
    case Ksizeof: {
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
          return ENEW(IntLitExpr){
              .loc = loc, .ty = context.getSize_t(), .ival = APInt(context.getSize_t()->getBitWidth(), getsizeof(ty))};
        }
        if (!(e = unary_expression()))
          return nullptr;
        if (l.tok.tok != TRbracket)
          return expectRB(getLoc()), nullptr;
        consume();
        return ENEW(IntLitExpr){
            .loc = loc, .ty = context.getSize_t(), .ival = APInt(context.getSize_t()->getBitWidth(), getsizeof(e))};
      }
      if (!(e = unary_expression()))
        return nullptr;
      return ENEW(IntLitExpr){
          .loc = loc, .ty = context.getSize_t(), .ival = APInt(context.getSize_t()->getBitWidth(), getsizeof(e))};
    }
    case K_Alignof: {
      Expr result;
      consume();
      if (l.tok.tok != TLbracket)
        return expectLB(getLoc()), nullptr;
      consume();
      if (istype(l.tok.tok)) {
        CType ty = type_name();
        if (!ty)
          return expect(getLoc(), "type-name"), nullptr;
        result = ENEW(IntLitExpr){
            .loc = loc, .ty = context.getSize_t(), .ival = APInt(context.getSize_t()->getBitWidth(), getAlignof(ty))};
      } else {
        Expr e = constant_expression();
        if (!e)
          return nullptr;
        result = ENEW(IntLitExpr){
            .loc = loc, .ty = context.getSize_t(), .ival = APInt(context.getSize_t()->getBitWidth(), getAlignof(e))};
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
        return ENEW(IntLitExpr) {.ty = context.getInt(),  .ival = APInt(32, static_cast<uint64_t>(*s) - '0')};
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
        Expr result = ENEW(FloatLitExpr) {.ty = ty, .fval = APFloat(*Format)};
        auto it = result->fval.convertFromString(fstr, APFloat::rmNearestTiesToEven);
        if (auto err = it.takeError()) {
            SmallString<32> msg;
            llvm::raw_svector_ostream OS(msg);
            OS << err;
            lex_error(loc, "error parsing floating literal: %R", msg.str());
        } else {
            auto status = *it;
            SmallString<20> buffer;
            if ((status & APFloat::opOverflow) ||
                ((status & APFloat::opUnderflow) && result->fval.isZero())) {
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
        return result;
    }
    xint128_t bigVal = xint128_t::get();
    if (alwaysFitsInto64Bits(radix, NumDigits)) {
        uint64_t N = 0;
        for (auto Ptr = DigitsBegin; Ptr < SuffixBegin; ++Ptr)
            N = N * radix + llvm::hexDigitValue(*Ptr);
        bigVal.m64[1] = N;
    } else {
        dbgprint("large integer literal found...\n");
        bool overflow = false;
        for (auto Ptr = DigitsBegin; Ptr < SuffixBegin; ++Ptr) {
            overflow |= umul_overflow(bigVal, radix, bigVal);
            overflow |= uadd_overflow(bigVal, llvm::hexDigitValue(*Ptr));
        }
        if (overflow)
          warning("integer literal is too large to be represented in any integer type");
    }
    // bool AllowUnsigned = su.isUnsigned || radix != 10;
    CType ty = context.getInt();
    if (su.isLongLong) {
        ty = su.isUnsigned ? context.getULongLong() : context.getLongLong();
    } else if (su.isLong) {
        ty = su.isUnsigned ? context.getULong() : context.getLong();
    } else {
        /*unsigned acts = result->ival.getActiveBits();
        if (acts > 64) {
            ty = context.typecache.u128;
        } else if (acts > 32) {
            ty = context.typecache.u64;
        } else {
            ty = context.typecache.u32;
        }*/
      ty = context.typecache.i32;
    }
    Expr result = ENEW(IntLitExpr) {.loc = loc, .ty = ty, .ival =
       bigVal.m64[0] ? APInt(ty->getBitWidth(), ArrayRef<uint64_t>(bigVal.m64, 2)) : APInt(ty->getBitWidth(), bigVal.m64[1])
    };
    return result;
}

  Expr primary_expression() {
    // primary expressions:
    //      constant
    //      `(` expression `)`
    //      identfier
    Expr result;
    Location loc = getLoc();
    switch (l.tok.tok) {
    case TCharLit: {
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
      result = ENEW(IntLitExpr){.loc = loc, .ty = ty, .ival = APInt(ty->getBitWidth(), l.tok.i)};
      consume();
    } break;
    case TStringLit: {
      xstring s = l.tok.str;
      auto enc = l.tok.enc;
      consume();
      while (l.tok.tok == TStringLit) {
        s.push_back(l.tok.str);
        if (l.tok.enc != enc)
          type_error(getLoc(),
                     "unsupported non-standard concatenation of string "
                     "literals for UTF-%u and UTF-%u",
                     (unsigned)enc, (unsigned)l.tok.enc);
        consume();
      }
      switch (enc) {
      case 8:
        result = ENEW(ArrToAddressExpr){
            .loc = loc,
            .ty = context.typecache.strty,
            .arr3 = ENEW(StringExpr){
                .ty = TNEW(ArrayType){.arrtype = context.typecache.i8,
                                      .hassize = true,
                                      .arrsize = (unsigned)s.size()},
                .str = s,
                .is_constant = true}};
        break;
      /*
      case 16:
          result = writeUTF8toUTF16(s);
          break;
      case 32:
          result = writeUTF8toUTF32(s);
          break;
      */
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
        result = context.getIntZero();
      consume();
    } break;
    case TIdentifier: {
      if (l.want_expr)
        result = context.getIntZero();
      else if (l.tok.s->second.getToken() == PP__func__) {
        CType ty =
            TNEW(ArrayType){.arrtype = context.typecache.i8,
                            .hassize = true,
                            .arrsize = (unsigned)sema.pfunc->getKeyLength()};
        result = ENEW(ArrToAddressExpr){
            .loc = loc,
            .ty = context.typecache.strty,
            .arr3 = ENEW(StringExpr){.ty = ty,
                                     .str = xstring::get(sema.pfunc->getKey()),
                                     .is_constant = true}};
      } else {
        CType ty = nullptr;
        for (size_t i = sema.scopeTop; i != (size_t)-1; i--) {
          auto &Table = sema.scopes[i];
          auto it = Table.typedefs.find(l.tok.s);
          if (it != Table.typedefs.end()) {
            auto &info = it->second;
            if (info.ty->tags & TYTYPEDEF)
              return type_error(loc, "typedef-names is not allowed here"),
                     nullptr;
            info.tags |= INFO_USED;
            ty = context.clone(info.ty);
            ty->tags &= ~(TYCONST | TYRESTRICT | TYVOLATILE | TYATOMIC | TYREGISTER | TYTHREAD_LOCAL | TYEXTERN | TYSTATIC | TYNORETURN | TYINLINE | TYPARAM);
            ty->tags |= TYLVALUE;
            break;
          }
          auto ti = Table.enums.find(l.tok.s);
          if (ti != Table.enums.end())
            return consume(), ENEW(IntLitExpr){.loc = loc, .ty = context.getInt(), .ival = APInt(32, ti->second)};
        }
        if (!ty) {
          type_error(loc, "use of undeclared identifier %I", l.tok.s);
          // if (l.tok.s.size() > 2)
          //     fixName(l.tok.s);
          consume();
          return nullptr;
        }
        switch (ty->k) {
        case TYFUNCTION:
          result = unary(ENEW(VarExpr){.loc = loc, .ty = ty, .sval = l.tok.s}, AddressOf, TNEW(PointerType){.tags = TYLVALUE, .p = ty});
          break;
        case TYARRAY:
          result = ENEW(ArrToAddressExpr){.loc = loc, .ty = TNEW(PointerType){.tags = TYLVALUE, .p = ty->arrtype}, .arr3 = ENEW(VarExpr){.loc = loc, .ty = ty, .sval = l.tok.s}};
          break;
        default:
          result = ENEW(VarExpr){.loc = loc, .ty = ty, .sval = l.tok.s};
        }
        consume();
      }
    } break;
    case TLbracket: {
      consume();
      if (!(result = expression()))
        return nullptr;
      if (l.tok.tok != TRbracket)
        return expectRB(getLoc()), nullptr;
      consume();
    } break;
    case K_Generic: {
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
      case TAddAdd: {
        isadd = PostfixIncrement;
      ADD:
        if (!assignable(result))
          return nullptr;
        consume();
        result = ENEW(PostFixExpr){.loc = result->loc,.ty = result->ty,.pop = isadd,.poperand = result};
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
          NameTypePair pair = result->ty->selems[i];
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
  Expr Bexpression() {
    Expr e;
    consume();
    if (l.tok.tok != TLbracket)
      return expectLB(getLoc()), nullptr;
    consume();
    if (!(e = expression()))
      return nullptr;
    if (!e->ty->isScalar())
      type_error(getLoc(), "expect scalar types");
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
    doinsertStmt(SNEW(CondJumpStmt) {.loc = Location::make_invalid(), .test = test, .T = dst, .F = thenBB});
    return insertLabel(thenBB);
  }
  void jumpIfFalse(Expr test, label_t dst) {
    label_t thenBB = jumper.createLabel();
    doinsertStmt(SNEW(CondJumpStmt) {.loc = Location::make_invalid(), .test = test, .T = thenBB, .F = dst});
    return insertLabel(thenBB);
  }
  void insertBr(label_t L) {
    return doinsertStmt(SNEW(GotoStmt) {.loc = Location::make_invalid(), .location = L});
  }
  void insertLabel(label_t L, IdentRef Name = nullptr) {
    sreachable = true;
    return insertStmtInternal(SNEW(LabelStmt) {.loc = Location::make_invalid(), .label = L, .labelName = Name});
  }
  void insertStmtInternal(Stmt s) {
      InsertPt->next = s;
      InsertPt = s;
  }
  void doinsertStmt(Stmt s) {
    if (s) {
      if (sreachable) {
        insertStmtInternal(s);
        if (isTerminator(s->k)) {
            unreachable_reason = s;
            sreachable = false;
        }
      } else {
        if (unreachable_reason && s->loc.isValid()) {
          warning(s->loc, "this statement is unreachable");
          note(unreachable_reason->loc, "after this *terminator* statement is unreachable");
          unreachable_reason = nullptr;
        }
      }
    }
  }
  void insertStmt(Stmt s) {
    if (s) {
      doinsertStmt(s);
    } else {
      dbgprint("insertStmt: got nullptr?\n");
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
      return doinsertStmt(SNEW(GotoStmt) {.loc = loc, .location = L});
    }
    case Kcontinue:
      tryContinue();
      consume();
      checkSemicolon();
      return doinsertStmt(SNEW(GotoStmt){.loc = loc, .location = jumper.topContinue});
    case Kbreak: 
    {
      consume();
      checkSemicolon();
      if (jumper.topBreak == INVALID_LABEL)
        type_error(getLoc(), "%s", "connot break: no outer for/while/switch/do-while");
      else {
        if (sreachable) {
          jumper.used_breaks.insert(jumper.topBreak);
          dbgprint("insert into jumper.topBreak: %u\n", jumper.topBreak);
        }
        doinsertStmt(SNEW(GotoStmt){.loc = loc, .location = jumper.topBreak});
      }
      return;
    }
    case Kreturn: {
      Expr e, r;
      consume();
      if (l.tok.tok == TSemicolon) {
        consume();
        if (sema.currentfunctionRet->tags & TYVOID)
          return doinsertStmt(SNEW(ReturnStmt){.loc = loc, .ret = nullptr});
        return warning(loc, "function should not return void in a function return %T", sema.currentfunctionRet),
               note("%s", "A return statement without an expression shall only appear in a function whose return type is void"),
               doinsertStmt(SNEW(ReturnStmt){.loc = loc, .ret = ENEW(DefaultExpr){.loc = loc, .ty = sema.currentfunctionRet}});
      }
      if (!(e = expression()))
        return;
      checkSemicolon();
      if (sema.currentfunctionRet->tags & TYVOID)
        return warning(loc,"function should return a value in a function return void"),
               note("A return statement with an expression shall not appear in a function whose return type is void"),
               doinsertStmt(SNEW(ReturnStmt){.loc = loc, .ret = nullptr});
      // warn_if_bad_cast(e, sema.currentfunctionRet, " returning '" & $e->ty &
      // "' from a function with return type '" & $sema.currentfunctionRet &
      // "'")
      if (!(r = castto(e, sema.currentfunctionRet)))
        return;
      return doinsertStmt(SNEW(ReturnStmt){.loc = loc, .ret = r});
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
      label_t BODY = jumper.createLabel();
      label_t CMP = jumper.createLabel();
      label_t LEAVE = jumper.createLabel();
      llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
      llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
      insertBr(CMP);
      insertLabel(BODY);
      statement();
      insertLabel(CMP);
      doinsertStmt(SNEW(CondJumpStmt) {.loc = Location::make_invalid(), .test = test, .T = BODY, .F = LEAVE});
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
          doinsertStmt(SNEW(ExprStmt) {.loc = ex->loc, .exprbody = ex});
          if (l.tok.tok != TSemicolon)
            expect(getLoc(), "';'");
        }
        consume();
      }
      if (l.tok.tok != TSemicolon) {
        if (!(cond = expression()))
          return;
        if (!cond->ty->isScalar())
          type_error(loc, "expect scalar");
        if (l.tok.tok != TSemicolon)
          expect(loc, "';'");
      }
      consume();
      if (l.tok.tok != TRbracket) {
        if (!(forincl = expression()))
          return;
        if (l.tok.tok != TRbracket)
          expectRB(loc);
      }
      consume();
      label_t CMP = jumper.createLabel();
      label_t LEAVE = jumper.createLabel();
      llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
      llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
      insertLabel(CMP);
      if (cond)
        jumpIfFalse(cond, LEAVE);
      if (forincl)
        doinsertStmt(SNEW(ExprStmt) {.loc = forincl->loc, .exprbody = forincl});
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
      jumpIfTrue(test, CMP);
      insertLabel(LEAVE);
    }
    case Kif: 
    {
      label_t IF_END = jumper.createLabel();
      Expr test;
      if (!(test = Bexpression()))
        return;
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
    checkSemicolon();
    return doinsertStmt(SNEW(ExprStmt){.loc = loc, .exprbody = e});
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
    Expr r, result = shift_expression();
    if (!result)
      return nullptr;
    for (;;) {
      switch (l.tok.tok) {
      case TLt:
        consume();
        if (!(r = shift_expression()))
          return nullptr;
        checkSpec(result, r);
        result = boolToInt(binop(
            result, isFloating(r->ty) ? FLT : (r->ty->isSigned() ? SLT : ULT),
            r, context.typecache.b));
        continue;
      case TLe:
        consume();
        if (!(r = shift_expression()))
          return nullptr;
        checkSpec(result, r);
        result = boolToInt(binop(
            result, isFloating(r->ty) ? FLE : (r->ty->isSigned() ? SLE : ULE),
            r, context.typecache.b));
        continue;
      case TGt:
        consume();
        if (!(r = shift_expression()))
          return nullptr;
        checkSpec(result, r);
        result = boolToInt(binop(
            result, isFloating(r->ty) ? FGT : (r->ty->isSigned() ? SGT : UGT),
            r, context.typecache.b));
        continue;
      case TGe:
        consume();
        if (!(r = shift_expression()))
          return nullptr;
        checkSpec(result, r);
        result = boolToInt(binop(
            result, isFloating(r->ty) ? FGE : (r->ty->isSigned() ? SGE : UGE),
            r, context.typecache.b));
        continue;
      default:
        return result;
      }
    }
  }
  Expr equality_expression() {
    Expr r, result = relational_expression();
    if (!result)
      return nullptr;
    for (;;) {
      if (l.tok.tok == TEq) {
        consume();
        if (!(r = relational_expression()))
          return nullptr;
        if ((result->ty->k == TYPOINTER) && (r->ty->k == TYPOINTER))
          result = boolToInt(binop(result, EQ, r, context.typecache.b));
        else {
          checkSpec(result, r);
          result = boolToInt(binop(result, isFloating(r->ty) ? FEQ : EQ, r,
                                   context.typecache.b));
        }
      } else if (l.tok.tok == TNe) {
        consume();
        if (!(r = relational_expression()))
          return nullptr;
        if (result->ty->k == TYPOINTER && r->ty->k == TYPOINTER)
          result = boolToInt(binop(result, NE, r, context.typecache.b));
        else {
          checkSpec(result, r);
          result = boolToInt(binop(result, isFloating(r->ty) ? FNE : NE, r,
                                   context.typecache.b));
        }
      } else
        return result;
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
        checkScalar(result, r);
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
        checkScalar(result, r);
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
        result = binop(result, Comma, r, result->ty);
      } else
        return result;
    }
  }
  Expr conditional_expression(Expr start) {
    Expr rhs, lhs;
    if (!(lhs = logical_OR_expression()))
      return nullptr;
    if (l.tok.tok != TColon)
      return lhs;
    consume();
    if (!(rhs = conditional_expression()))
      return nullptr;
    if (!compatible(lhs->ty, rhs->ty))
      return type_error(getLoc(),
                        "incompatible type for conditional-expression: the "
                        "left is , the right is",
                        lhs->ty, rhs->ty),
             nullptr;
    conv(lhs, rhs);
    return ENEW(ConditionExpr){.loc = start->loc,
                               .ty = lhs->ty,
                               .cond = start,
                               .cleft = lhs,
                               .cright = rhs};
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
      if (!assignable(result))
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
  Stmt run() {
    Stmt ast;
    consume();
    enterBlock(); // the global scope!
    sema.scopeTop = 0;
    ast = translation_unit();
    leaveBlock2();
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
  Stmt function_body(xvector<NameTypePair> &params, Location loc) {
    consume();
    Stmt head = SNEW(HeadStmt) {.loc = loc};
    enterBlock();
    assert(jumper.cur == 0 && "labels scope should be empty in start of function");
    for (const auto &it: params)
      sema.scopes[sema.scopeTop].typedefs[it.name] = Variable_Info {.ty = it.ty, .loc = loc};
    {
      llvm::SaveAndRestore<Stmt> saved_ip(InsertPt, head);
      eat_compound_statement();
      if (!isTerminator(getInsertPoint()->k)) {
        Stmt s = getInsertPoint();
        Location loc2 = getLoc();
        if (sema.pfunc->second.getToken() == PP_main) {
          insertStmt(SNEW(ReturnStmt) {.loc = loc2, .ret = context.getIntZero()});
        } else {
          if (sema.currentfunctionRet->tags & TYVOID) {
            insertStmt(SNEW(ReturnStmt) {.loc = loc2, .ret = nullptr});
          } else {
            // if this label never reaches (never break to it)
            if (s->k != SLabel || (!s->labelName &&
                !jumper.used_breaks.contains(s->label))
              ) {
               warning(loc2, "control reaches end of non-void function");
                note(loc, "in the definition of function %I", sema.pfunc);
            }
            insertStmt(SNEW(ReturnStmt) {.loc = loc, .ret = ENEW(UndefExpr) {.loc = loc2, .ty = sema.currentfunctionRet}});
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
    consume();
    return head;
  }

}; // end Parser
