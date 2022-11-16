// A recursive descent parser(https://en.wikipedia.org/wiki/Recursive_descent_parser)
// for C language(https://open-std.org/JTC1/SC22/WG14/www/docs/n3054.pdf, https://en.cppreference.com/w/c/23).
//
// Also,
// static type checking,
// syntax analysis(https://en.wikipedia.org/wiki/Syntax_(programming_languages), https://en.wikipedia.org/wiki/Parsing),
// and Semantics(https://en.wikipedia.org/wiki/Semantics_(computer_science)).

struct Type_info {
    CType ty = nullptr;
    Location loc = Location();
};
struct Label_Info {
    label_t idx = 0;
    uint8_t flags = LBL_UNDEFINED;
    Location loc = Location();
};
constexpr uint8_t 
    LOCAL_GARBAGE = 0,         // just declared
    USED = 0x1,                // set if variable is used
    ASSIGNED = 0x2,            // set if assigned or initialized
    PARAM = 0x4;               // set if is a parameter
struct Variable_Info {
    CType ty = nullptr;
    Location loc = Location();
    llvm::Constant *val = nullptr;
    uint8_t tags = 0;
};
struct Parser : public DiagnosticHelper {
    enum Implict_Conversion_Kind {
        Implict_Cast,
        Implict_Assign,
        Implict_Init,
        Implict_Return,
        Implict_Call
    };
    Lexer l;
    xcc_context &context;
    struct Sema { // Semantics processor
        CType currentfunctionRet = nullptr, currentInitTy = nullptr;
        Stmt currentswitch = nullptr;
        IdentRef pfunc; // current function name: `__func__`
        FunctionAndBlockScope<Variable_Info, 50> typedefs;
        BlockScope<Type_info, 20> tags;
        SmallVector<Token, 5> tokens_cache;
        uint8_t currentAlign = 0; // current align(bytes)
        bool type_error = false;   // type error
    } sema;
    struct JumpBuilder {
        DenseMap<IdentRef, Label_Info> labels{}; // named labels
        unsigned cur = 0;
        llvm::SmallSet<label_t, 12> used_breaks{};
        label_t topBreak = INVALID_LABEL, topContinue = INVALID_LABEL;
        unsigned createLabel() { return cur++; }
        Label_Info &lookupLabel(IdentRef Name) {
            return labels.insert(std::make_pair(Name, Label_Info())).first->second;
        }
    } jumper;
    IRGen &irgen;
    Stmt InsertPt = nullptr;
    bool sreachable;
    Stmt unreachable_reason = nullptr;
    Expr intzero, intone, size_t_one, cfalse, ctrue;
    StringPool string_pool;
    type_tag_t TYLONGDOUBLE, TYLONG, TYULONG;
    Parser(SourceMgr &SM, IRGen &irgen, DiagnosticConsumer &Diag, xcc_context &theContext)
        : DiagnosticHelper{Diag},l(SM, *this, theContext, Diag), context{theContext}, irgen{irgen},
          intzero{wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt::getZero(context.getInt()->getBitWidth())))},
          intone{wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt(context.getInt()->getBitWidth(), 1)))},
          cfalse{wrap(context.getBool(), ConstantInt::getFalse(irgen.ctx))},
          ctrue{wrap(context.getBool(), ConstantInt::getTrue(irgen.ctx))},
          string_pool{irgen}, TYLONGDOUBLE{context.getLongDoubleTag()}, TYLONG{context.getLongTag()}, TYULONG{context.getULongTag()} { }
    template <typename T> auto getsizeof(T a) { return irgen.getsizeof(a); }
    template <typename T> auto getAlignof(T a) { return irgen.getAlignof(a); }
    Expr binop(Expr a, BinOp op, Expr b, CType ty) {
        return ENEW(BinExpr){.loc = a->loc, .ty = ty, .lhs = a, .bop = op, .rhs = b};
    }
    Expr unary(Expr e, UnaryOp op, CType ty) {
        return ENEW(UnaryExpr){.loc = e->loc, .ty = ty, .uoperand = e, .uop = op};
    }
    Expr complex_from_real(Expr real, CType ty) {
        ty = tryGetComplexTypeFromNonComplex(ty->tags);
        auto zero = llvm::Constant::getNullValue(irgen.wrapNoComplexSCalar(ty));
        if (real->k == EConstant)
            return wrap(ty, llvm::ConstantStruct::get(irgen.wrapComplex(ty->tags), {real->C, zero}), real->loc);
        return binop(real, Complex_CMPLX, wrap(ty, zero), ty);
    }
    Expr complex_from_imag(Expr imag, CType ty) {
        ty = tryGetComplexTypeFromNonComplex(ty->tags);
        auto zero = ConstantFP::getZero(irgen.wrapNoComplexSCalar(ty));
        if (imag->k == EConstant)
            return wrap(ty, llvm::ConstantStruct::get(irgen.wrapComplex(ty->tags), {zero, imag->C}), imag->loc);
        return binop(wrap(ty, zero), Complex_CMPLX, imag, ty);
    }
    Expr complex_pair(Expr a, Expr b, CType ty) {
        return (a->k == EConstant && b->k == EConstant) ?
            wrap(ty, llvm::ConstantStruct::get(irgen.wrapComplex(ty->tags), {a->C, b->C})) :
            binop(a, Complex_CMPLX, b, ty);
    }
    Expr complex_get_real(Expr e) {
        if (e->k == EConstant) {
            auto CS = cast<llvm::ConstantStruct>(e->C);
            return wrap(e->ty, CS->getOperand(0), e->loc);
        }
        if (e->k == EBin && e->bop == Complex_CMPLX) {
            return e->lhs;
        }
        llvm_unreachable("");
    }
    Expr complex_get_imag(Expr e) {
        if (e->k == EConstant) {
            auto CS = cast<llvm::ConstantStruct>(e->C);
            return wrap(e->ty, CS->getOperand(1), e->loc);
        }
        if (e->k == EBin && e->bop == Complex_CMPLX) {
            return e->rhs;
        }
        llvm_unreachable("");
    }
    CType tryGetComplexType(type_tag_t tags) {
        if (tags & TYDOUBLE)
            return context.getComplexDouble();
        else if (tags & TYFLOAT)
            return context.getComplexFloat();
        else if (tags & TYF128)
            return context.getComplexFloat128();
        return context.make(tags);
    }
    CType tryGetComplexTypeFromNonComplex(type_tag_t tags) {
        if (tags & TYDOUBLE)
            return context.getComplexDouble();
        else if (tags & TYFLOAT)
            return context.getComplexFloat();
        else if (tags & TYF128)
            return context.getComplexFloat128();
        return context.make(tags | TYCOMPLEX);
    }
    Expr complex_pair(Expr a, Expr b) {
        assert(a && b && "complex_pair: nullptr is invalid");
        return complex_pair(a, b, tryGetComplexTypeFromNonComplex(a->ty->tags));
    }
    Expr complex_zero(CType ty) {
        assert(ty->tags & TYCOMPLEX);
        auto T = irgen.wrapComplex(ty->tags);
        auto zero = ConstantFP::getZero(T->getTypeAtIndex((unsigned)0));
        return wrap(ty, llvm::ConstantStruct::get(T, {zero, zero}));
    }
    ArenaAllocator &getAllocator() { return context.getAllocator(); }
    CType gettypedef(IdentRef Name) {
        assert(Name && "gettypedef: Name is nullptr");
        auto it = sema.typedefs.getSym(Name);
        if (it && it->ty->tags & TYTYPEDEF)
            return it->ty;
        return nullptr;
    }
    label_t getLabel(IdentRef Name) {
        assert(Name && "expect a Name to put");
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
        default: llvm_unreachable("");
        }
        return ref.idx;
    }
    label_t putLable(IdentRef Name, Location loc) {
        assert(Name && "expect a Name to put");
        Label_Info &ref = jumper.lookupLabel(Name);
        switch (ref.flags) {
        case LBL_UNDEFINED: // not used, declared
            ref.flags = LBL_DECLARED;
            ref.idx = jumper.createLabel();
            ref.loc = loc;
            break;
        case LBL_FORWARD: // used, and defined => ok!
            ref.flags = LBL_OK;
            break;
        case LBL_DECLARED: // declared => declared twice!
            type_error(loc, "duplicate label: %I", Name);
            note(ref.loc, "previous declaration of label %I is here", Name);
            break;
        default: llvm_unreachable("");
        }
        return ref.idx;
    }
    CType gettagByName(IdentRef Name, uint8_t expected, Location full_loc) {
        assert(Name && "gettagByName: Name is nullptr");
        CType result;
        size_t idx;
        auto r = sema.tags.getSym(Name, idx);
        if (r) {
            if (r->ty->k != expected)
                type_error(full_loc, "%s %I is not a %s", expected, Name, get_type_name_str(expected));
            result = r->ty;
        } else {
            result = TNEW(IncompleteType){.align = 0, .tag = expected, .name = Name};
            Type_info info = Type_info{.ty = result, .loc = full_loc};
            insertStmt(SNEW(DeclStmt){.loc = full_loc, .decl_idx = sema.tags.putSym(Name, info), .decl_ty = result});
        }
        return result;
    }
    size_t puttag(IdentRef Name, CType ty, Location loc, uint8_t k) {
        assert(ty && "no type provided");
        bool found = false;
        if (Name) {
            size_t prev;
            auto old = sema.tags.getSymInCurrentScope(Name, prev);
            if (old) {
                if (old->ty->k != TYINCOMPLETE) {
                    type_error("%s %I redefined", get_type_name_str(k), Name);
                    note(old->loc, "previous declaration is here");
                } else if (old->ty->tag != k) {
                    type_error("%s %I redeclared with different as different kind: %s", get_type_name_str(k), Name,
                               get_type_name_str(old->ty->tag));
                    note(old->loc, "previous declaration is here");
                }
                old->ty = ty;
                old->loc = loc;
                insertStmt(SNEW(UpdateForwardDeclStmt){
                    .loc = loc,
                    .prev_idx = prev,
                    .now = ty,
                });
                found = true;
            }
        }
        size_t Idx = sema.tags.putSym(Name ? Name : reinterpret_cast<IdentRef>(ty), Type_info{.ty = ty, .loc = loc});
        if (!found) {
            insertStmt(SNEW(DeclStmt){.loc = loc, .decl_idx = Idx, .decl_ty = ty});
        }
        return Idx;
    }
    void putenum(IdentRef Name, uint64_t val, Location full_loc) {
        assert(Name && "enum has no Name");
        auto old = sema.typedefs.getSymInCurrentScope(Name);
        if (old) {
            type_error(full_loc, "%I redefined", Name);
            note(old->loc, "previous declaration is here");
        }
        sema.typedefs.putSym(Name, Variable_Info{
                                       .ty = context.getConstInt(),
                                       .loc = full_loc,
                                       .val = ConstantInt::get(cast<llvm::IntegerType>(irgen.types[x32]), val),
                                       .tags = ASSIGNED | USED, // enums are used by default ignore warnings
                                   });
    }
    // function
    size_t putsymtype2(IdentRef Name, CType yt, Location full_loc) {
        CType base = yt->ret;
        if (base->k == TYARRAY)
            type_error(full_loc, "function cannot return array");
        if (base->tags & TYREGISTER)
            warning(full_loc, "'register' in function return type");
        if (base->tags & TYTHREAD_LOCAL)
            warning(full_loc, "'_Thread_local' in function return type");
        if (base->tags & (TYCONST | TYRESTRICT | TYVOLATILE | TYATOMIC))
            warning(full_loc, "type qualifiers ignored in function");
        size_t idx;
        auto it = sema.typedefs.getSymInCurrentScope(Name, idx);
        if (it) {
            if (!compatible(yt, it->ty)) {
                type_error(full_loc, "conflicting types for function declaration %I", Name);
            } else if (!(it->ty->ret->tags & TYSTATIC) && base->tags & TYSTATIC) {
                type_error(full_loc, "static declaration of %I follows non-static declaration", Name);
                note(it->loc, "previous declaration of %I was here", Name);
            } else {
                it->ty->tags |= it->ty->ret->tags & (TYSTATIC);
            }
        } else {
            idx = sema.typedefs.putSym(
                Name, Variable_Info{.ty = yt, .loc = full_loc, .tags = ASSIGNED}); // function never undefined
        }
        return idx;
    }
    // typedef, variable
    size_t putsymtype(IdentRef Name, CType yt, Location full_loc) {
        assert(Name && "missing a Name to put");
        assert(yt && "missing a type to put");
        if (yt->k == TYFUNCTION)
            return putsymtype2(Name, yt, full_loc);
        size_t idx;
        auto it = sema.typedefs.getSymInCurrentScope(Name, idx);
        if (it) {
            if (yt->tags & TYTYPEDEF) {
                if (!compatible(it->ty, yt)) {
                    type_error("%I redeclared as different kind of symbol", Name);
                    note(it->loc, "previous declaration of %I is here", Name);
                }
                goto PUT;
            }
            if (isTopLevel() || (yt->tags & (TYEXTERN | TYSTATIC))) {
                auto M = type_qualifiers | TYSTATIC | TYTHREAD_LOCAL | TYREGISTER | TYTYPEDEF | TYATOMIC;
                CType old = it->ty;
                bool err = true;
                type_tag_t t1 = yt->tags, t2 = old->tags;
                if ((t1 & M) != (t2 & M))
                    type_error(it->loc, "conflicting type qualifiers for %I", Name);
                else if (!compatible(old, yt))
                    type_error(full_loc, "conflicting types for %I: (%T v.s. %T)", Name, yt, old);
                else
                    err = false;
                if (err)
                    note(it->loc, "previous declaration is here", Name);
            } else {
                type_error(full_loc, "%I redeclared", Name);
            }
            PUT:
            it->ty = yt;
        } else {
            idx = sema.typedefs.putSym(Name, Variable_Info{.ty = yt, .loc = full_loc, .tags = LOCAL_GARBAGE});
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
        case K_Alignas: return true;
        default: break;
        }
        if (a.tok == TIdentifier) 
            return gettypedef(a.s) != nullptr;
        return false;
    }
    Expr make_cast(Expr from, CastOp op, CType to) {
        return ENEW(CastExpr){.loc = from->loc, .ty = to, .castop = op, .castval = from};
    }
    enum Cast_Status {
        Cast_Ok,
        Cast_Imcompatible,
        Cast_DiscardsQualifiers,
        Cast_Sign
    };
    static enum Cast_Status canBeSavelyCastTo(CType p, CType expected) {
        assert(p && "p is nullptr");
        assert(expected && "expected is nullptr");
        assert(p->k == expected->k);
        switch (p->k) {
        case TYPRIM:
            if ((p->tags & (ty_prim | TYCOMPLEX)) == (expected->tags & (ty_prim | TYCOMPLEX)))
                return Cast_Ok;
            if (getNoSignTypeIndex(p->tags) == getNoSignTypeIndex(expected->tags))
                return Cast_Sign;
            return Cast_Imcompatible;
        case TYFUNCTION:
            if (canBeSavelyCastTo(p->ret, expected->ret) != Cast_Ok || p->params.size() != expected->params.size())
                return Cast_Imcompatible;
            for (unsigned i = 0; i < expected->params.size(); ++i)
                if (canBeSavelyCastTo(p->params[i].ty, expected->params[i].ty) != Cast_Ok)
                    return Cast_Imcompatible;
            return Cast_Ok;
        case TYSTRUCT:
        case TYENUM:
        case TYUNION: return p == expected ? Cast_Ok : Cast_Imcompatible;
        case TYPOINTER:
            if ((p->p->tags & TYCONST) > (expected->p->tags & TYCONST))
                return Cast_DiscardsQualifiers;
            if (p->p->tags & TYVOID || expected->p->tags & TYVOID)
                return Cast_Ok;
            return canBeSavelyCastTo(p->p, expected->p);
        case TYINCOMPLETE: return (p->tag == expected->tag && p->name == expected->name) ? Cast_Ok : Cast_Imcompatible;
        case TYBITFIELD: llvm_unreachable("");
        case TYARRAY:
            if (p->hassize != expected->hassize)
                return Cast_Imcompatible;
            if (p->hassize && (p->arrsize != expected->arrsize))
                return Cast_Imcompatible;
            return canBeSavelyCastTo(p->arrtype, expected->arrtype);
        }
        llvm_unreachable("");
    }
    Expr float_cast(Expr e, CType to) {
        assert(e && "cast from nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        if (((e->ty->tags & TYFLOAT) && (to->tags & TYDOUBLE)) ||         // float to double
            ((e->ty->tags & (TYFLOAT | TYDOUBLE)) && (to->tags & TYF128)) // float/double to fp128
        ) {
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getFPExtend(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
            return make_cast(e, FPExt, to);
        }
        if (((e->ty->tags & TYF128) && (to->tags & (TYFLOAT | TYDOUBLE))) || // fp128 to float/double
            ((e->ty->tags & TYDOUBLE) && (to->tags & TYFLOAT))               // double to float
        ) {
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getFPTrunc(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
            return make_cast(e, FPTrunc, to);
        }
        return e;
    }
    Expr int_cast(Expr e, CType to) {
        assert(e && "cast from nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        if ((to->tags & intergers_or_bool)) {
            if (to->getBitWidth() == e->ty->getBitWidth())
                return bit_cast(e, to);
            if (intRank(to->tags) > intRank(e->ty->tags)) {
                if (e->k == EConstant) {
                    auto f = (e->ty->tags & signed_integers) ? &llvm::ConstantExpr::getSExt : &llvm::ConstantExpr::getZExt;
                    return wrap(to, f(e->C, irgen.wrapNoComplexSCalar(to), false), e->loc);
                }
                return make_cast(e, e->ty->tags & signed_integers ? SExt : ZExt, to);
            }
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getTrunc(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
            return make_cast(e, Trunc, to);
        }
        return type_error("invalid conversion from %T to %T", e->ty, to), e;
    }
    Expr integer_to_ptr(Expr e, CType to) {
        assert(e->ty && "cannot cast from expression with no type");
        assert((e->ty->tags & intergers_or_bool) && "bad call to integer_to_ptr()");
        assert((to->k == TYPOINTER) && "bad call to integer_to_ptr()");
        if (e->k == EConstant) {
            auto CI = cast<ConstantInt>(e->C);
            if (CI->isZero()) // A interger constant expression with the value 0 is a *null pointer constant*
                return wrap(to, llvm::ConstantPointerNull::get(cast<llvm::PointerType>(irgen.types[xptr])),
                            e->loc);
            return wrap(to, llvm::ConstantExpr::getIntToPtr(CI, irgen.wrapNoComplexSCalar(to)), e->loc);
        }
        return make_cast(e, IntToPtr, to);
    }
    Expr ptr_to_integer(Expr e, CType to) {
        assert(e->ty && "cannot cast from expression with no type");
        assert((e->ty->k == TYPOINTER) && "bad call to ptr_to_integer()");
        assert((to->tags & intergers_or_bool) && "bad call to ptr_to_integer()");
        if (e->k == EConstant)
            return wrap(to, llvm::ConstantExpr::getPtrToInt(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
        return make_cast(e, PtrToInt, to);
    }
    Expr integer_to_float(Expr e, CType to) {
        assert(e->ty && "cannot cast from expression with no type");
        assert(to->isFloating() && "bad call to integer_to_float()");
        assert((e->ty->tags & intergers_or_bool) && "bad call to integer_to_float()");
        if (e->ty->isSigned()) {
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getSIToFP(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
            return make_cast(e, SIToFP, to);
        }
        if (e->k == EConstant)
            return wrap(to, llvm::ConstantExpr::getUIToFP(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
        return make_cast(e, UIToFP, to);
    }
    Expr float_to_integer(Expr e, CType to) {
        assert(e->ty && "cannot cast from expression with no type");
        assert(e->ty->isFloating() && "bad call to float_to_integer()");
        assert((to->tags & intergers_or_bool) && "bad call to float_to_integer()");
        if (to->isSigned()) {
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getFPToSI(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
            return make_cast(e, FPToSI, to);
        }
        if (e->k == EConstant) 
            return wrap(to, llvm::ConstantExpr::getFPToUI(e->C, irgen.wrapNoComplexSCalar(to)), e->loc);
        return make_cast(e, FPToUI, to);
    }
    Expr ptr_cast(Expr e, CType to, enum Implict_Conversion_Kind implict = Implict_Cast) {
        assert(e->ty && "cannot cast from expression with no type");
        assert((e->ty->k == TYPOINTER && to->k == TYPOINTER) && "bad call to ptr_cast()");
        if (implict != Implict_Cast) {
            auto status = canBeSavelyCastTo(e->ty, to);
            if (status == Cast_Ok)
                goto PTR_CAST;
            {
                CType arg1 = e->ty;
                CType arg2 = to;
                const char *msg;
                switch (implict) {
                case Implict_Cast: llvm_unreachable("");
                case Implict_Assign:
                    std::swap(arg1, arg2);
                    if (status == Cast_DiscardsQualifiers)
                        msg = "assigning to %T from %T discards qualifiers";
                    else if (status == Cast_Sign)
                        msg = "assigning to %T from %T converts between pointers to integer types with different "
                              "sign";
                    else
                        msg = "incompatible pointer types assigning to %T from %T";
                    break;
                case Implict_Init:
                    std::swap(arg1, arg2);
                    if (status == Cast_DiscardsQualifiers)
                        msg = "initializing %T with an expression of type %T' discards qualifiers";
                    else if (status == Cast_Sign)
                        msg = "initializing %T with an expression of type %T converts between pointers to integer "
                              "types with different sign";
                    else
                        msg = "incompatible pointer types initializing %T with an expression of type '%T'";
                    break;
                case Implict_Return:
                    if (status == Cast_DiscardsQualifiers)
                        msg = "returning %T from a function with result type %T discards qualifiers";
                    else if (status == Cast_Sign)
                        msg = "returning %T from a function with result type %T converts between pointers to "
                              "integer types with different sign";
                    else
                        msg = "incompatible pointer types returning %T from a function with result type %T";
                    break;
                case Implict_Call:
                    if (status == Cast_DiscardsQualifiers)
                        msg = "passing %T to parameter of type %T discards qualifiers";
                    else if (status == Cast_Sign)
                        msg = "passing %T to parameter of type %T converts between pointers to integer types with "
                              "different sign";
                    else
                        msg = "incompatible pointer types passing %T to parameter of type %T";
                    break;
                }
                warning(e->loc, msg, arg1, arg2);
            }
        }
PTR_CAST:
        return bit_cast(e, to);
    }
    Expr castto(Expr e, CType to, enum Implict_Conversion_Kind implict = Implict_Cast) {
        assert(e && "cast object is nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        Location loc = getLoc();
        if (type_equal(e->ty, to))
            return e;
        if (e->ty->tags & TYVOID)
            return type_error(loc, "cannot cast 'void' expression to type %T", to), e;
        if (to->k == TYINCOMPLETE || e->ty->k == TYINCOMPLETE)
            return type_error(loc, "cannot cast to incomplete type: %T", to), e;
        if (to->k == TYSTRUCT || e->ty->k == TYSTRUCT || to->k == TYUNION || e->ty->k == TYUNION)
            return type_error(loc, "cannot cast between different struct/unions"), e;
        if (e->ty->k == TYENUM) // cast from enum: bit-cast e to int first, then do-cast to ty
            return castto(bit_cast(e, context.getInt()), to);
        if (to->k == TYENUM) // cast to enum: do-cast e to int first, then bit-cast to enum
            return bit_cast(castto(e, context.getInt()), to);
        if (!(e->ty->isScalar() && to->isScalar()))
            return type_error(loc, "cast operand shall have scalar type"), e;
        if (to->tags & TYBOOL) {
            // simplify 'boolean(a) zext to int(b)' to 'a'
            if (e->k == EConstant)
                foldBool(e, false);
            else if (e->k == ECast && e->castop == ZExt && e->castval->ty->tags & TYBOOL)
                return e->castval;
            if (e->k == EConstant)
                if (auto CI = dyn_cast<ConstantInt>(e->C))
                    return getCBool(!CI->isZero());
            return unary(e, ToBool, to);
        }
        if (to->isFloating() && e->ty->k == TYPOINTER)
            return type_error(loc, "A floating type shall not be converted to any pointer type"), e;
        if (e->ty->isFloating() && to->k == TYPOINTER)
            return type_error(loc, "A floating type shall not be converted to any pointer type"), e;
        if (e->ty->k == TYPOINTER && to->k == TYPOINTER)
            return ptr_cast(e, to, implict);
        // if e is a complex number
        if (e->ty->tags & TYCOMPLEX) {
            Expr lhs = complex_get_real(e);
            Expr rhs = complex_get_imag(e);
            if (e->ty->isFloating()) {
                if (to->isFloating())
                    return (to->tags & TYCOMPLEX) ? 
                        complex_pair(float_cast(lhs, to), float_cast(rhs, to), to) :
                        float_cast(lhs, to);
                return (to->tags & TYCOMPLEX) ? 
                    complex_pair(float_to_integer(lhs, to), float_to_integer(rhs, to), to) :
                    float_to_integer(lhs, to);
            }
            if (to->isFloating())
                return (to->tags & TYCOMPLEX) ? complex_pair(integer_to_float(lhs, to), integer_to_float(rhs, to), to) :
                  integer_to_float(lhs, to);
            return (to->tags & TYCOMPLEX) ?
                complex_pair(int_cast(lhs, to), int_cast(rhs, to), to) :
                int_cast(lhs, to);
        }
        // than, e is not a complex number, and cast to complex
        if (to->tags & TYCOMPLEX) {
            if (to->isFloating())
                return complex_from_real((this->*(e->ty->isFloating() ? &Parser::float_cast : &Parser::integer_to_float))(e, to), to);
            if (e->ty->isFloating())
                return complex_from_real((this->*(e->ty->isFloating() ? &Parser::float_cast : &Parser::float_to_integer))(e, to), to);
            return complex_from_real(int_cast(e, to), to);
        }
        if (e->ty->isFloating() && to->isFloating())
            return float_cast(e, to);
        if (e->ty->tags & intergers_or_bool && to->isFloating())
            return integer_to_float(e, to);
        if (e->ty->isFloating() && to->tags & intergers_or_bool)
            return float_to_integer(e, to);
        if (e->ty->k == TYPOINTER && to->tags & intergers_or_bool)
            return ptr_to_integer(e, to);
        if (e->ty->tags & intergers_or_bool && to->k == TYPOINTER)
            return integer_to_ptr(e, to);
        return int_cast(e, to);
    }
    bool isTopLevel() { return sema.currentfunctionRet == nullptr; }
    void enterBlock() {
        sema.typedefs.push();
        sema.tags.push();
    }
    void leaveBlock() {
        const auto &T = sema.typedefs;
        if (!getNumErrors()) { // if we have parse errors, we may emit bad warnings about ununsed variables
            for (auto it = T.current_block(); it != T.end(); ++it) {
                if (!(it->info.tags & USED) && !(it->info.ty->tags & TYTYPEDEF)) {
                    if (it->info.tags & ASSIGNED) {
                        (it->info.tags & PARAM) ? 
                            warning(it->info.loc, "unused parameter %I", it->sym) :
                            warning(it->info.loc, "variable %I is unused after assignment", it->sym);
                    } else {
                        warning(it->info.loc, "unused variable %I", it->sym);
                    }
                }
            }
        }
        sema.typedefs.pop();
        sema.tags.pop();
    }
    void leaveBlock2() {
        if (!getNumErrors()) {
            for (const auto &it : sema.typedefs) {
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
        sema.typedefs.finalizeGlobalScope();
        sema.tags.finalizeGlobalScope();
    }
    void to2(Expr &e, type_tag_t tag) {
        if ((e->ty->tags & ty_prim) != tag)
            e = castto(e, context.make(tag));
    }
    void integer_promotions(Expr &e) {
        if (e->ty->k == TYBITFIELD || (e->ty->tags & (TYBOOL | TYINT8 | TYUINT8 | TYINT16 | TYUINT16) && !(e->ty->tags & TYCOMPLEX)))
            to2(e, TYINT);
    }
    void complex_conv(Expr &a, Expr &b, const type_tag_t at, const type_tag_t bt) {
        if ((at & (ty_prim | TYCOMPLEX)) == (bt & (ty_prim | TYCOMPLEX)))
            return;
        // C A Reference Manual Fifth Edition
        // Complex types and the usual binary conversions
        if (bt & TYCOMPLEX) {
            // both operands are complex
            // the shorter operand is converted to the type of the longer
            unsigned rA = scalarRankNoComplex(at);
            unsigned rB = scalarRankNoComplex(bt);
            if (rA > rB)
                b = castto(b, a->ty);
            else
                a = castto(a, b->ty);
            return;
        }
        // complex float has the highest rank
        if (at & floatings) {
            b = castto(b, a->ty);
            return;
        }
        // complex integer + floatings
        // complex int + double => complex double
        if (bt & floatings) {
            CType resultTy = tryGetComplexTypeFromNonComplex(bt);
            a = castto(a, resultTy);
            b = castto(b, resultTy);
            return;
        }
        // complex integer + integer
        CType resultTy = tryGetComplexTypeFromNonComplex(intRank(at) > intRank(bt) ? at : bt);
        a = castto(a, resultTy);
        b = castto(b, resultTy);
        return;
    }
    void conv(Expr &a, Expr &b) {
        const type_tag_t at = a->ty->tags;
        const type_tag_t bt = b->ty->tags;
        if (a->ty->k != TYPRIM || b->ty->k != TYPRIM)
            return;
        if (at & TYCOMPLEX)
            return complex_conv(a, b, at, bt);
        if (bt & TYCOMPLEX)
            return complex_conv(b, a, bt, at);
        if (at & TYLONGDOUBLE)
            return to2(b, TYLONGDOUBLE);
        if (bt & TYLONGDOUBLE)
            return to2(a, TYLONGDOUBLE);
        if (at & TYDOUBLE)
            return to2(b, TYDOUBLE);
        if (bt & TYDOUBLE)
            return to2(a, TYDOUBLE);
        if (at & TYFLOAT)
            return to2(b, TYFLOAT);
        if (bt & TYFLOAT)
            return to2(a, TYFLOAT);
        integer_promotions(a);
        integer_promotions(b);
        if (intRank(a->ty->tags) == intRank(b->ty->tags))
            return;
        auto sizeofa = getsizeof(a->ty);
        auto sizeofb = getsizeof(b->ty);
        bool isaunsigned = (at & unigned_integers);
        bool isbunsigned = (bt & unigned_integers);
        // if operand has the same sign
        if (isaunsigned == isbunsigned)
            return sizeofa > sizeofb ? b = castto(b, a->ty) : a = castto(a, b->ty), (void)0;
        if (isaunsigned && (sizeofa > sizeofb))
            return (void)(b = castto(b, a->ty));
        if (isbunsigned && (sizeofb > sizeofa))
            return (void)(a = castto(a, b->ty));
        if (!isaunsigned && sizeofa > sizeofb)
            return (void)(b = castto(b, a->ty));
        if (!isbunsigned && sizeofb > sizeofa)
            return (void)(a = castto(a, b->ty));
        isaunsigned ? 
            b = castto(b, a->ty) : 
            a = castto(a, b->ty);
    }
    void default_argument_promotions(Expr &e) {
        if (e->ty->k == TYENUM || e->ty->k == TYUNION || e->ty->k == TYSTRUCT || e->ty->k == TYPOINTER)
            return;
        if (e->ty->tags & TYFLOAT)
            e = castto(e, context.getDobule());
        integer_promotions(e);
    }
    bool checkInteger(CType ty) { return ty->tags & intergers_or_bool; }
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
            return type_error(
                getLoc(),
                "bad operands to binary expression:\n    operands type mismatch: %E(has type %T) and %E(has type %T)",
                a, b);
        if (!(a->ty->isScalar() && b->ty->isScalar()))
            return type_error(getLoc(), "scalar types expected");
        conv(a, b);
    }
    bool isAssignOp(enum BinOp op) {
        switch (op) {
        case Assign:
        case AtomicrmwAdd:
        case AtomicrmwSub:
        case AtomicrmwAnd:
        case AtomicrmwOr:
        case AtomicrmwXor: return true;
        default: return false;
        }
    }
    Expr comma(Expr a, Expr b) { return binop(simplify(a), Comma, simplify(b), b->ty); }
    Expr simplify(const Expr e) {
        switch (e->k) {
        case EBin:
            if (isAssignOp(e->bop) || e->bop == LogicalAnd || e->bop == LogicalOr)
                return e;
            if (e->rhs->isSimple()) {
                warning(e->rhs->loc, "unused binary expression result in right-hand side (%E)", e->rhs);
                note("simplify %E to %E", e, e->lhs);
                return simplify(e->lhs);
            }
            if (e->lhs->isSimple()) {
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
        case EVoid: return simplify(e->castval);
        case ECondition:
            // if the cond is simple, we should simplify it in constant folding
            if (e->cleft->isSimple() && e->cright->isSimple()) {
                warning("unused conditional-expression left-hand side and right-hand result");
                note("simplify %E to %E", e, e->cond);
                return simplify(e->cond);
            }
        case ECast: warning(e->loc, "unused cast expression, replace '(type)x' with 'x'"); return simplify(e->castval);
        case ECall: return e;
        case ESubscript:
            if (e->right->isSimple()) {
                warning("unused array substract, replace 'a[b]' with a");
                return simplify(e->left);
            }
            warning("unused subscript expression");
            return comma(e->left, e->right);
        case EPostFix:
        case EArray:
        case EStruct:
        case EConstantArray:
        case EMemberAccess:
        case EVar:
        case EConstant:
        case EArrToAddress:
        case EConstantArraySubstript: return e;
        }
        llvm_unreachable("bad expr kind");
    }
    void make_ptr_arith(Expr &e) {
        assert(e->ty->k == TYPOINTER && "expect a pointer");
        if ((e->ty->p->tags & TYVOID) || (e->k == EUnary && e->uop == AddressOf && e->ty->p->k == TYFUNCTION)) {
            e = context.clone(e);
            e->ty = context.getChar();
        }
    }
    Expr make_add_pointer(Expr rhs, Expr lhs) {
        Expr ptrPart = lhs;
        Expr intPart = rhs;
        if (rhs->ty->k == TYPOINTER) {
            ptrPart = rhs;
            intPart = lhs;
        }
        if (!checkInteger(intPart->ty)) {
            type_error(lhs->loc, "integer type expected");
            return ptrPart;
        }
        make_ptr_arith(ptrPart);
        if (intPart->k == EConstant) {
            if (auto CI = dyn_cast<ConstantInt>(intPart->C)) {
                APInt offset = intPart->ty->isSigned() ? CI->getValue().sextOrTrunc(irgen.pointerSizeInBits)
                                                       : CI->getValue().zextOrTrunc(irgen.pointerSizeInBits);
                if (ptrPart->k == EConstantArray)
                    return ENEW(ConstantArraySubstriptExpr){
                        .loc = ptrPart->loc, .ty = ptrPart->ty, .carray = ptrPart->array, .cidx = offset};
                if (ptrPart->k == EConstantArraySubstript) {
                    bool overflow = false;
                    auto result = ENEW(ConstantArraySubstriptExpr){.loc = ptrPart->loc,
                                                                   .ty = ptrPart->ty,
                                                                   .carray = ptrPart->array,
                                                                   .cidx = offset.uadd_ov(ptrPart->cidx, overflow)};
                    if (overflow)
                        warning("overflow when doing addition on pointers, the result is %A", &result->cidx);
                    return result;
                }
            }
        }
        return binop(ptrPart, SAddP, intPart, ptrPart->ty);
    }
    void make_add(Expr &result, Expr &r) {
        if (result->ty->k == TYPOINTER || r->ty->k == TYPOINTER) {
            if (result->ty->k == r->ty->k)
                return type_error("adding two pointers are not allowed");
            result = make_add_pointer(result, r);
            return;
        }
        checkSpec(result, r);
        if (result->ty->tags & TYCOMPLEX) {
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    auto X = cast<llvm::ConstantStruct>(result->C);
                    auto Y = cast<llvm::ConstantStruct>(r->C);
                    const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                    const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                    const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                    const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                    APFloat REAL = a;
                    handleOpStatus(REAL.add(c, APFloat::rmNearestTiesToEven));


                    APFloat IMAG = b;
                    handleOpStatus(IMAG.add(d, APFloat::rmNearestTiesToEven));

                    result = wrap(r->ty, llvm::ConstantStruct::get(irgen.wrapComplex(r->ty->tags),  ConstantFP::get(irgen.ctx, REAL), ConstantFP::get(irgen.ctx, IMAG)), result->loc);
                    return;
                }
                result = binop(result, CFAdd, r, r->ty);
                return;
            }
            if (result->k == EConstant && r->k == EConstant) {
                if (auto CS = dyn_cast<ConstantAggregateZero>(result->C))
                    return (void)(result = r);
                if (auto CS = dyn_cast<ConstantAggregateZero>(r->C))
                    return /* (void)(result = result) */;
                auto X = cast<llvm::ConstantStruct>(result->C);
                auto Y = cast<llvm::ConstantStruct>(r->C);
                const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();

                result = wrap(r->ty, llvm::ConstantStruct::get(
                    irgen.wrapComplex(r->ty->tags), 
                    ConstantInt::get(irgen.ctx, a + c), 
                    ConstantInt::get(irgen.ctx, b + d)),
                    result->loc
                );
                return;
            }
            result = binop(result, CAdd, r, r->ty);
            return;
        }
        if (result->ty->isFloating()) {
            if (result->k == EConstant && r->k == EConstant) {
                auto CF1 = cast<ConstantFP>(result->C);
                auto CF2 = cast<ConstantFP>(r->C);
                APFloat F = CF1->getValue();
                handleOpStatus(F.add(CF2->getValue(), APFloat::rmNearestTiesToEven));
                result = wrap(r->ty, ConstantFP::get(irgen.ctx, F), r->loc);
                return;
            }
            result = binop(result, FAdd, r, r->ty);
            return;
        }
        if (result->k == EConstant && r->k == EConstant) {
            if (auto CI = dyn_cast<ConstantInt>(result->C)) {
                if (CI->isZero())
                    return (void)(result = r);
                if (auto CI2 = dyn_cast<ConstantInt>(r->C)) {
                    if (CI2->isZero())
                        return;
                    bool overflow = false;
                    result = wrap(result->ty,
                                  ConstantInt::get(irgen.ctx, result->ty->isSigned()
                                                                  ? CI->getValue().sadd_ov(CI2->getValue(), overflow)
                                                                  : CI->getValue().uadd_ov(CI2->getValue(), overflow)));
                    if (overflow)
                        warning("%s addition overflow, the result is %A",
                                result->ty->isSigned() ? "signed" : "unsigned",
                                &cast<ConstantInt>(result->C)->getValue());
                    return;
                }
            }
            result = wrap(r->ty, llvm::ConstantExpr::getAdd(result->C, r->C, false, r->ty->isSigned()), r->loc);
            return;
        }
        result = binop(result, r->ty->isSigned() ? SAdd : UAdd, r, r->ty);
    }
    void make_sub(Expr &result, Expr &r) {
        bool p1 = result->ty->k == TYPOINTER;
        bool p2 = r->ty->k == TYPOINTER;
        if (p1 && p2) {
            if (!compatible(result->ty->p, r->ty->p))
                warning(getLoc(), "incompatible type when substract two pointers");
            CType ty = context.getIntPtr();
            result = binop(make_cast(result, PtrToInt, ty), PtrDiff, make_cast(r, PtrToInt, ty), ty);
            return;
        }
        if (p1 || p2) {
            Expr intPart = p1 ? r : result;
            Expr ptrPart = p1 ? result : r;
            if (intPart->k == EConstant) {
                intPart = wrap(intPart->ty, llvm::ConstantExpr::getNeg(intPart->C), intPart->loc);
            } else {
                intPart = unary(intPart, UNeg, intPart->ty);
            }
            result = make_add_pointer(ptrPart, intPart);
            return;
        }
        checkSpec(result, r);
        if (result->ty->tags & TYCOMPLEX) {
            if (result->k == EConstant && r->k == EConstant) {
                if (auto CS = dyn_cast<ConstantAggregateZero>(result->C))
                    return (void)(result = r);
                if (auto CS = dyn_cast<ConstantAggregateZero>(r->C))
                    return /* (void)(result = result) */;
            }
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    auto X = cast<llvm::ConstantStruct>(result->C);
                    auto Y = cast<llvm::ConstantStruct>(r->C);
                    const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                    const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                    const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                    const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                    APFloat REAL = a;
                    handleOpStatus(REAL.subtract(c, APFloat::rmNearestTiesToEven));


                    APFloat IMAG = b;
                    handleOpStatus(IMAG.subtract(d, APFloat::rmNearestTiesToEven));

                    result = wrap(r->ty, llvm::ConstantStruct::get(irgen.wrapComplex(r->ty->tags),  ConstantFP::get(irgen.ctx, REAL), ConstantFP::get(irgen.ctx, IMAG)), result->loc);
                    return;
                }
                result = binop(result, CFSub, r, r->ty);
                return;
            }
            if (result->k == EConstant && r->k == EConstant) {
                auto X = cast<llvm::ConstantStruct>(result->C);
                auto Y = cast<llvm::ConstantStruct>(r->C);
                const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();

                result = wrap(r->ty, llvm::ConstantStruct::get(
                    irgen.wrapComplex(r->ty->tags), 
                    ConstantInt::get(irgen.ctx, a - c), 
                    ConstantInt::get(irgen.ctx, b - d)),
                    result->loc
                );
                return;
            }
            result = binop(result, CSub, r, r->ty);
            return;
        }
        if (result->ty->isFloating()) {
            if (result->k == EConstant && r->k == EConstant) {
                auto CF1 = cast<ConstantFP>(result->C);
                auto CF2 = cast<ConstantFP>(r->C);
                APFloat F = CF1->getValue();
                handleOpStatus(F.subtract(CF2->getValue(), APFloat::rmNearestTiesToEven));
                result = wrap(r->ty, ConstantFP::get(irgen.ctx, F), r->loc);
                return;
            }
            conv(result, r);
            result = binop(result, FSub, r, r->ty);
            return;
        }
        if (result->k == EConstant && r->k == EConstant) {
            if (auto CI = dyn_cast<ConstantInt>(result->C)) {
                if (CI->isZero())
                    return (void)(result = r);
                if (auto CI2 = dyn_cast<ConstantInt>(r->C)) {
                    if (CI2->isZero())
                        return;
                    bool overflow = false;
                    result = wrap(result->ty,
                                  ConstantInt::get(irgen.ctx, result->ty->isSigned()
                                                                  ? CI->getValue().sadd_ov(CI2->getValue(), overflow)
                                                                  : CI->getValue().uadd_ov(CI2->getValue(), overflow)));
                    if (overflow)
                        warning("%s addition overflow, the result is %A",
                                result->ty->isSigned() ? "signed" : "unsigned",
                                &cast<ConstantInt>(result->C)->getValue());
                    return;
                }
            }
            result = wrap(r->ty, llvm::ConstantExpr::getSub(result->C, r->C, false, r->ty->isSigned()), r->loc);
            return;
        }
        result = binop(result, r->ty->isSigned() ? SSub : USub, r, r->ty);
    }
    void make_shl(Expr &result, Expr &r) {
        checkInteger(result, r);
        integer_promotions(result);
        integer_promotions(r);
        if (r->k != EConstant)
            goto NOT_CONSTANT;
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
            if (result->k != EConstant)
                goto NOT_CONSTANT;
            {
                const APInt &obj = cast<ConstantInt>(result->C)->getValue();
                bool overflow = false;
                if (result->ty->isSigned() && obj.isNegative())
                    warning(result->loc, "shifting a negative signed value is undefined");
                result = wrap(
                    result->ty,
                    ConstantInt::get(irgen.ctx, result->ty->isSigned() ? obj.sshl_ov(shift, overflow) : obj << shift),
                    result->loc);
                if (overflow)
                    warning(result->loc, "shift-left overflow, result is %A",
                            &cast<ConstantInt>(result->C)->getValue());
                return;
            }
BAD:
            result = wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getZero(width)), result->loc);
            return;
        }
        if (result->k != EConstant)
            goto NOT_CONSTANT;
        result = wrap(r->ty, llvm::ConstantExpr::getShl(result->C, r->C), r->loc);
        return;
NOT_CONSTANT:
        result = binop(result, Shl, r, result->ty);
    }
    void make_shr(Expr &result, Expr &r) {
        checkInteger(result, r);
        integer_promotions(result);
        integer_promotions(r);
        if (r->k != EConstant)
            goto NOT_CONSTANT;
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
            if (result->k != EConstant)
                goto NOT_CONSTANT;
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
        if (result->k != EConstant)
            goto NOT_CONSTANT;
        result = wrap(r->ty, llvm::ConstantExpr::getShl(result->C, r->C), r->loc);
        return;
NOT_CONSTANT:
        result = binop(result, result->ty->isSigned() ? AShr : Shr, r, result->ty);
    }
    Expr bit_cast(Expr e, CType to) {
        Expr r = context.clone(e);
        r->ty = to;
        return r;
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
        } else if (r->k == EConstant) {
            if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
                if (CI->isZero())
                    goto ZERO;
                if (CI->getValue().isAllOnes() && op != Xor)
                    goto ONE;
            }
        }
        result = binop(result, op, r, result->ty);
        return;
ZERO : {
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
ONE : {
    const StringRef allOnes = "all ones(all bits set, -1)";
    if (op == Or) {
        warning(result->loc, "'bitsize or' to %R is always all ones", allOnes);
        result = result =
            wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getAllOnes(result->ty->getBitWidth())), result->loc);
        return;
    }
    warning(result->loc, "'bitsize and' to %R is always itself", allOnes);
}
    }
    void make_mul(Expr &result, Expr &r) {
        checkArithmetic(result, r);
        conv(result, r);
        if (result->ty->tags & TYCOMPLEX) {
            if (result->k == EConstant && r->k == EConstant) {
                if (auto CS = dyn_cast<ConstantAggregateZero>(result->C))
                    return (void)(result = complex_zero(r->ty));
                if (auto CS = dyn_cast<ConstantAggregateZero>(r->C))
                    return (void)(result = complex_zero(r->ty));
            }
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    auto X = cast<llvm::ConstantStruct>(result->C);
                    auto Y = cast<llvm::ConstantStruct>(r->C);
                    const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                    const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                    const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                    const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                    result = wrap(r->ty, llvm::ConstantStruct::get(
                        irgen.wrapComplex(r->ty->tags),  
                        ConstantFP::get(irgen.ctx, a * c - b * d), 
                        ConstantFP::get(irgen.ctx, b * c + a * d)), 
                        result->loc
                    );
                    return;
                }
                result = binop(result, CFMul, r, r->ty);
                return;
            }
            if (result->k == EConstant && r->k == EConstant) {
                auto X = cast<llvm::ConstantStruct>(result->C);
                auto Y = cast<llvm::ConstantStruct>(r->C);
                const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();

                result = wrap(r->ty, llvm::ConstantStruct::get(
                    irgen.wrapComplex(r->ty->tags), 
                    ConstantInt::get(irgen.ctx, a * c - b * d), 
                    ConstantInt::get(irgen.ctx, b * c + a * d)),
                    result->loc
                );
                return;
            }
            result = binop(result, CMul, r, r->ty);
            return;
        }
        if (result->k != EConstant)
            goto NOT_CONSTANT;
        if (const auto lhs = dyn_cast<ConstantInt>(result->C)) {
            if (lhs->isZero())
                goto ZERO;
            if (r->k != EConstant)
                goto NOT_CONSTANT;
            if (const auto rhs = dyn_cast<ConstantInt>(r->C)) {
                bool overflow = false;
                if (rhs->isZero())
                    goto ZERO;
                result = wrap(r->ty,
                              ConstantInt::get(irgen.ctx, r->ty->isSigned()
                                                              ? lhs->getValue().smul_ov(rhs->getValue(), overflow)
                                                              : lhs->getValue().umul_ov(rhs->getValue(), overflow)),
                              result->loc);
                if (overflow)
                    warning(result->loc, "multiplication overflow");
                return;
            }
        } else if (const auto lhs = dyn_cast<ConstantFP>(result->C)) {
            if (r->k != EConstant)
                goto NOT_CONSTANT;
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
        result = binop(result, r->ty->isFloating() ? FMul : (r->ty->isSigned() ? SMul : UMul), r, r->ty);
        return;
ZERO:
        warning(result->loc, "multiplying by zero is always zero (zero-product property)");
    }
    void make_div(Expr &result, Expr &r) {
        checkArithmetic(result, r);
        conv(result, r);
        if (result->ty->tags & TYCOMPLEX) {
            conv(result, r);
            if (result->k == EConstant && r->k == EConstant) {
                if (auto CS = dyn_cast<ConstantAggregateZero>(result->C))
                    ; // TODO:
                if (auto CS = dyn_cast<ConstantAggregateZero>(r->C))
                    ; // TODO:
            }
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    auto X = cast<llvm::ConstantStruct>(result->C);
                    auto Y = cast<llvm::ConstantStruct>(r->C);
                    const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                    const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                    const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                    const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                    APFloat tmp = c * c + d * d;
                    result = wrap(r->ty, llvm::ConstantStruct::get(
                        irgen.wrapComplex(r->ty->tags),  
                        ConstantFP::get(irgen.ctx, (a * c + b * d) / tmp), 
                        ConstantFP::get(irgen.ctx, (b * c - a * d) / tmp)
                        ), 
                        result->loc
                    );
                    return;
                }
                result = binop(result, CFDiv, r, r->ty);
                return;
            }
            const bool isSigned = r->ty->isSigned();
            if (result->k == EConstant && r->k == EConstant) {
                auto X = cast<llvm::ConstantStruct>(result->C);
                auto Y = cast<llvm::ConstantStruct>(r->C);
                const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();
                APInt tmp0 = c * c + d * d;
                APInt tmp1 = a * c + b * d;
                APInt tmp2 = b * c - a * d;
                result = wrap(r->ty, llvm::ConstantStruct::get(
                    irgen.wrapComplex(r->ty->tags), 
                    ConstantInt::get(irgen.ctx, isSigned ? tmp1.sdiv(tmp0) : tmp1.udiv(tmp0)), 
                    ConstantInt::get(irgen.ctx, isSigned ? tmp2.sdiv(tmp0) : tmp2.udiv(tmp0))),
                    result->loc
                );
                return;
            }
            result = binop(result, isSigned ? CSDiv : CUDiv, r, r->ty);
            return;
        }
        if (r->k != EConstant)
            goto NOT_CONSTANT;
        if (const auto CI = dyn_cast<ConstantInt>(r->C)) {
            if (CI->getValue().isZero()) {
                warning(result->loc, "integer division by zero is undefined");
                result = wrap(r->ty, llvm::UndefValue::get(CI->getType()), result->loc);
                return;
            }
            if (result->k != EConstant)
                goto NOT_CONSTANT;
            result = wrap(r->ty,
                          llvm::ConstantExpr::get(r->ty->isSigned() ? llvm::Instruction::SDiv : llvm::Instruction::UDiv,
                                                  result->C, r->C),
                          result->loc);
            return;
        } else if (const auto CFP = dyn_cast<ConstantFP>(r->C)) {
            if (CFP->isZero()) {
                warning(result->loc, "floating division by zero is undefined");
                result = wrap(r->ty, llvm::UndefValue::get(CFP->getType()), result->loc);
                return;
            }
            if (result->k != EConstant)
                goto NOT_CONSTANT;
            result = wrap(r->ty, llvm::ConstantExpr::get(llvm::Instruction::FDiv, result->C, r->C), result->loc);
            return;
        }
NOT_CONSTANT:
        result = binop(result, r->ty->isFloating() ? FDiv : (r->ty->isSigned() ? SDiv : UDiv), r, r->ty);
    }
    void handleOpStatus(APFloat::opStatus status) { }
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
                result =
                    wrap(r->ty,
                         llvm::ConstantExpr::get(r->ty->isSigned() ? llvm::Instruction::SRem : llvm::Instruction::URem,
                                                 result->C, r->C),
                         result->loc);
                return;
            } else if (const auto CFP = dyn_cast<ConstantFP>(r->C)) {
                if (CFP->isZero()) {
                    warning("integer remainder by zero is undefined");
                    result = wrap(r->ty, llvm::UndefValue::get(CFP->getType()), result->loc);
                    return;
                }
                result = wrap(r->ty, llvm::ConstantExpr::get(llvm::Instruction::FRem, result->C, r->C), result->loc);
                return;
            }
        }
        result = binop(result, r->ty->isFloating() ? FRem : (r->ty->isSigned() ? SRem : URem), r, r->ty);
    }
    static BinOp get_relational_expression_op(Token tok, bool isFloating, bool isSigned) {
        switch (tok) {
        case TLe: return isFloating ? FLE : (isSigned ? SLE : ULE);
        case TLt: return isFloating ? FLT : (isSigned ? SLT : ULT);
        case TGt: return isFloating ? FGT : (isSigned ? SGT : UGT);
        case TGe: return isFloating ? FGE : (isSigned ? SGE : UGE);
        case TNe: return isFloating ? FNE : NE;
        case TEq: return isFloating ? FEQ : EQ;
        default: llvm_unreachable("");
        }
    }
    void make_cmp(Expr &result, Token tok, bool isEq = false) {
        Expr r;
        consume();
        if (!(r = isEq ? relational_expression() : shift_expression()))
            return;
        checkSpec(result, r);
        if ((result->ty->tags & TYCOMPLEX) && !isEq)
            return type_error("complex numbers unsupported in relational-expression");
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
                        status = lhsNeg != rhsNeg
                                     ? lhsNeg ? -1 : 1
                                     : APInt::tcCompare(A1.getRawData(), A2.getRawData(), A1.getNumWords());
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
                result = getBool(B);
                return;
            }
            if (const auto CFP = dyn_cast<ConstantFP>(result->C)) {
                const auto CFP2 = dyn_cast<ConstantFP>(r->C);
                bool B;
                APFloat::cmpResult status = CFP->getValue().compare(CFP2->getValue());
                switch (tok) {
                case TGt: B = status == cmpGreaterThan; break;
                case TGe: B = status == cmpEqual || status == cmpGreaterThan; break;
                case TEq: B = status == cmpEqual; break;
                case TNe: B = status != cmpEqual; break;
                case TLt: B = status == cmpLessThan || status == cmpEqual; break;
                case TLe: B = status == cmpLessThan; break;
                default: llvm_unreachable("bad operator to make_cmp");
                }
                result = getBool(B);
                return;
            }
            if (result->ty->tags & TYCOMPLEX) {
                auto X = cast<llvm::ConstantStruct>(result->C);
                auto Y = cast<llvm::ConstantStruct>(r->C);
                if (const auto &C1 = dyn_cast<ConstantInt>(X->getOperand(0))) {
                    const auto  &a = C1->getValue(),
                                &b = cast<ConstantInt>(X->getOperand(1))->getValue(),
                                &c = cast<ConstantInt>(Y->getOperand(0))->getValue(),
                                &d = cast<ConstantInt>(Y->getOperand(1))->getValue();
                    const bool l = (tok == TEq) ? a == c : a != c,
                               r = (tok == TEq) ? b == d : b != d;
                    result = getBool(tok == TEq ? (l & r) : (l | r));
                    return;
                }
                const auto 
                    &a = cast<ConstantFP>(X->getOperand(0))->getValue(),
                    &b = cast<ConstantFP>(X->getOperand(1))->getValue(),
                    &c = cast<ConstantFP>(Y->getOperand(0))->getValue(),
                    &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                const bool l = (tok == TEq) ? a.compare(c) == cmpEqual : a.compare(c) != cmpEqual,
                           r = (tok == TEq) ? b.compare(d) == cmpEqual : b.compare(d) != cmpEqual;
                result = getBool(tok == TEq ? (l & r) : (l | r));
                return;
            }
            
        }
        if (result->ty->tags & TYCOMPLEX)
            result = boolToInt(binop(result, tok == TEq ? CEQ : CNE, r, context.getBool()));
        else
            result =
                boolToInt(binop(result, get_relational_expression_op(tok, result->ty->isFloating(), result->ty->isSigned()),
                            r, context.getBool()));
    }
    Expr boolToInt(Expr e) {
        return ENEW(CastExpr){.loc = e->loc, .ty = context.getInt(), .castop = ZExt, .castval = e};
    }
    enum DeclaratorFlags {
        Direct = 0,  
        Abstract = 1,
        Function = 2 
    };
    CType declaration_specifiers() { return specifier_qualifier_list(); }
    void consume() {
        // eat token from preprocessor
        l.cpp();
        if (l.tok.tok >= TIdentifier)
            // make preprocessor identfiers to normal identfier
            l.tok.tok = TIdentifier;
    }
    SourceMgr &SM() { return l.SM; }
    Location getLoc() { return l.getLoc(); }
    Expr constant_expression() { return conditional_expression(); }
    void checkSemicolon() {
        if (l.tok.tok != TSemicolon)
            return warning(getLoc(), "missing ';'");
        consume();
    }
    void type_qualifier_list(CType &ty) {
        for (;;)
            switch (l.tok.tok) {
            case Kconst: ty->tags |= TYCONST, consume(); continue;
            case Krestrict: ty->tags |= TYRESTRICT, consume(); continue;
            case Kvolatile: ty->tags |= TYVOLATILE, consume(); continue;
            case K_Atomic: ty->tags |= TYATOMIC, consume(); continue;
            default: return;
            }
    }
// At most, one storage-class specifier may be given in the declaration specifiers in a declaration, except
// that:
// — thread_local may appear with static or extern,
// — auto may appear with all the others except typedef, and
// — constexpr may appear with auto, register, or static
    void verify_one_storage_class(type_tag_t tags) {
        // no storage-class specifiers founded, exit now
        if (!(tags & storage_class_specifiers))
            return;
        if (tags & TYTHREAD_LOCAL)
            tags &= ~(TYSTATIC | TYEXTERN);
        if (tags & TYAUTO) 
            tags &= ~TYTYPEDEF;
        if (tags & TYCONSTEXPR)
            tags &= ~(TYAUTO | TYSTATIC | TYREGISTER);
        if (llvm::countPopulation(tags & storage_class_specifiers) > 1)
            type_error("at most one storage-class specifier may be given in the declaration specifiers in a declaration");
    }
    CType specifier_qualifier_list() {
        Location loc = getLoc();
        CType eat_typedef = nullptr;
        type_tag_t tags = 0;
        unsigned b = 0, L = 0, s = 0, f = 0, d = 0, i = 0, c = 0, v = 0, numsigned = 0, numunsigned = 0, numcomplex = 0;
        type_tag_t old_tag;
        const Token firstTok = l.tok.tok;
        unsigned count = 0;
        for (;;++count) {
            old_tag = tags;
            switch (l.tok.tok) {
            case Kinline: tags |= TYINLINE; goto NO_REPEAT;
            case K_Noreturn: tags |= TYNORETURN; goto NO_REPEAT;
            case Kextern: tags |= TYEXTERN; goto NO_REPEAT;
            case Kstatic: tags |= TYSTATIC; goto NO_REPEAT;
            case K_Thread_local: tags |= TYTHREAD_LOCAL; goto NO_REPEAT;
            case Kregister: tags |= TYREGISTER; goto NO_REPEAT;
            case Krestrict: tags |= TYRESTRICT; goto NO_REPEAT;
            case Kvolatile: tags |= TYVOLATILE; goto NO_REPEAT;
            case Ktypedef: tags |= TYTYPEDEF; goto NO_REPEAT;
            case Kconst: tags |= TYCONST; goto NO_REPEAT;
            case Kconstexpr: tags |= TYCONSTEXPR; goto NO_REPEAT;
            case Kauto: tags |= TYAUTO; goto NO_REPEAT;
            case K_Atomic: tags |= TYATOMIC; goto NO_REPEAT;
NO_REPEAT:
            if (old_tag == tags) {
                warning(loc, "duplicate '%s' storage-class-specifier/type-qualifier", show(l.tok.tok));
                --count;
            }
            break;
            case Ksigned: ++numsigned; break;
            case Kunsigned: ++numunsigned; break;
            case Klong: ++L; break;
            case Kshort: ++s; break;
            case Kint: ++i; break;
            case Kdouble: ++d; break;
            case Kfloat: ++f; break;
            case Kvoid: ++v; break;
            case Kchar: ++c; break;
            case K_Complex: ++numcomplex; break;
            case K_Bool: ++b; break;

            case TIdentifier:
                if (eat_typedef) // we cannot eat double typedef
                    goto BREAK;
                eat_typedef = gettypedef(l.tok.s);
                if (!eat_typedef)
                    goto BREAK;
                eat_typedef = context.clone(eat_typedef);
                eat_typedef->tags &= ~TYTYPEDEF;
                break;
            case Kunion:
            case Kstruct:
                if (eat_typedef)
                    goto BREAK;
                eat_typedef = struct_union(l.tok.tok);
                if (!eat_typedef)
                    goto BREAK;
                continue;
            case Kenum:
                if (eat_typedef)
                    goto BREAK;
                eat_typedef = enum_decl();
                if (!eat_typedef)
                    goto BREAK;
                continue;
            default: goto BREAK;
            }
            consume();
        }
        BREAK:
        if (count == 0) {
            warning(loc, "type-specifier missing, defaults to 'int'");
            return context.getInt();
        }
        if (count == 1) {
            if (eat_typedef) return eat_typedef;
            if (tags) {
                warning(loc, "type-specifier missing(has type-qualifiers), defaults to 'int'");
                CType intTy = context.clone(context.getInt());
                intTy->tags |= tags;
                return intTy;
            }
            switch (firstTok) {
            case K_Bool: return context.getBool();
            case Kvoid: return context.getVoid();
            case Kdouble: return context.getDobule();
            case Kfloat: return context.getFloat();
            case Kchar: return context.getChar();
            case Kint:
            case Ksigned: return context.getInt();
            case Kunsigned: return context.getUInt();
            case Kshort: return context.getShort();
            case K_Complex:
                warning("%s", "plain '_Complex' requires a type specifier; assuming '_Complex double'");
                return context.getComplexDouble();
            default: llvm_unreachable("unhandled type in specifier_qualifier_list!");
            }
        }
        verify_one_storage_class(tags);
        bool isa_integer = i || numunsigned || numsigned || c || b;
        bool ias_float = d || f;
        if (isa_integer && ias_float)
            type_error("both integer and float type specifiers");
        else if (v && (isa_integer || ias_float))
            type_error("'void' never combine with other type specifiers");
        else if ((numunsigned && numsigned) || numsigned > 1 || numunsigned > 1) {
            type_error("duplicate 'signed/unsigned'");
            return context.getInt();
        }
        if (numcomplex) {
            if (numcomplex > 1)
                type_error("duplicate '_Complex'");
            if (v) {
                type_error("'_Complex void' is invalid");
                return context.getComplexDouble();
            }
            tags |= TYCOMPLEX;
        }
        if (f) {
            if (f > 1)
                type_error(loc, "duplicate 'float'");
            tags |= TYFLOAT;
        } else if (d) {
            if (d > 1)
                type_error(loc, "duplicate 'double'");
            if (L > 1)
                type_error(loc, "too many 'long's for 'double'");
            tags |= (L ? TYLONGDOUBLE : TYDOUBLE);
        } else if (s) {
            if (s > 1)
                type_error(loc, "duplicate 'short'");
            tags |= (numunsigned ? TYUSHORT : TYSHORT);
        } else if (L) {
            switch (L) {
            case 1: tags |= (numunsigned ? TYULONG : TYLONG); break;
            case 2: tags |= (numunsigned ? TYULONGLONG : TYLONGLONG); break;
            case 3: type_error("'long long long' is too long for XCC"); break;
            default: type_error(loc, "too many 'long'");
            }
        } else if (c) {
            if (c > 1)
                type_error(loc, "duplicate 'char'");
            tags |= numunsigned ? TYUCHAR : TYCHAR;
        } else if (i) {
            if (i > 1)
                type_error(loc, "duplicate 'int'");
            tags |= numsigned ? TYUINT : TYINT;
        } else if (v) {
            if (v > 1)
                type_error(loc, "duplicate 'void'");
            tags |= TYVOID;
        } else if (numunsigned) {
            tags |= TYUINT;
        } else if (numsigned) {
            tags |= TYINT;
        } else if (b) { 
            tags |= TYBOOL;
        } else {
            if (!eat_typedef) {
                warning(loc, "type-specifier missing, defaults to 'int'");
                eat_typedef = context.clone(context.getInt());
                goto MERGE;
            }
        }
        if (!eat_typedef)
            return context.make(tags);
MERGE:
        auto k = eat_typedef->k;
        if ((tags & (ty_prim | TYCOMPLEX)) && (eat_typedef->tags & (ty_prim | TYCOMPLEX) || k != TYPRIM)) {
            type_error(loc, "bad declaration specifier: cannot merge %T with %T", eat_typedef, context.make(tags));
        }
        eat_typedef->tags |= tags;
        return eat_typedef;
    }
    Declator declarator(CType base, enum DeclaratorFlags flags = Direct) {
        CType ty = base;
        while (l.tok.tok == TMul)
            consume(), ty = context.getPointerType(ty), type_qualifier_list(ty);
        return direct_declarator(ty, flags);
    }
    Expr initializer_list() {
        Expr e, result;
        int m;
        if (l.tok.tok != TLcurlyBracket) {
            auto ty = sema.currentInitTy;
            if (!ty)
                return assignment_expression();
            auto k = ty->k;
            if (k == TYARRAY && l.tok.tok == TStringLit) {
                xstring s = l.tok.str;
                auto enc = l.tok.enc;
                Location loc1 = getLoc();
                consume();
                while (l.tok.tok == TStringLit) {
                    s.push_back(l.tok.str);
                    if (l.tok.enc != enc)
                        type_error(loc1, "unsupported non-standard concatenation of string literals for UTF-%u and UTF-%u",(unsigned)enc, (unsigned)l.tok.enc);
                    consume();
                }
                s.make_eos();
                switch (enc) {
                    case 8:
                        if (!(ty->arrtype->tags & TYCHAR))
                            type_error(loc1, "initializing non-char array with string literal");
                        return wrap(context.getFixArrayType(context.getChar(), s.size() + 1), llvm::ConstantDataArray::getString(irgen.ctx, s.str(), false), loc1);
                    case 16:
                    {
                        if (context.getWcharTag() & (TYINT32 | TYUINT32)) {
                            SmallVector<uint32_t> data;
                            uint32_t state = 0, codepoint;
                            for (auto c : s) {
                                if (decode(&state, &codepoint, (uint32_t)(unsigned char)c))
                                    continue;
                                if (codepoint <= 0xFFFF) {
                                    data.push_back(codepoint);
                                    continue;
                                }
                                data.push_back(0xD7C0 + (codepoint >> 10));
                                data.push_back(0xDC00 + (codepoint & 0x3FF));
                            }
                            data.push_back(0);
                            if (!(ty->arrtype->tags & TYINT32))
                                type_error(loc1, "initializing %T array with wide string literal", ty->arrtype);
                            return wrap(context.getFixArrayType(context.getWchar(), s.size()), llvm::ConstantDataArray::get(irgen.ctx, data), loc1);
                        } else {
                            SmallVector<uint16_t> data;
                            uint32_t state = 0, codepoint;
                            for (auto c : s) {
                                if (decode(&state, &codepoint, (uint32_t)(unsigned char)c))
                                    continue;
                                if (codepoint <= 0xFFFF) {
                                    data.push_back(codepoint);
                                    continue;
                                }
                                data.push_back(0xD7C0 + (codepoint >> 10));
                                data.push_back(0xDC00 + (codepoint & 0x3FF));
                            }
                            data.push_back(0);
                            if (!(ty->arrtype->tags & TYINT16))
                                type_error(loc1, "initializing %T array with wide string literal", ty->arrtype);
                            return wrap(context.getFixArrayType(context.getWchar(), s.size()), llvm::ConstantDataArray::get(irgen.ctx, data), loc1);
                        }
                    }
                    case 32:
                    {
                        SmallVector<uint32_t> data;
                        uint32_t state = 0, codepoint;
                        for (const auto c : s)
                            if (!decode(&state, &codepoint, (uint32_t)(unsigned char)c))
                                data.push_back(codepoint);
                        data.push_back(0);
                        if (!(ty->arrtype->tags & TYUINT32))
                            type_error(loc1, "initializing %T array with wide string literal", ty->arrtype);
                        return wrap(context.getFixArrayType(context.getUChar(), s.size()), llvm::ConstantDataArray::get(irgen.ctx, data), loc1);
                    }
                    default: llvm_unreachable("bad string encoding");
                }
            }
            if (k != TYPRIM && k != TYPOINTER && k != TYENUM)
                return type_error(getLoc(), "expect bracket initializer"), nullptr;
            if (!(e = assignment_expression()))
                return nullptr;
            return castto(e, ty, Implict_Init);
        }
        result = sema.currentInitTy->k == TYSTRUCT
                     ? ENEW(StructExpr){.ty = sema.currentInitTy, .arr2 = xvector<Expr>::get()}
                     : ENEW(ArrayExpr){.ty = sema.currentInitTy, .arr = xvector<Expr>::get()};
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
            CType dummy =
                reinterpret_cast<CType>(getAllocator().Allocate(ctype_max_size,
                                                                alignof(std::max_align_t))); // make a dummy type, large
                                                                                             // enough to hold any
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
            memcpy(reinterpret_cast<void *>(dummy), reinterpret_cast<const void *>(st2.ty), ctype_size_map[st2.ty->k]);
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
            CType ty = TNEW(ArrayType){.vla = nullptr, .arrtype = base, .hassize = false, .arrsize = 0};
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
                    return type_error(getLoc(), "size of array has non-integer type %T", e->ty), Declator();
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
        case TLbracket: {
            CType ty = TNEW(FunctionType){.ret = base, .params = xvector<Param>::get(), .isVarArg = false};
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
        default: return Declator(name, base);
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
            return Declator(nullptr, TNEW(BitfieldType){.bittype = base, .bitsize = bitsize});
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
        Location full_loc = getLoc();
        consume(); // eat struct/union
        IdentRef Name = nullptr;
        if (l.tok.tok == TIdentifier) {
            Name = l.tok.s;
            consume();
            if (l.tok.tok != TLcurlyBracket) // struct Foo
                return gettagByName(Name, tok == Kstruct ? TYSTRUCT : TYUNION, full_loc);
        } else if (l.tok.tok != TLcurlyBracket) {
            return parse_error(getLoc(), "expect '{' for start anonymous struct/union"), nullptr;
        }
        CType result = TNEW(StructType){.sname = Name, .selems = xvector<Declator>::get()};
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
                    }
                    if (l.tok.tok == TRcurlyBracket) {
                        consume();
                        break;
                    }
                }
            }
        } else
            consume();
        result->sidx = puttag(Name, result, full_loc, tok == Kstruct ? TYSTRUCT : TYUNION);
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
        IdentRef Name = nullptr;
        Location full_loc = getLoc();
        consume(); // eat enum
        if (l.tok.tok == TIdentifier) {
            Name = l.tok.s;
            consume();
            if (l.tok.tok != TLcurlyBracket) // struct Foo
                return gettagByName(Name, TYENUM, full_loc);
        } else if (l.tok.tok != TLcurlyBracket)
            return parse_error(full_loc, "expect '{' for start anonymous enum"), nullptr;
        CType result = TNEW(EnumType){.ename = Name};
        if (!irgen.options.g)
            result->eelems = xvector<EnumPair>::get();
        consume();
        for (uint64_t c = 0;; c++) {
            if (l.tok.tok != TIdentifier)
                break;
            Location full_loc = getLoc();
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
                        warning(full_loc, "enum constant exceeds 32 bit");
                    }
                    c = I.getLimitedValue();
                }
            }
            putenum(s, c, full_loc);
            if (irgen.options.g)
                result->eelems.push_back(EnumPair{.name = s, .val = c});
            if (l.tok.tok == TComma)
                consume();
            else
                break;
        }
        if (l.tok.tok != TRcurlyBracket)
            parse_error(getLoc(), "expect '}'");
        consume();
        result->eidx = puttag(Name, result, full_loc, TYENUM);
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
            Location full_loc = getLoc();
            Declator nt = declarator(base, Function);
            if (!nt.ty)
                return expect(full_loc, "abstract-declarator"), false;
            params.push_back(nt);
            if (nt.ty->k == TYINCOMPLETE)
                type_error(full_loc, "parameter %u has imcomplete type %T", i, nt.ty), ok = false;
            if (nt.name) {
                if (sema.typedefs.getSymInCurrentScope(nt.name))
                    type_error(full_loc, "duplicate parameter %I", nt.name);
                sema.typedefs.putSym(nt.name, Variable_Info{.ty = nt.ty, .loc = full_loc, .tags = ASSIGNED | USED | PARAM});
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
    void checkAlign(uint64_t &a) {
        if (isPowerOf2_64(a))
            a = llvm::Log2_64(a);
        else
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
            uint64_t a;
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
            uint64_t a;
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
        uint64_t ok;
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
    bool assignable(Expr e) {
        if (e->k == EVar) {
            Variable_Info &var_info = sema.typedefs.getSym(e->sval);
            var_info.tags |= ASSIGNED;
            if (var_info.ty->tags & TYCONST) {
                type_error(getLoc(), "cannot modify const-qualified variable %R: %T", sema.typedefs.getSymName(e->sval),
                           var_info.ty);
                return false;
            }
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
        if (l.tok.tok == K_Static_assert) {
            consume_static_assert();
            return;
        }
        if (l.tok.tok == TSemicolon) {
            consume();
            return;
        }
        if (!(base = declaration_specifiers()))
            return expect(loc, "declaration-specifiers");
        if (sema.currentAlign) {
            size_t m = getAlignof(base) * 8;
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
            return insertStmt(SNEW(DeclOnlyStmt){.loc = loc, .decl = base});
        }
        result = SNEW(VarDeclStmt){.loc = loc, .vars = xvector<VarDecl>::get()};
        for (;;) {
            Location full_loc = getLoc();
            Declator st = declarator(base, Direct);
            if (!st.ty)
                return;
            if (l.tok.tok == TLcurlyBracket) {
                if (st.ty->k != TYFUNCTION)
                    return type_error(full_loc, "unexpected function definition");
                // function definition
                if (st.name->second.getToken() == PP_main) {
                    if (st.ty->params.size()) {
                        if (!(st.ty->params.front().ty->tags & TYINT)) {
                            type_error(full_loc, "first parameter of main is not 'int'");
                        }
                        if (st.ty->params.size() >= 2) {
                            if (!(st.ty->params[1].ty->k == TYPOINTER && st.ty->params[1].ty->p->k == TYPOINTER &&
                                  st.ty->params[1].ty->p->p->tags & (TYCHAR | TYUCHAR))) {
                                type_error(full_loc, "second parameter of main is not 'char**'");
                            }
                        }
                    }
                }
                size_t idx = putsymtype2(st.name, st.ty, full_loc);
                sema.pfunc = st.name;
                if (!isTopLevel())
                    return parse_error(full_loc, "function definition is not allowed here"),
                           note("function can only declared in global scope");
                Stmt res = SNEW(FunctionStmt){.loc = full_loc,
                                              .func_idx = idx,
                                              .funcname = st.name,
                                              .functy = st.ty,
                                              .args = xvector<size_t>::get_with_length(st.ty->params.size())};
                sema.currentfunctionRet = st.ty->ret;
                res->funcbody = function_body(st.ty->params, res->args, loc);
                sema.currentfunctionRet = nullptr;
                res->numLabels = jumper.cur;
                jumper.cur = 0;
                sreachable = true;
                for (auto it = sema.typedefs.begin(); it != sema.typedefs.end(); ++it)
                    if (!(it->info.ty->tags & TYCONST))
                        it->info.val = nullptr;
                return insertStmt(res);
            }
            st.ty->noralize();
#if 0
            print_cdecl(st.name->getKey(), st.ty, llvm::errs(), true);
#endif
            size_t idx = putsymtype(st.name, st.ty, full_loc);

            result->vars.push_back(VarDecl{.name = st.name, .ty = st.ty, .init = nullptr, .idx = idx});
            if (st.ty->tags & TYINLINE)
                warning(full_loc, "inline can only used in function declaration");
            if (!(st.ty->tags & TYEXTERN)) {
                if ((st.ty->tags & TYVOID) && !(st.ty->tags & TYTYPEDEF))
                    return type_error(full_loc, "variable %I declared void", st.name);
                if (st.ty->k == TYINCOMPLETE)
                    return type_error(full_loc, "variable %I has imcomplete type %T", st.name);
            }
            if (l.tok.tok == TAssign) {
                Expr init;
                auto &var_info = sema.typedefs.getSym(idx);
                var_info.tags |= ASSIGNED;
                if (st.ty->k == TYARRAY && st.ty->vla) {
                    return type_error(full_loc, "variable length array may not be initialized");
                } else if (st.ty->k == TYFUNCTION)
                    return type_error(full_loc, "function may not be initialized");
                if (st.ty->tags & TYTYPEDEF)
                    return type_error(full_loc, "'typedef' may not be initialized");
                consume();
                {
                    llvm::SaveAndRestore<CType> saved_ctype(sema.currentInitTy, st.ty);
                    init = initializer_list();
                }
                if (!init)
                    return expect(full_loc, "initializer-list");
                result->vars.back().init = init;
                if (st.ty->k == TYARRAY && !st.ty->hassize && !st.ty->vla) 
                    result->vars.back().ty = init->ty;
                if (init->k == EConstant && st.ty->tags & TYCONST)
                    var_info.val = init->C; // for const and constexpr, their value can be fold to constant
                if ((isTopLevel() || (st.ty->tags & (TYSTATIC | TYEXTERN))) && init->k != EConstant && init->k != EConstantArray &&
                    init->k != EConstantArraySubstript) {
                    // take address of globals is constant!
                    if (!
                        (init->k == EVar && 
                            (
                                sema.typedefs.isInGlobalScope(init->sval) || 
                                sema.typedefs.getSym(init->sval).ty->tags & (TYEXTERN | TYSTATIC)
                            )
                        )
                        ) {
                        type_error(full_loc, "global initializer is not constant");
                    }
                }
            } else {
                if (st.ty->k == TYARRAY) {
                    if (st.ty->vla && isTopLevel())
                        type_error(full_loc, "variable length array declaration not allowed at file scope");
                    else if (st.ty->hassize == false && !st.ty->vla && (st.ty->tag & TYEXTERN)) {
                        if (isTopLevel()) {
                            warning(full_loc, "array %I assumed to have one element", st.name);
                            st.ty->hassize = true, st.ty->arrsize = 1;
                        } else {
                            type_error(full_loc, "array size missing in %I", st.name);
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
                    return ENEW(VoidExpr){.loc = e->loc, .ty = context.getVoid(), .voidexpr = e};
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
        return declarator(base, Abstract).ty;
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
            valid_condition(e, true);
            return boolToInt(unary(e, LogicalNot, context.getInt()));
        }
        case TMul: {
            Expr e;
            consume();
            if (!(e = cast_expression()))
                return nullptr;
            if (e->ty->k != TYPOINTER)
                return type_error(loc, "pointer expected"), nullptr;
            make_deref(e, loc);
            return e;
        }
        case TBitNot: {
            Expr e;
            consume();
            if (!(e = cast_expression()))
                return nullptr;
            if (!checkInteger(e->ty))
                return type_error(loc, "integer type expected"), nullptr;
            integer_promotions(e);
            if (e->k == EConstant) // fold simple bitwise-not, e.g., ~0ULL
                return wrap(e->ty, llvm::ConstantExpr::getNot(e->C));
            return unary(e, Not, e->ty);
            ;
        }
        case TBitAnd: {
            Expr e;
            consume();
            if (!(e = expression()))
                return nullptr;
            if (e->k == EConstantArray) {
                assert(e->ty->k == TYPOINTER);
                e->ty = context.getPointerType(context.getFixArrayType(
                    e->ty->p, cast<llvm::ArrayType>(e->array->getValueType())->getNumElements()));
                return e;
            }
            if (e->k == EArrToAddress)
                return e->ty = context.getPointerType(e->arr3->ty), e;
            if (e->ty->k == TYBITFIELD)
                return type_error(loc, "cannot take address of bit-field"), e;
            if (e->ty->tags & TYREGISTER)
                return type_error(loc, "take address of register variable"), e;
            if (e->k == EUnary && e->uop == AddressOf && e->ty->p->k == TYFUNCTION)
                return e->ty->tags &= ~TYLVALUE, e;
            if (!assignable(e))
                return e;
            return unary(e, AddressOf, context.getPointerType(e->ty));
        }
        case TDash: {
            Expr e;
            consume();
            if (!(e = unary_expression()))
                return nullptr;
            if (!checkArithmetic(e->ty))
                return type_error(loc, "arithmetic type expected"), nullptr;
            integer_promotions(e);
            if (e->k == EConstant) { // fold simple negate numbers, e.g, -10
                if (e->ty->tags & TYCOMPLEX) {
                    auto CS = cast<llvm::ConstantStruct>(e->C);
                    if (auto CF = dyn_cast<ConstantFP>(CS->getOperand(0))) {
                        const auto &r = CF->getValue();
                        const auto &i = cast<ConstantFP>(CS->getOperand(1))->getValue();
                        auto REAL = ConstantFP::get(irgen.ctx, -r);
                        auto IMAG = ConstantFP::get(irgen.ctx, -i);
                        return wrap(e->ty, llvm::ConstantStruct::get(irgen.wrapComplex(e->ty), {REAL, IMAG}), e->loc);
                    }
                    const auto &r = cast<ConstantInt>(CS->getOperand(0))->getValue();
                    const auto &i = cast<ConstantInt>(CS->getOperand(1))->getValue();
                    auto REAL = ConstantInt::get(irgen.ctx, -r);
                    auto IMAG = ConstantInt::get(irgen.ctx, -i);
                    return wrap(e->ty, llvm::ConstantStruct::get(irgen.wrapComplexForInteger(e->ty), {REAL, IMAG}), e->loc);
                }
                return wrap(e->ty,
                            e->ty->isSigned() ? llvm::ConstantExpr::getNSWNeg(e->C) : llvm::ConstantExpr::getNeg(e->C),
                            e->loc);
            }
            return unary(e, e->ty->isSigned() ? SNeg : UNeg, e->ty);
        }
        case TAdd: {
            Expr e;
            consume();
            if (!(e = unary_expression()))
                return nullptr;
            if (!checkArithmetic(e->ty))
                return type_error(loc, "arithmetic type expected"), nullptr;
            integer_promotions(e);
            return e;
        }
        case TAddAdd:
        case TSubSub: {
            Expr e;
            consume();
            if (!(e = unary_expression()))
                return nullptr;
            if (!assignable(e))
                return e;
            CType ty = e->ty->k == TYPOINTER ? context.getSize_t() : e->ty;
            Expr obj = e;
            Expr one = wrap(ty, ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), 1)), e->loc);
            (tok == TAddAdd) ? make_add(e, one) : make_sub(e, one);
            return binop(obj, Assign, e, e->ty);
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
                    return wrap(context.getSize_t(),
                                ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getsizeof(ty))),
                                loc);
                }
                if (!(e = unary_expression()))
                    return nullptr;
                if (l.tok.tok != TRbracket)
                    return expectRB(getLoc()), nullptr;
                consume();
                return wrap(context.getSize_t(),
                            ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getsizeof(e))), loc);
            }
            if (!(e = unary_expression()))
                return nullptr;
            return wrap(context.getSize_t(),
                        ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getsizeof(e))), loc);
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
                result =
                    wrap(context.getSize_t(),
                         ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getAlignof(ty))), loc);
            } else {
                Expr e = constant_expression();
                if (!e)
                    return nullptr;
                result =
                    wrap(context.getSize_t(),
                         ConstantInt::get(irgen.ctx, APInt(context.getSize_t()->getBitWidth(), getAlignof(e))), loc);
            }
            if (l.tok.tok != TRbracket)
                return expectRB(getLoc()), nullptr;
            consume();
            return result;
        }
        default: return postfix_expression();
        }
    }
    static bool alwaysFitsInto64Bits(unsigned Radix, unsigned NumDigits) {
        switch (Radix) {
        case 2: return NumDigits <= 64;
        case 8: return NumDigits <= 64 / 3;
        case 10: return NumDigits <= 19;
        case 16: return NumDigits <= 64 / 4;
        default: llvm_unreachable("impossible Radix");
        }
    }

    // the input must be null-terminated
    Expr parse_pp_number(const xstring str) {
        Location loc = getLoc();
        const unsigned char *DigitsBegin = nullptr;
        bool isFPConstant = false;
        uint64_t radix;
        const unsigned char *s = reinterpret_cast<const unsigned char *>(str.data());
        if (str.size() == 2) {
            if (!llvm::isDigit(str.front()))
                return lex_error(loc, "expect one digit"), nullptr;
            return wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt(32, static_cast<uint64_t>(*s) - '0')));
        }
        if (*s == '0') {
            s++;
            switch (*s) {
            case '.': DigitsBegin = s; goto common;
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
            default: // 0j, 0i, 0LL, 0ULL, ...
                if (s[1] < '0' || s[1] > '7') {
                    DigitsBegin = s - 1;
                    radix = 10;
                    goto common;
                }
                DigitsBegin = s;
                radix = 8;
                do
                    ++s;
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
                    if (*s == '+' || *s == '-')
                        s++;
                    if (!llvm::isDigit(*s))
                        return lex_error(loc, "expect exponent digits"), nullptr;
                    do
                        s++;
                    while (llvm::isDigit(*s));
                }
            }
        }
        struct Suffixs {
            bool isUnsigned : 1, isLongLong : 1, isLong : 1, isFloat : 1, isFloat128 : 1, HasSize : 1, isImaginary : 1;
        } su{};
        const unsigned char *SuffixBegin = s;
        for (;;) {
            unsigned char c1;
            switch ((c1 = *s++)) {
            case '\0': goto NEXT;
            case 'f':
            case 'F':
                if (!isFPConstant || su.HasSize)
                    break;
                su.HasSize = su.isFloat = true;
                continue;
            case 'u':
            case 'U':
                if (isFPConstant || su.isUnsigned)
                    break;
                su.isUnsigned = true;
                continue;
            case 'l':
            case 'L':
                if (su.HasSize)
                    break;
                su.HasSize = true;
                if (c1 == *s) {
                    if (isFPConstant)
                        break;
                    su.isLongLong = true;
                    ++s;
                } else
                    su.isLong = true;
                continue;
            case 'i':
            case 'I':
            case 'j':
            case 'J':
                if (su.isImaginary)
                    break;
                su.isImaginary = true;
                continue;
            case 'q':
            case 'Q':
                if (!isFPConstant || su.HasSize)
                    break;
                su.isFloat128 = true;
                continue;
            }
            lex_error(loc, "invalid suffix '%c' in %s constant", c1, isFPConstant ? "floating" : "integer");
            break;
        }
NEXT:
        const uintptr_t NumDigits = (uintptr_t)SuffixBegin - (uintptr_t)DigitsBegin;
        if (isFPConstant) {
            StringRef fstr(str.data(),
                           std::min((uintptr_t)str.size() - 1, (uintptr_t)SuffixBegin - (uintptr_t)str.data()));
            CType ty = context.getDobule();
            const llvm::fltSemantics *Format = &llvm::APFloat::IEEEdouble();
            if (su.isFloat128) {
                ty = context.getFloat128();
                Format = &APFloat::IEEEquad();
            } else if (su.isFloat) {
                Format = &APFloat::IEEEsingle();
                ty = context.getFloat();
            }
            APFloat F(*Format);
            auto it = F.convertFromString(fstr, APFloat::rmNearestTiesToEven);
            if (auto err = it.takeError()) {
                std::string msg = llvm::toString(std::move(err));
                lex_error(loc, "error parsing floating literal: %R", StringRef(msg));
            } else {
                auto status = *it;
                SmallString<20> buffer;
                if ((status & APFloat::opOverflow) || ((status & APFloat::opUnderflow) && F.isZero())) {
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
            if (su.isImaginary) {
                ty = tryGetComplexTypeFromNonComplex(ty->tags);
                llvm::StructType *T = irgen.wrapComplex(ty->tags);
                return wrap(ty, llvm::ConstantStruct::get(T, {
                    ConstantFP::getZero(T), ConstantFP::get(irgen.ctx, F)
                }), loc);
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
        uint64_t H = bigVal.high(), L = bigVal.low();
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
                    ty = context.u128;
                else
                    ty = context.i128;
            } else {
                /* 64 bits */
                if ((int64_t)L < 0) /* test MSB */
                    ty = context.u64;
                else {
                    if (L >> 32) { /* more than 32 bits ? */
                        ty = context.i64;
                    } else {                /* 32 bits */
                        if ((int32_t)L < 0) /* test MSB */
                            ty = context.u32;
                        else
                            ty = context.i32;
                    }
                }
            }
        }
        ConstantInt *CI = H ? 
                ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), {H, L})) :
                ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), L));
        if (su.isImaginary) {
            ty = context.make(ty->tags | TYCOMPLEX);
            llvm::StructType *T = irgen.wrapComplexForInteger(ty->tags);
            return wrap(ty, llvm::ConstantStruct::get(T, {
                ConstantInt::get(irgen.ctx, APInt::getZero(CI->getValue().getBitWidth())), CI
            }), loc);
        }
        return wrap(ty, CI, loc);
    }
    Expr wrap(CType ty, llvm::Constant *C, Location loc = Location()) {
        return ENEW(ConstantExpr){.loc = loc, .ty = ty, .C = C};
    }
    Expr getIntZero() { return intzero; }
    Expr getIntOne() { return intone; }
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
                ty = context.getWchar();
                break;
            case Iulong: // Iulong: U'c'
                ty = context.getChar16_t();
                break;
            case Ilonglong: // Ilonglong: L'c'
                ty = context.getWchar();
                break;
            default: llvm_unreachable("");
            }
            result = wrap(ty, ConstantInt::get(irgen.ctx, APInt(ty->getBitWidth(), l.tok.i)), loc);
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
                               "unsupported non-standard concatenation of string literals for UTF-%u and UTF-%u",
                               (unsigned)enc, (unsigned)l.tok.enc);
                consume();
            }
            switch (enc) {
            case 8:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.str8ty, .array = string_pool.getAsUTF8(s)};
                break;
            case 16:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.str16ty, .array = string_pool.getAsUTF16(s, context.getWcharTag() & (TYINT32 | TYUINT32))};
                break;
            case 32:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.str32ty, .array = string_pool.getAsUTF32(s)};
                break;
            default: llvm_unreachable("bad encoding");
            }
        } break;
        case PPNumber: {
            result = parse_pp_number(l.tok.str);
            if (result)
                result->loc = getLoc();
            else
                result = getIntZero();
            consume();
        } break;
        case TIdentifier: {
            if (l.want_expr)
                result = getIntZero();
            else if (l.tok.s->second.getToken() == PP__func__) {
                xstring s = xstring::get(sema.pfunc->getKey());
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.str8ty, .array = string_pool.getAsUTF8(s)};
            } else {
                IdentRef sym = l.tok.s;
                size_t idx;
                Variable_Info *it = sema.typedefs.getSym(sym, idx);
                consume();
                if (!it)
                    return type_error(loc, "use of undeclared identifier %I", sym), getIntZero();
                if (it->ty->tags & TYTYPEDEF)
                    return type_error("typedefs are not allowed here %I", sym), getIntZero();
                it->tags |= USED;
                CType ty = context.clone(it->ty);
                // lvalue conversions
                ty->tags &= (ty_prim | TYCOMPLEX | TYIMAGINARY);
                ty->tags |= TYLVALUE;
                if (it->val)
                    return wrap(ty, it->val, loc);
                switch (ty->k) {
                case TYFUNCTION:
                    result =
                        unary(ENEW(VarExpr){.loc = loc, .ty = ty, .sval = idx}, AddressOf, context.getPointerType(ty));
                    break;
                case TYARRAY:
                    result = ENEW(ArrToAddressExpr){.loc = loc,
                                                    .ty = context.getPointerType(ty->arrtype),
                                                    .arr3 = ENEW(VarExpr){.loc = loc, .ty = ty, .sval = idx}};
                    break;
                default: result = ENEW(VarExpr){.loc = loc, .ty = ty, .sval = idx};
                }
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
                        return type_error(loc, "more then one compatible types in Generic expression"), nullptr;
                    result = e;
                }
                switch (l.tok.tok) {
                case TComma: consume(); continue;
                case TRbracket: consume(); goto GENERIC_END;
                default: return expect(loc, "','' or ')'"), nullptr;
                }
            }
GENERIC_END:
            if (result)
                return result;
            if (defaults)
                return defaults;
            return type_error(loc, "Generic expression(has type %T) not compatible with any generic association type",
                              testty),
                   nullptr;
        } break;
        default:
            return parse_error(loc, "unexpected token in primary_expression: %s\n", show(l.tok.tok)), consume(),
                   nullptr;
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
            case TSubSub: isadd = PostfixDecrement; goto ADD;
            case TAddAdd: {
                isadd = PostfixIncrement;
ADD:
                if (!assignable(result))
                    return nullptr;
                consume();
                result = ENEW(PostFixExpr){.loc = result->loc, .ty = result->ty, .pop = isadd, .poperand = result};
            }
                continue;
            case TArrow: isarrow = true; goto DOT;
            case TDot: {
                isarrow = false;
DOT:
                CType ty = result->ty;
                bool isLvalue = false;
                consume();
                if (l.tok.tok != TIdentifier)
                    return expect(getLoc(), "identifier"), nullptr;
                if (isarrow) {
                    if (result->ty->k != TYPOINTER)
                        return type_error(getLoc(), "member reference type %T is not a pointer; did you mean to use '.'"), result;
                    ty = result->ty->p;
                    isLvalue = true;
                } else {
                    if (result->ty->k == TYPOINTER) {
                        return type_error("member reference type %T is a pointer; did you mean to use '->'", result->ty), result;
                    }
                }
                if (ty->k != TYSTRUCT && ty->k != TYUNION)
                    return type_error(getLoc(), "member access is not struct or union"), result;
                for (size_t i = 0; i < ty->selems.size(); ++i) {
                    Declator pair = ty->selems[i];
                    if (l.tok.s == pair.name) {
                        result = ENEW(MemberAccessExpr) {
                            .loc = result->loc, .ty = pair.ty, .obj = result, .idx = (unsigned)i};
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
                CType ty = (result->k == EUnary && result->uop == AddressOf)
                               ? result->ty->p
                               : ((result->ty->k == TYPOINTER) ? result->ty->p : result->ty);
                Expr f = result;
                result =
                    ENEW(CallExpr){.loc = result->loc, .ty = ty->ret, .callfunc = f, .callargs = xvector<Expr>::get()};
                if (ty->k != TYFUNCTION)
                    return type_error(getLoc(), "expect function or function pointer, but the expression has type %T",
                                      ty),
                           nullptr;
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
                        return type_error(getLoc(), "too few arguments to variable argument function"),
                               note("at lease %z arguments needed", (size_t)(params.size() + 1)), nullptr;
                    for (size_t j = 0; j < params.size(); ++j) {
                        Expr e = castto(result->callargs[j], params[j].ty, Implict_Call);
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
                        return type_error(getLoc(), "expect %z parameters, %z arguments provided",
                                          (size_t)params.size(), (size_t)result->callargs.size()),
                               nullptr;
                    for (size_t j = 0; j < result->callargs.size(); ++j) {
                        Expr e = castto(result->callargs[j], params[j].ty, Implict_Call);
                        if (!e)
                            break;
                        result->callargs[j] = e;
                    }
                }
            } break;
            case TLSquareBrackets: // array subscript
            {
                Expr rhs;
                consume();
                if (!(rhs = expression()))
                    return result;
                if (result->ty->k != TYPOINTER && rhs->ty->k != TYPOINTER) {
                    type_error(getLoc(), "subscripted value is not an array, pointer, or vector");
                } else {
                    result = make_add_pointer(result, rhs);
                    make_deref(result, getLoc());
                }
                if (l.tok.tok != TRSquareBrackets)
                    warning(getLoc(), "missing ']' after array subscript");
                else
                    consume();
            } break;
            default: return result;
            }
        }
    }
    void make_deref(Expr &e, Location loc) {
        assert(e->ty->k == TYPOINTER && "bad call to make_deref: expect a pointer");
        if ((e->ty->p->k == TYINCOMPLETE) || (e->ty->p->tags & TYVOID)) {
            type_error(loc, "dereference from incomplete/void type: %T", e->ty);
            return;
        }
        if (e->k == EConstantArraySubstript) {
            uint64_t i = e->cidx.getLimitedValue();
            llvm::Constant *C = e->carray->getInitializer();
            if (auto CS = dyn_cast<llvm::ConstantDataSequential>(C)) {
                const unsigned numops = CS->getNumElements();
                if (e->cidx.uge(numops))
                    warning(loc, "array index %A is past the end of the array (which contains %u elements)",
                            &e->cidx, numops);
                else
                    return (void)(e = wrap(e->ty->p, CS->getElementAsConstant(e->cidx.getZExtValue()), e->loc));

            } else {
                auto CA = cast<llvm::ConstantAggregate>(C);
                const unsigned numops = CA->getNumOperands();
                if (e->cidx.uge(numops))
                    warning(loc, "array index %A is past the end of the array (which contains %u elements)",
                            &e->cidx, numops);
                else
                    return (void)(e = wrap(e->ty->p, CA->getOperand(i), e->loc));
            }
        }
        CType ty = context.clone(e->ty->p);
        ty->tags |= TYLVALUE;
        e = unary(e, Dereference, ty);
    }
    void tryContinue() {
        if (jumper.topContinue == INVALID_LABEL)
            type_error(getLoc(), "%s", "connot continue: no outer for/while/do-while");
    }
    Expr getCBool(bool b) { return b ? ctrue : cfalse; }
    Expr getBool(bool b) { return b ? getIntOne() : getIntZero(); }
    void foldBool(Expr &e, bool reverse = false) {
        llvm::Constant *C = e->C;
        switch (C->getValueID()) {
        case llvm::Value::ConstantIntVal: {
            const auto CI = cast<ConstantInt>(C);
            e = getBool(reverse ? CI->isZero() : !CI->isZero());
        } break;
        case llvm::Value::ConstantFPVal: {
            const auto CFP = cast<ConstantFP>(C);
            e = getBool(reverse ? CFP->isZero() : CFP->getValue().isNonZero());
        } break;
        case llvm::Value::UndefValueVal:
        case llvm::Value::PoisonValueVal: {
            /* TODO: undefined value is true/false ? */
        } break;
        case llvm::Value::ConstantStructVal: {
            const auto CT = cast<llvm::ConstantStruct>(C);
            assert(CT->getNumOperands() == 2 && "bad complex constant!");
            const auto O0 = CT->getOperand(0),
                       O1 = CT->getOperand(1);
            if (const auto CI1 = dyn_cast<ConstantInt>(O0)) {
                const auto CI2 = cast<ConstantInt>(O1);
                bool c = !CI1->isZero() || !CI2->isZero();
                if (reverse)
                    c = !c;
                e = getBool(c);
            } else {
                const auto CF1 = cast<ConstantFP>(O0);
                const auto CF2 = cast<ConstantFP>(O1);
                bool c = !CF1->isZero() || !CF2->isZero();
                if (reverse)
                    c = !c;
                e = getBool(c);
            }
        } break;
        case llvm::Value::ConstantAggregateZeroVal: {
            assert((e->ty->tags & TYCOMPLEX) && "only complex number/structures are able to convert to bool");
            e = getBool(reverse);
        } break;
        case llvm::Value::ConstantPointerNullVal: {
            e = getBool(reverse);
        } break;
        default: break;
        }
    }
    const APInt *want_integer_constant() {
        Location loc = getLoc();
        Expr e = constant_expression();
        if (!e)
            return nullptr;
        if (sema.currentswitch)
            e = castto(e, sema.currentswitch->itest->ty);
        if (e->k != EConstant)
            return type_error(loc, "case value is not a integer constant expression"), nullptr;
        if (auto CI = dyn_cast<ConstantInt>(e->C))
            return &CI->getValue();
        return type_error(loc, "statement requires expression of integer type (%T invalid)", e->ty), nullptr;
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
                e = reverse ? getIntZero() : getIntOne();
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
    void setInsertPoint(Stmt I) { InsertPt = I; }
    Stmt getInsertPoint() { return InsertPt; }
    void clearInsertPoint() { InsertPt = nullptr; }
    void jumpIfTrue(Expr test, label_t dst) {
        label_t thenBB = jumper.createLabel();
        if (test->k == EConstant) {
            if (auto CI = dyn_cast<ConstantInt>(test->C)) {
                insertBr(CI->isZero() ? thenBB : dst);
                goto NEXT;
            }
        }
        insertStmt(SNEW(CondJumpStmt){.loc = Location(), .test = test, .T = dst, .F = thenBB});
NEXT:
        return insertLabel(thenBB);
    }
    void jumpIfFalse(Expr test, label_t dst) {
        label_t thenBB = jumper.createLabel();
        if (test->k == EConstant) {
            if (auto CI = dyn_cast<ConstantInt>(test->C)) {
                insertBr(CI->isZero() ? dst : thenBB);
                goto NEXT;
            }
        }
        insertStmt(SNEW(CondJumpStmt){.loc = Location(), .test = test, .T = thenBB, .F = dst});
NEXT:
        return insertLabel(thenBB);
    }
    void condJump(Expr test, label_t T, label_t F) {
        return insertStmt(SNEW(CondJumpStmt){.loc = Location(), .test = test, .T = T, .F = F});
    }
    void insertBr(label_t L, Location loc = Location()) {
        return insertStmt(SNEW(GotoStmt){.loc = loc, .location = L});
    }
    void insertLabel(label_t L, IdentRef Name = nullptr) {
        sreachable = true;
        return insertStmtInternal(SNEW(LabelStmt){.loc = Location(), .label = L, .labelName = Name});
    }
    void insertStmtInternal(Stmt s) {
        InsertPt->next = s;
        InsertPt = s;
    }
    void insertStmt(Stmt s) {
        if (sreachable || s->k == SCompound) {
            insertStmtInternal(s);
            if (s->isTerminator()) {
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
    void insertStmtEnd() { InsertPt->next = nullptr; }
    void statement() {
        // parse a statement
        Location loc = getLoc();
        switch (l.tok.tok) {
        case TSemicolon: return consume();
        case K__asm__: return parse_asm(), checkSemicolon();
        case TLcurlyBracket: return compound_statement();
        case Kcase: {
            const APInt *CaseStart = nullptr, *CaseEnd = nullptr;
            consume();
            CaseStart = want_integer_constant();
            if (l.tok.tok == TEllipsis)
                consume(), CaseEnd = want_integer_constant();
            if (l.tok.tok != TColon)
                parse_error(getLoc(), "missing ':' after 'case'");
            else
                consume();
            if (!sema.currentswitch)
                parse_error(loc, "'default' statement not in switch statement");
            else {
                if (CaseStart) {
                    label_t L = jumper.createLabel();
                    insertLabel(L);
                    if (!CaseEnd)
                        sema.currentswitch->switchs.push_back(SwitchCase(loc, L, CaseStart));
                    else {
                        if (*CaseStart == *CaseEnd)
                            sema.currentswitch->switchs.push_back(SwitchCase(loc, L, CaseStart));
                        else {
                            if (CaseStart->ugt(*CaseEnd))
                                warning(loc, "empty case range specified");
                            else {
                                sema.currentswitch->gnu_switchs.push_back(GNUSwitchCase(loc, L, CaseStart, CaseEnd));
                            }
                        }
                    }
                }
            }
            if (l.tok.tok == TRcurlyBracket)
                parse_error(getLoc(), "label at end of compound statement: expected statement");
            else
                statement();
        }
            return;
        case Kdefault: {
            consume();
            if (l.tok.tok != TColon)
                parse_error(loc, "missing ':' after 'default'");
            else
                consume();
            if (!sema.currentswitch)
                parse_error(loc, "'default' statement not in switch statement");
            else if (sema.currentswitch->sw_default_loc.isValid()) {
                parse_error(loc, "multiple default labels in one switch");
                note(sema.currentswitch->sw_default_loc, "previous default defined here");
            } else {
                sema.currentswitch->sw_default_loc = loc;
                insertLabel(sema.currentswitch->sw_default);
            }
            if (l.tok.tok == TRcurlyBracket)
                parse_error(getLoc(), "label at end of compound statement: expected statement");
            else
                statement();
        }
            return;
        case Kgoto: {
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
        case Kbreak: {
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
        case Kreturn: {
            consume();
            if (l.tok.tok == TSemicolon) {
                consume();
                if (sema.currentfunctionRet->tags & TYVOID)
                    return insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = nullptr});

                return warning(loc, "function should not return void in a function return %T", sema.currentfunctionRet),
                       note("%s", "A return statement without an expression shall only appear in a function whose "
                                  "return type is void"),
                       insertStmt(SNEW(ReturnStmt){
                           .loc = loc,
                           .ret = wrap(sema.currentfunctionRet,
                                       llvm::UndefValue::get(irgen.wrap2(sema.currentfunctionRet)), loc)});
            }
            Expr e;
            if (!(e = expression()))
                return;
            checkSemicolon();
            if (sema.currentfunctionRet->tags & TYVOID)
                return warning(loc, "function should return a value in a function return void"),
                       note("A return statement with an expression shall not appear in a function whose return type is "
                            "void"),
                       insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = nullptr});
            return insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = castto(e, sema.currentfunctionRet, Implict_Return)});
        }
        case Kwhile:
        case Kswitch: {
            Expr test;
            Token tok = l.tok.tok;
            if (!(test = Bexpression()))
                return;
            if (tok == Kswitch) {
                integer_promotions(test);
                if (test->ty->k != TYPRIM && !(test->ty->tags & intergers_or_bool)) {
                    type_error("switch requires expression of integer type (%T invalid)", test->ty);
                    test->ty = context.getInt();
                }
                label_t LEAVE = jumper.createLabel();
                Stmt sw = SNEW(SwitchStmt){.loc = loc,
                                           .itest = test,
                                           .switchs = xvector<SwitchCase>::get(),
                                           .gnu_switchs = xvector<GNUSwitchCase>::get_with_capacity(0),
                                           .sw_default = jumper.createLabel(),
                                           .sw_default_loc = Location()};
                llvm::SaveAndRestore<Stmt> saved_sw(sema.currentswitch, sw);
                llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
                insertStmt(sw); // insert switch instruction
                unreachable_reason = sw;
                sreachable = false;
                statement();
                if (!sema.currentswitch->sw_default_loc.isValid()) {
                    // no default found: insert `default: break;`
                    sema.currentswitch->sw_default_loc = getLoc();
                    insertLabel(sema.currentswitch->sw_default);
                }
                insertLabel(LEAVE);
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
            insertStmt(SNEW(CondJumpStmt){.loc = Location(), .test = test, .T = BODY, .F = LEAVE});
            insertLabel(LEAVE);
            return;
        }
        case Kfor: {
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
                    if (ex->isSimple())
                        warning(loc, "expression in for-clause-1(for loop initialization) has no effect");
                    else
                        insertStmt(SNEW(ExprStmt){.loc = ex->loc, .exprbody = ex});
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
                if (forincl->isSimple()) {
                    warning(loc3, "expression in expression-3(for loop increment) has no effect");
                    forincl = nullptr;
                }
            }
            consume();
            label_t CMP = jumper.createLabel();
            label_t LEAVE = jumper.createLabel();
            llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
            llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
            if (cond)
                condJump(cond, CMP, LEAVE);
            else
                insertLabel(CMP);
            if (forincl)
                insertStmt(SNEW(ExprStmt){.loc = forincl->loc, .exprbody = forincl});
            statement();
            insertBr(CMP);
            insertLabel(LEAVE);
            leaveBlock();
            return;
        }
        case Kdo: {
            Expr test;
            label_t CMP = jumper.createLabel();
            label_t LEAVE = jumper.createLabel();
            llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
            llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
            insertLabel(CMP);
            consume();
            statement();
            if (l.tok.tok != Kwhile)
                parse_error(loc, "missing 'while' in do-while statement");
            else
                consume();
            if (l.tok.tok != TLbracket)
                return expectLB(loc);
            consume();
            if (!(test = Bexpression()))
                return;
            checkSemicolon();
            jumpIfTrue(test, CMP);
            condJump(test, CMP, LEAVE);
        }
        case Kif: {
            Expr test;
            if (!(test = Bexpression()))
                return;
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
        case Kelse: return parse_error(loc, "'else' without a previous 'if'"), consume();
        case TIdentifier: {
            Location full_loc = getLoc();
            TokenV tok = l.tok;
            consume();
            if (l.tok.tok == TColon) { // labeled-statement
                consume();
                label_t L = putLable(tok.s, full_loc);
                insertLabel(L, tok.s);
                if (l.tok.tok == TRcurlyBracket) {
                    warning(full_loc, "missing statement after label, add ';' for you");
                    return;
                }
                return statement();
            }
            l.tokenq.push_back(l.tok), l.tok = tok;
            // not a labeled-statement, now put the token back and try to parse a expression
        }
        default: break;
        }
        Expr e = expression();
        if (!e)
            return;
        e = simplify(e);
        if (e->isSimple())
            warning(loc, "expression statement has no effect");
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
            default: return result;
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
            case TGe: make_cmp(result, l.tok.tok);
            default: return result;
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
            case TNe: make_cmp(result, l.tok.tok, true);
            default: return result;
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
                if (result->k == EConstant) {
                    if (auto CI = dyn_cast<ConstantInt>(result->C)) {
                        if (CI->isZero()) {
                            warning(result->loc, "logical AND first operand is zero always evaluates to zero");
                            result = getIntZero();
                            continue;
                        }
                        if (r->k == EConstant) {
                            if (auto CI2 = dyn_cast<ConstantInt>(r->C))
                                result = CI2->isZero() ? getIntZero() : getIntOne();
                            else
                                result = r;
                            continue;
                        }
                    }
                } else if (r->k == EConstant) {
                    if (auto CI = dyn_cast<ConstantInt>(r->C)) {
                        if (CI->isZero()) {
                            warning(result->loc, "logical AND second operand is zero always evaluates to zero");
                            result = getIntZero();
                        }
                        /* else { result = result; }*/
                        continue;
                    }
                }
                if (result->k == EConstant && r->k == EConstant) {
                    result = wrap(context.getInt(), llvm::ConstantExpr::getAnd(result->C, r->C), result->loc);
                } else if (result->k == ECast && result->castop == ZExt && r->k == ECast && r->castop == ZExt) {
                    result = boolToInt(binop(result->castval, LogicalAnd, r->castval, context.getInt()));
                } else {
                    result = boolToInt(binop(result, LogicalAnd, r, context.getInt()));
                }
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
                if (result->k == EConstant) {
                    if (auto CI = dyn_cast<ConstantInt>(result->C)) {
                        if (!CI->isZero()) {
                            warning(result->loc, "logical OR first operand is non-zero always evaluates to non-zero");
                            result = getIntOne();
                            continue;
                        }
                        if (r->k == EConstant) {
                            if (auto CI2 = dyn_cast<ConstantInt>(r->C))
                                result = CI2->isZero() ? getIntZero() : getIntOne();
                            else
                                result = r;
                            continue;
                        }
                    }
                } else if (r->k == EConstant) {
                    if (auto CI = dyn_cast<ConstantInt>(r->C)) {
                        if (!CI->isZero()) {
                            warning(result->loc, "logical OR second operand is non-zero always evaluates to non-zero");
                            result = getIntZero();
                        }
                        /* else { result = result; }*/
                        continue;
                    }
                }
                if (result->k == EConstant && r->k == EConstant) {
                    result = wrap(context.getInt(), llvm::ConstantExpr::getOr(result->C, r->C), result->loc);
                } else if (result->k == ECast && result->castop == ZExt && r->k == ECast && r->castop == ZExt) {
                    result = boolToInt(binop(result->castval, LogicalOr, r->castval, context.getInt()));
                } else {
                    result = boolToInt(binop(result, LogicalOr, r, context.getInt()));
                }
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
                if (result->isSimple()) {
                    warning(result->loc,
                            "left-hand side of comma expression has no effect, replace with right-hand side");
                    result = r;
                } else
                    result = binop(result, Comma, r, result->ty);
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
            warning(getLoc(), "incompatible type for conditional-expression: (%T and %T)", lhs->ty, rhs->ty);
        conv(lhs, rhs);
        valid_condition(start);
        if (start->k == EConstant) {
            if (auto CI = dyn_cast<ConstantInt>(start->C)) {
                return CI->isZero() ? rhs : lhs;
            }
        }
        return ENEW(ConditionExpr){.loc = start->loc, .ty = lhs->ty, .cond = start, .cleft = lhs, .cright = rhs};
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
        case TAsignBitXor: return true;
        default: return false;
        }
    }
    void make_assign(Token tok, Expr &lhs, Expr &rhs) {
        switch (tok) {
        case TAssign: return;
        case TAsignAdd: return make_add(lhs, rhs);
        case TAsignSub: return make_sub(lhs, rhs);
        case TAsignMul: return make_mul(lhs, rhs);
        case TAsignDiv: return make_div(lhs, rhs);
        case TAsignRem: return make_rem(lhs, rhs);
        case TAsignShl: return make_shl(lhs, rhs);
        case TAsignShr: return make_shr(lhs, rhs);
        case TAsignBitAnd: return make_bitop(lhs, rhs, And);
        case TAsignBitOr: return make_bitop(lhs, rhs, Or);
        case TAsignBitXor: return make_bitop(lhs, rhs, Xor);
        default: llvm_unreachable("bad assignment operator!");
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
            consume();
            if (!assignable(result)) {
                (void)assignment_expression();
                return result;
            }
            if (!(e = assignment_expression()))
                return nullptr;
            if (result->ty->tags & TYATOMIC) {
                BinOp op = getAtomicrmwOp(tok);
                if (op) {
                    if (!(rhs = castto(e, result->ty)))
                        return rhs;
                    return binop(result, op, rhs, result->ty);
                }
            }
            // make `a += b` to `a = a + b`
            make_assign(tok, result, e);
            rhs = castto(e, result->ty, Implict_Assign);
            return binop(result, Assign, rhs, result->ty);
        } else
            return result;
    }
    Stmt translation_unit() {
        Stmt head = SNEW(HeadStmt){.loc = getLoc()};
        setInsertPoint(head);
        sreachable = true;
        while (l.tok.tok != TEOF) {
            if (l.tok.tok == Kasm)
                parse_asm(), checkSemicolon();
            else
                declaration();
            if (getNumErrors())
                break;
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
        statics("Parse scope statics\n");
        statics("  Max typedef scope size: %zu\n", sema.typedefs.maxSyms);
        statics("  Max tags scope size: %zu\n", sema.tags.maxSyms);
        endStatics();
        num_typedefs = sema.typedefs.maxSyms;
        num_tags = sema.tags.maxSyms;
        return ast;
    }
    void compound_statement() {
        consume();
        enterBlock();
        NullStmt nullstmt{.k = SHead}; // make a dummy statement in stack, remove it when scope leaves
        Stmt head = SNEW(CompoundStmt){.loc = getLoc(), .inner = reinterpret_cast<Stmt>(&nullstmt)};
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
            if (getNumErrors())
                break;
        }
    }
    Stmt function_body(const xvector<Param> params, xvector<size_t> &args, Location loc) {
        consume();
        Stmt head = SNEW(HeadStmt){.loc = loc};
        sema.typedefs.push_function();
        enterBlock();
        assert(jumper.cur == 0 && "labels scope should be empty in start of function");
        Location loc4 = getLoc();
        for (size_t i = 0; i < params.size(); ++i)
            args[i] =
                sema.typedefs.putSym(params[i].name, Variable_Info{.ty = params[i].ty, .loc = loc4, .tags = ASSIGNED});
        {
            llvm::SaveAndRestore<Stmt> saved_ip(InsertPt, head);
            eat_compound_statement();
            if (!getInsertPoint()->isTerminator()) {
                Stmt s = getInsertPoint();
                Location loc2 = getLoc();
                if (sema.pfunc->second.getToken() == PP_main) {
                    insertStmt(SNEW(ReturnStmt){.loc = loc2, .ret = getIntZero()});
                } else {
                    if (!(sema.currentfunctionRet->tags & TYVOID)) {
                        // if this label never reaches (never break to it)
                        if (s->k != SLabel || (s->labelName == nullptr && !jumper.used_breaks.contains(s->label))) {
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
        for (const auto &it : jumper.labels) {
            IdentRef name = it.first;
            switch (it.second.flags) {
            case LBL_FORWARD: type_error(it.second.loc, "use of undeclared label: %I", name); break;
            case LBL_DECLARED: warning(it.second.loc, "unused label: %I", name); break;
            case LBL_OK: break;
            default: llvm_unreachable("");
            }
        }
        jumper.used_breaks.clear();
        leaveBlock();
        sema.typedefs.pop_function();
        consume();
        return head;
    }

}; // end class Parser
