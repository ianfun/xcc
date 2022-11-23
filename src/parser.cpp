// A recursive descent parser(https://en.wikipedia.org/wiki/Recursive_descent_parser)
// for C language(https://open-std.org/JTC1/SC22/WG14/www/docs/n3054.pdf, https://en.cppreference.com/w/c/23).
//
// Also,
// static type checking,
// syntax analysis(https://en.wikipedia.org/wiki/Syntax_(programming_languages), https://en.wikipedia.org/wiki/Parsing),
// and Semantics(https://en.wikipedia.org/wiki/Semantics_(computer_science)).

struct Parser : public EvalHelper {
    enum {
        LBL_UNDEFINED = 0, LBL_FORWARD = 1, LBL_DECLARED = 2, LBL_OK = 4
    };
    struct Type_info {
        CType ty = nullptr;
        Location loc = Location();
    };
    struct Label_Info {
        label_t idx = 0;
        uint8_t flags = LBL_UNDEFINED;
        Location loc = Location();
    };
    enum {
        LOCAL_GARBAGE = 0,         // just declared
        USED = 0x1,                // set if variable is used
        ASSIGNED = 0x2,            // set if assigned or initialized
        PARAM = 0x4,               // set if is a parameter
        CONST_VAR = 0x5
    };
    struct Variable_Info {
        CType ty = nullptr;
        Location loc = Location();
        llvm::Constant *val = nullptr;
        uint8_t tags = 0;
    };
    enum Implict_Conversion_Kind: unsigned char {
        Implict_Cast,
        Implict_Assign,
        Implict_Init,
        Implict_Return,
        Implict_Call
    };
    Lexer l;
    xcc_context &context;
    struct Sema { // Semantics processor
        CType currentfunction = nullptr, currentInitTy = nullptr;
        Stmt currentswitch = nullptr;
        IdentRef pfunc; // current function name: `__func__`
        FunctionAndBlockScope<Variable_Info> typedefs;
        BlockScope<Type_info> tags;
        uint8_t currentAlign = 0; // current align(bytes)
        bool type_error = false;   // type error
    } sema;
    struct JumpBuilder {
        DenseMap<IdentRef, Label_Info> labels{}; // named labels
        unsigned cur = 0;
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
    llvm::ConstantPointerNull *null_ptr;
    Expr null_ptr_expr;
    template <typename T> auto getsizeof(T a) { return irgen.getsizeof(a); }
    template <typename T> auto getAlignof(T a) { return irgen.getAlignof(a); }
private:
    [[nodiscard]] Expr binop(Expr a, BinOp op, Expr b, CType ty) {
        return ENEW(BinExpr){.loc = a->loc, .ty = ty, .lhs = a, .bop = op, .rhs = b};
    }
    [[nodiscard]] Expr unary(Expr e, UnaryOp op, CType ty) {
        return ENEW(UnaryExpr){.loc = e->loc, .ty = ty, .uoperand = e, .uop = op};
    }
    Expr complex_from_real(Expr real, CType ty) {
        ty = context.tryGetComplexTypeFromNonComplex(ty);
        auto zero = llvm::Constant::getNullValue(irgen.wrapNoComplexScalar(ty));
        if (real->k == EConstant)
            return wrap(ty, llvm::ConstantStruct::get(irgen.wrapComplex(ty), {real->C, zero}), real->loc);
        return binop(real, Complex_CMPLX, wrap(ty, zero), ty);
    }
    Expr complex_from_imag(Expr imag, CType ty) {
        ty = context.tryGetComplexTypeFromNonComplex(ty);
        auto zero = ConstantFP::getZero(irgen.wrapNoComplexScalar(ty));
        if (imag->k == EConstant)
            return wrap(ty, llvm::ConstantStruct::get(irgen.wrapComplex(ty), {zero, imag->C}), imag->loc);
        return binop(wrap(ty, zero), Complex_CMPLX, imag, ty);
    }
    Expr complex_pair(Expr a, Expr b, CType ty) {
        return (a->k == EConstant && b->k == EConstant) ?
            wrap(ty, llvm::ConstantStruct::get(irgen.wrapComplex(ty), {a->C, b->C})) :
            binop(a, Complex_CMPLX, b, ty);
    }
    Expr complex_get_real(Expr e) {
        if (e->k == EConstant) {
            if (const auto AZ = dyn_cast<ConstantAggregateZero>(e->C))
                return wrap(e->ty, AZ->getElementValue((unsigned)0), e->loc);
            if (const auto CS = dyn_cast<llvm::ConstantStruct>(e->C))
                return wrap(e->ty, CS->getOperand(0), e->loc);
        }
        if (e->k == EBin && e->bop == Complex_CMPLX) {
            return e->lhs;
        }
        return unary(e, C__real__, context.getComplexElementType(e->ty));
    }
    Expr complex_get_imag(Expr e) {
        if (e->k == EConstant) {
            if (const auto AZ = dyn_cast<ConstantAggregateZero>(e->C))
                return wrap(e->ty, AZ->getElementValue((unsigned)1), e->loc);
            if (const auto CS = dyn_cast<llvm::ConstantStruct>(e->C))
                return wrap(e->ty, CS->getOperand(1), e->loc);
        }
        if (e->k == EBin && e->bop == Complex_CMPLX) {
            return e->rhs;
        }
        return unary(e, C__imag__, context.getComplexElementType(e->ty));
    }
    Expr complex_pair(Expr a, Expr b) {
        assert(a && b && "complex_pair: nullptr is invalid");
        return complex_pair(a, b, context.tryGetComplexTypeFromNonComplex(a->ty));
    }
    Expr complex_zero(CType ty) {
        assert(ty->isComplex());
        auto T = irgen.wrapComplex(ty);
        auto zero = llvm::Constant::getNullValue(T->getTypeAtIndex((unsigned)0));
        return wrap(ty, llvm::ConstantStruct::get(T, {zero, zero}));
    }
    Expr complex_neg_zero(CType ty) {
        assert(ty->isComplex());
        auto T = irgen.wrapComplex(ty);
        auto neg_zero = ConstantFP::getNegativeZero(T->getTypeAtIndex((unsigned)0));
        return wrap(ty, llvm::ConstantStruct::get(T, {neg_zero, neg_zero}));
    }
    Expr complex_pos_neg_zero(CType ty) {
        assert(ty->isComplex());
        auto T = irgen.wrapComplex(ty);
        auto TY = T->getTypeAtIndex((unsigned)0);
        auto zero = ConstantFP::getZero(TY);
        auto neg_zero = ConstantFP::getNegativeZero(TY);
        return wrap(ty, llvm::ConstantStruct::get(T, {zero, neg_zero}));
    }
    Expr complex_neg_nan_pair(CType ty) {
        assert(ty->isComplex());
        auto T = irgen.wrapComplex(ty);
        auto TY = T->getTypeAtIndex((unsigned)0);
        auto neg_zero = ConstantFP::getNegativeZero(TY);
        auto qnan = ConstantFP::getQNaN(TY, true);
        return wrap(ty, llvm::ConstantStruct::get(T, {neg_zero, qnan}));
    }
    Expr complex_inf_pair(CType ty) {
        assert(ty->isComplex());
        auto T = irgen.wrapComplex(ty);
        auto inf = ConstantFP::getInfinity(T->getTypeAtIndex((unsigned)0));
        return wrap(ty, llvm::ConstantStruct::get(T, {inf, inf}));
    }
    ArenaAllocator &getAllocator() { return context.getAllocator(); }
    CType gettypedef(IdentRef Name) {
        assert(Name && "gettypedef: Name is nullptr");
        auto it = sema.typedefs.getSym(Name);
        if (it && it->ty->hasTag(TYTYPEDEF))
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
    CType gettagByName(IdentRef Name, enum CTypeKind expected, Location full_loc) {
        assert(Name && "gettagByName: Name is nullptr");
        CType result;
        size_t idx;
        auto r = sema.tags.getSym(Name, idx);
        if (r) {
            if (r->ty->getKind() != expected)
                type_error(full_loc, "%s %I is not a %s", show2(r->ty->getKind()), Name, show2(expected));
            result = r->ty;
        } else {
            result = TNEW(IncompleteType){.tag = transform(expected), .name = Name};
            result->setKind(TYINCOMPLETE);
            Type_info info = Type_info{.ty = result, .loc = full_loc};
            insertStmt(SNEW(DeclStmt){.loc = full_loc, .decl_idx = sema.tags.putSym(Name, info), .decl_ty = result});
        }
        return result;
    }
    size_t puttag(IdentRef Name, CType ty, Location loc, enum CTypeKind k) {
        assert(ty && "no type provided");
        bool found = false;
        if (Name) {
            size_t prev;
            auto old = sema.tags.getSymInCurrentScope(Name, prev);
            if (old) {
                if (old->ty->getKind() != TYINCOMPLETE) {
                    type_error(loc, "%s %I redefined", show2(k), Name);
                    note(old->loc, "previous declaration is here");
                } else if (transform(old->ty->tag) != k) {
                    type_error(loc, "%s %I redeclared with different as different kind: %s", show2(k), Name,
                               show(old->ty->tag));
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
                                       .val = ConstantInt::get(irgen.integer_types[5], val),
                                       .tags = ASSIGNED | USED, // enums are used by default ignore warnings
                                   });
    }
    // function
    size_t putsymtype2(IdentRef Name, CType yt, Location full_loc) {
        CType base = yt->ret;
        if (base->getKind() == TYARRAY)
            type_error(full_loc, "function cannot return array");
        if (base->hasTag(TYREGISTER))
            warning(full_loc, "'register' in function return type");
        if (base->hasTag(TYTHREAD_LOCAL))
            warning(full_loc, "'_Thread_local' in function return type");
        if (base->hasTag((TYCONST | TYRESTRICT | TYVOLATILE | TYATOMIC)))
            warning(full_loc, "type qualifiers ignored in function");
        size_t idx;
        auto it = sema.typedefs.getSymInCurrentScope(Name, idx);
        if (it) {
            if (!compatible(yt, it->ty)) {
                type_error(full_loc, "conflicting types for function declaration %I", Name);
            } else if (!it->ty->ret->hasTag(TYSTATIC) && base->hasTag(TYSTATIC)) {
                type_error(full_loc, "static declaration of %I follows non-static declaration", Name);
                note(it->loc, "previous declaration of %I was here", Name);
            } else {
                it->ty->addTag(it->ty->ret->getTags() & (TYSTATIC));
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
        if (yt->getKind() == TYFUNCTION)
            return putsymtype2(Name, yt, full_loc);
        size_t idx;
        auto it = sema.typedefs.getSymInCurrentScope(Name, idx);
        if (it) {
            if (yt->hasTag(TYTYPEDEF)) {
                if (!compatible(it->ty, yt)) {
                    type_error(full_loc, "%I redeclared as different kind of symbol", Name);
                    note(it->loc, "previous declaration of %I is here", Name);
                }
                goto PUT;
            }
            if (isTopLevel() || yt->isGlobalStorage()) {
                auto M = type_qualifiers | TYSTATIC | TYTHREAD_LOCAL | TYREGISTER | TYTYPEDEF;
                CType old = it->ty;
                bool err = true;
                type_tag_t t1 = yt->getTagsQualifiersAndStoragesOnly(), t2 = old->getTagsQualifiersAndStoragesOnly();
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
    bool istype() {
        return is_declaration_specifier(l.tok.tok) ?
            true :
            (l.tok.tok == TIdentifier ? gettypedef(l.tok.s) != nullptr : false);
    }
    Expr make_cast(Expr from, CastOp op, CType to) {
        return ENEW(CastExpr){.loc = from->loc, .ty = to, .castop = op, .castval = from};
    }
    enum Cast_Status: unsigned char {
        Cast_Ok,
        Cast_Imcompatible,
        Cast_DiscardsQualifiers,
        Cast_Sign
    };
    static enum Cast_Status canBeSavelyCastTo(const_CType p, const_CType expected) {
        if (p->getKind() != expected->getKind())
            return Cast_Imcompatible;
        const auto mask = OpaqueCType::important_mask;
        switch (p->getKind()) {
        case TYPRIM:
        {
            if ((p->getTags() & mask) != (expected->getTags() & mask))
                return Cast_Imcompatible;
            bool A = p->isInteger();
            if (A != expected->isInteger()) return Cast_Imcompatible;
            if (A) {
                if (p->getIntegerKind().asLog2() == expected->getIntegerKind().asLog2())
                    return p->isSigned() == expected->isSigned() ? Cast_Ok : Cast_Sign;
                return Cast_Imcompatible;
            }
            return p->getFloatKind().asEnum() == expected->getFloatKind().asEnum() ? Cast_Ok : Cast_Imcompatible;
        }
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
            if (p->p->isVoid() || expected->p->isVoid())
                return Cast_Ok;
            if ((p->p->getTags() & TYCONST) > (expected->p->getTags() & TYCONST))
                return Cast_DiscardsQualifiers;
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
    Expr float_cast2(Expr e, CType to) {
        assert(e && "cast from nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        assert(e->ty->getKind() == TYPRIM && to->getKind() == TYPRIM);
        assert(e->ty->isFloating() && to->isFloating());
        return float_cast(e, to, to->getFloatKind().asEnum(), e->ty->getFloatKind().asEnum());
    }
    Expr float_cast(Expr e, CType to, enum FloatKindEnum k1, enum FloatKindEnum k2) {
        assert(e && "cast from nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        assert(e->ty->getKind() == TYPRIM && to->getKind() == TYPRIM);
        assert(e->ty->isFloating() && to->isFloating());
        // LLVM cannot cast between fp128 and ppc_fp128
        //   => error: invalid cast opcode for cast from 'ppc_fp128' to 'fp128'
        //   => error: invalid cast opcode for cast from 'fp128' to 'ppc_fp128'
        const unsigned 
            rA = k1,
            rB = k2;
        auto k3 = rA > rB ? k1 : k2;
        if (rA > rB)
            return e->k == EConstant ?
                wrap(to, llvm::ConstantExpr::getFPExtend(e->C, irgen.wrapNoComplexScalar(to)), e->loc) :
                make_cast(e, FPExt, to);
        if (rA < rB)
            return e->k == EConstant ?
                wrap(to, llvm::ConstantExpr::getFPTrunc(e->C, irgen.wrapNoComplexScalar(to)), e->loc) :
                make_cast(e, FPTrunc, to);
        if ((k3 >= F_Decimal32 && k3 <= F_Decimal128) || k3 == F_PPC128 || k3 == F_BFloat)
            type_error(e->loc, "unsupported floating fast");
        return e;
    }
    Expr int_cast_promote(Expr e, bool isASigned, CType to) {
        return
            e->k == EConstant ?
                    wrap(to, (e->ty->isSigned() ? &llvm::ConstantExpr::getSExt : &llvm::ConstantExpr::getZExt)(e->C, irgen.wrapNoComplexScalar(to), false), e->loc) :
                    make_cast(e, e->ty->isSigned() ? SExt : ZExt, to);
    }
    Expr int_cast(Expr e, CType to) {
        assert(e && "cast from nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        if (to->isInteger()) {
            const unsigned
                rA = to->getIntegerKind().getBitWidth(),
                rB = e->ty->getIntegerKind().getBitWidth();
            if (rA > rB)
                return
                    e->k == EConstant ?
                        wrap(to, (e->ty->isSigned() ? &llvm::ConstantExpr::getSExt : &llvm::ConstantExpr::getZExt)(e->C, irgen.wrapNoComplexScalar(to), false), e->loc) :
                        make_cast(e, e->ty->isSigned() ? SExt : ZExt, to);
            if (rA < rB)
                return e->k == EConstant ?
                    wrap(to, llvm::ConstantExpr::getTrunc(e->C, irgen.wrapNoComplexScalar(to)), e->loc) :
                    make_cast(e, Trunc, to);
            return bit_cast(e, to);
        }
        return type_error(e->loc, "invalid conversion from %T to %T", e->ty, to), e;
    }
    Expr integer_to_ptr(Expr e, CType to, enum Implict_Conversion_Kind implict) {
        assert(e->ty && "cannot cast from expression with no type");
        assert((e->ty->isInteger()) && "bad call to integer_to_ptr()");
        assert((to->getKind() == TYPOINTER) && "bad call to integer_to_ptr()");
        if (e->k == EConstant) {
            auto CI = cast<ConstantInt>(e->C);
            if (CI->isZero()) // A interger constant expression with the value 0 is a *null pointer constant*
                return wrap(to, null_ptr, e->loc);
            return wrap(to, llvm::ConstantExpr::getIntToPtr(CI, irgen.pointer_type), e->loc);
        }
        if (implict != Implict_Cast) {
            CType arg1 = e->ty;
            CType arg2 = to;
            const char *msg;
            switch (implict) {
            case Implict_Cast: llvm_unreachable("");
            case Implict_Assign:
                std::swap(arg1, arg2);
                msg = "assignment to %T from %T makes pointer from integer without a cast";
                break;
            case Implict_Init:
                std::swap(arg1, arg2);
                msg = "initialization of %T from %T makes pointer from integer without a cast";
                break;
            case Implict_Return:
                msg = "returning %T from a function with result type %T make pointer from integer without a cast";
                break;
            case Implict_Call:
                msg = "passing %T to parameter of type %T makes pointer from integer without a cast";
                break;
            }
            warning(e->loc, msg, arg1, arg2);
        } else if (e->ty->getIntegerKind().asBits() < irgen.pointerSizeInBits) {
            warning(e->loc, "cast to %T from smaller integer type %T", to, e->ty);
        }
        return make_cast(e, IntToPtr, to);
    }
    Expr ptr_to_integer(Expr e, CType to, enum Implict_Conversion_Kind implict) {
        assert(e->ty && "cannot cast from expression with no type");
        assert((e->ty->getKind() == TYPOINTER) && "bad call to ptr_to_integer()");
        assert((to->isInteger()) && "bad call to ptr_to_integer()");
        if (implict != Implict_Cast) {
            CType arg1 = e->ty;
            CType arg2 = to;
            const char *msg;
            switch (implict) {
            case Implict_Cast: llvm_unreachable("");
            case Implict_Assign:
                std::swap(arg1, arg2);
                msg = "assignment to %T from %T makes integer from pointer without a cast";
                break;
            case Implict_Init:
                std::swap(arg1, arg2);
                msg = "initialization of %T from %T makes integer from pointer without a cast";
                break;
            case Implict_Return:
                msg = "returning %T from a function with result type %T make integer from pointer without a cast";
                break;
            case Implict_Call:
                msg = "passing %T to parameter of type %T makes integer from pointer without a cast";
                break;
            }
            warning(e->loc, msg, arg1, arg2);
        } else if (to->getIntegerKind().asBits() < irgen.pointerSizeInBits) {
            warning(e->loc, "cast to smaller integer type %T from %T", to, e->ty);
        }
        if (e->k == EConstant)
            return wrap(to, llvm::ConstantExpr::getPtrToInt(e->C, irgen.wrapNoComplexScalar(to)), e->loc);
        return make_cast(e, PtrToInt, to);
    }
    Expr integer_to_float(Expr e, CType to) {
        assert(e->ty && "cannot cast from expression with no type");
        assert(to->isFloating() && "bad call to integer_to_float()");
        assert((e->ty->isInteger()) && "bad call to integer_to_float()");
        if (e->ty->isSigned()) {
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getSIToFP(e->C, irgen.wrapNoComplexScalar(to)), e->loc);
            return make_cast(e, SIToFP, to);
        }
        if (e->k == EConstant)
            return wrap(to, llvm::ConstantExpr::getUIToFP(e->C, irgen.wrapNoComplexScalar(to)), e->loc);
        return make_cast(e, UIToFP, to);
    }
    Expr float_to_integer(Expr e, CType to) {
        puts("float_to_integer");
        assert(e->ty && "cannot cast from expression with no type");
        assert(e->ty->isFloating() && "bad call to float_to_integer()");
        assert((to->isInteger()) && "bad call to float_to_integer()");
        if (to->isSigned()) {
            if (e->k == EConstant)
                return wrap(to, llvm::ConstantExpr::getFPToSI(e->C, irgen.wrapNoComplexScalar(to)), e->loc);
            return make_cast(e, FPToSI, to);
        }
        if (e->k == EConstant) 
            return wrap(to, llvm::ConstantExpr::getFPToUI(e->C, irgen.wrapNoComplexScalar(to)), e->loc);
        return make_cast(e, FPToUI, to);
    }
    Expr ptr_cast(Expr e, CType to, enum Implict_Conversion_Kind implict = Implict_Cast) {
        assert(e->ty && "cannot cast from expression with no type");
        assert((e->ty->getKind() == TYPOINTER && to->getKind() == TYPOINTER) && "bad call to ptr_cast()");
        if (implict != Implict_Cast) {
            const auto status = canBeSavelyCastTo(e->ty, to);
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
                        msg = "initializing %T with an expression of type %T discards qualifiers";
                    else if (status == Cast_Sign)
                        msg = "initializing %T with an expression of type %T converts between pointers to integer "
                              "types with different sign";
                    else
                        msg = "incompatible pointer types initializing %T with an expression of type %T";
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
#if 0
        llvm::errs() << "# from " << e->ty << " to " << to << '\n';
#endif
        assert(e && "cast object is nullptr");
        assert(e->ty && "cannot cast from expression with no type");
        assert(to && "cast type is nullptr");
        Location loc = getLoc();
        auto k = to->getKind();
        auto k0 = e->ty->getKind();
        if (type_equal(e->ty, to))
            return e;
        if (k0 == TYPOINTER && k == TYPRIM && to->isInteger())
            return ptr_to_integer(e, to, implict);
        if (k0 == TYPRIM && e->ty->isInteger() && k == TYPOINTER)
            return integer_to_ptr(e, to, implict);
        if (k0 == TYPOINTER && k == TYPOINTER)
            return ptr_cast(e, to, implict);
        if (k0 == TYENUM) // cast from enum: bit-cast e to int first, then do-cast to ty
            return castto(bit_cast(e, context.getInt()), to);
        if (k == TYENUM) // cast to enum: do-cast e to int first, then bit-cast to enum
            return bit_cast(castto(e, context.getInt()), to);
        if (k0 != TYPRIM || k != TYPRIM)
            return type_error(loc, "invalid conversion from %T to %T", e->ty, to), e;
        if (e->ty->hasTag(TYVOID))
            return type_error(loc, "cannot convert 'void' expression to type %T", to), e;
        if (to->isBool()) {
            // simplify 'boolean(a) zext to int(b)' to 'a'
            if (e->k == EConstant)
                foldBool(e, false);
            else if (e->k == ECast && e->castop == ZExt && e->castval->ty->isBool())
                return e->castval;
            if (e->k == EConstant)
                if (auto CI = dyn_cast<ConstantInt>(e->C))
                    return getCBool(!CI->isZero());
            return unary(e, ToBool, to);
        }
        // Annex G - IEC 60559-compatible complex arithmetic
        // G.4.2 Real and imaginary
        // 1. When a value of imaginary type is converted to a real type other than bool), the result is a positive zero.
        // 2. When a value of real type is converted to an imaginary type, the result is a positive imaginary zero.
        // G.4.3 Imaginary and complex
        // 1.When a value of imaginary type is converted to a complex type, the real part of the complex result value is a positive zero and the imaginary part of the complex result value is determined by the conversion rules for the corresponding real types.
        // 2 When a value of complex type is converted to an imaginary type, the real part of the complex value is discarded and the value of the imaginary part is converted according to the conversion rules for the corresponding real types.
        if (e->ty->isImaginary()) {
            if (!(to->isComplex() || to->isImaginary()))
                return wrap(to, llvm::Constant::getNullValue(irgen.wrapNoComplexScalar(to)), e->loc);
            CType ty = context.make(e->ty->del(TYIMAGINARY | TYCOMPLEX));
            CType ty2 = context.make(to->del(TYCOMPLEX | TYIMAGINARY));
            Expr e2 = context.clone(e);
            e2->ty = ty;
            Expr res = castto(e2, ty2);
            if (to->isImaginary()) 
                return res;
            return complex_pair(wrap(to, llvm::Constant::getNullValue(irgen.wrapNoComplexScalar(ty2)), e->loc), res, to);
        }
        if (to->isImaginary()) {
            if (e->ty->isComplex()) 
                return bit_cast(complex_get_imag(e), to);
            return wrap(to, llvm::Constant::getNullValue(irgen.wrapNoComplexScalar(to)), e->loc);
        }
        // if e is a complex number
        if (e->ty->isComplex()) {
            Expr lhs = complex_get_real(e);
            Expr rhs = complex_get_imag(e);
            if (e->ty->isFloating()) {
                if (to->isFloating())
                    return (to->isComplex()) ? 
                        complex_pair(float_cast2(lhs, to), float_cast2(rhs, to), to) :
                        float_cast2(lhs, to);
                return (to->isComplex()) ? 
                    complex_pair(float_to_integer(lhs, to), float_to_integer(rhs, to), to) :
                    float_to_integer(lhs, to);
            }
            if (to->isFloating()){
                return to->isComplex() ? complex_pair(integer_to_float(lhs, to), integer_to_float(rhs, to), to) :
                  integer_to_float(lhs, to);
            }
            return to->isComplex() ?
                complex_pair(int_cast(lhs, to), int_cast(rhs, to), to) :
                int_cast(lhs, to);
        }
        // than, e is not a complex number, and cast to complex
        if (to->isComplex()) {
            CType realTy = context.getComplexElementType(to);
            if (to->isFloating())
                return complex_from_real((this->*(e->ty->isFloating() ? &Parser::float_cast2 : &Parser::integer_to_float))(e, realTy), to);
            if (e->ty->isFloating())
                return complex_from_real(float_to_integer(e, realTy), to);
            return complex_from_real(int_cast(e, realTy), to);
        }
        if (e->ty->isFloating() && to->isFloating())
            return float_cast2(e, to);
        if (e->ty->isInteger() && to->isFloating())
            return integer_to_float(e, to);
        if (e->ty->isFloating() && to->isInteger())
            return float_to_integer(e, to);
        return int_cast(e, to);
    }
    bool isTopLevel() { return sema.currentfunction == nullptr; }
    void enterBlock() {
        sema.typedefs.push();
        sema.tags.push();
    }
    void leaveBlock() {
        const auto &T = sema.typedefs;
        if (!getNumErrors()) { // if we have parse errors, we may emit bad warnings about ununsed variables
            for (auto it = T.current_block(); it != T.end(); ++it) {
                if (!(it->info.tags & USED) && !(it->info.ty->hasTag(TYTYPEDEF))) {
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
                if (it.info.ty->hasTag(TYTYPEDEF))
                    continue;
                if (it.info.ty->getKind() == TYFUNCTION) {
                    if (it.info.ty->ret->hasTag(TYSTATIC))
                        warning(it.info.loc, "static function %I' declared but not used", it.sym);
                } else if (it.info.ty->hasTag(TYSTATIC)) {
                    warning(it.info.loc, "static variable %I declared but not used", it.sym);
                }
            }
        }
        sema.typedefs.finalizeGlobalScope();
        sema.tags.finalizeGlobalScope();
    }
    void integer_promotions(Expr &e) {
        // XXX: should _BitInt be converted?
        // Since integer promotions are old thing. 
        if (e->ty->getKind() == TYPRIM && e->ty->isInteger()) {
            if (e->ty->getIntegerKind().asLog2() < 5) {
                e = (e->k == EConstant ?
                        wrap(context.getInt(), (e->ty->isSigned() ? &llvm::ConstantExpr::getSExt : &llvm::ConstantExpr::getZExt)(e->C, irgen.integer_types[4], false), e->loc) :
                        make_cast(e, e->ty->isSigned() ? SExt : ZExt, context.getInt()));
            }
        }
    }
    void complex_conv(Expr &a, Expr &b, CType at, CType bt) {
        if (at->basic_equals(bt))
            return;
        // C A Reference Manual Fifth Edition
        // Complex types and the usual binary conversions
        if (bt->isComplex()) {
            // both operands are complex
            // the shorter operand is converted to the type of the longer
            unsigned rA = scalarRankNoComplex(at);
            unsigned rB = scalarRankNoComplex(bt);
            if (rA > rB)
                b = castto(b, at);
            else
                a = castto(a, bt);
            return;
        }
        // complex float has the highest rank
        if (at->isFloating()) {
            b = castto(b, at);
            return;
        }
        // complex integer + floatings
        // complex int + double => complex double
        if (bt->isFloating()) {
            CType resultTy = context.tryGetComplexTypeFromNonComplex(bt);
            a = castto(a, resultTy);
            b = castto(b, resultTy);
            return;
        }
        // complex integer + integer
        CType resultTy = context.tryGetComplexTypeFromNonComplex(intRank(at) > intRank(bt) ? at : bt);
        a = castto(a, resultTy);
        b = castto(b, resultTy);
        return;
    }
    void conv(Expr &a, Expr &b) {
        CType at = a->ty;
        CType bt = b->ty;
        if (at->getKind() != TYPRIM || bt->getKind() != TYPRIM)
            return;
        if (at->isComplex())
            return complex_conv(a, b, at, bt);
        if (bt->isComplex())
            return complex_conv(b, a, bt, at);
        if (at->isFloating()) {
            // TOTO: Decimal Float
            auto k1 = at->getFloatKind().asEnum();
            auto k2 = bt->getFloatKind().asEnum();
            if (k1 == k2)
                return;
#define HANDLE_FP_CAST(F) \
if (k1 == F) return (void)(b = float_cast(b, at, k1, F)); \
if (k2 == F) return (void)(a = float_cast(a, bt, k2, F));
            HANDLE_FP_CAST(F_Decimal128);
            HANDLE_FP_CAST(F_Decimal64);
            HANDLE_FP_CAST(F_Decimal32);
            HANDLE_FP_CAST(F_PPC128);
            HANDLE_FP_CAST(F_Quadruple);
            HANDLE_FP_CAST(F_Double);
            HANDLE_FP_CAST(F_Float);
            HANDLE_FP_CAST(F_Half);
            HANDLE_FP_CAST(F_BFloat);
            llvm_unreachable("unhandled floating type");
        }
        auto sizeofa = at->getIntegerKind().asLog2();
        auto sizeofb = bt->getIntegerKind().asLog2();
        bool is_a_unsigned = at->isUnsigned();
        bool is_b_unsigned = bt->isUnsigned();
        integer_promotions(a);
        integer_promotions(b);
        if (sizeofa == sizeofb) {
            if (is_a_unsigned == is_b_unsigned)
                return;
            if (is_a_unsigned)
                b = bit_cast(b, at);
            else
                a = bit_cast(a, bt);
            return;
        }
        if (sizeofa > sizeofb) {
            return (void)(b = int_cast_promote(b, is_b_unsigned, at));
        }
        assert(sizeofa < sizeofb);
        a = int_cast_promote(a, is_a_unsigned, bt);
    }
    // clang::Sema::DefaultArgumentPromotion
    void default_argument_promotions(Expr &e) {
        if (e->ty->getKind() != TYPRIM)
            return;
        if (e->ty->isFloating()) {
            // https://clang.llvm.org/docs/LanguageExtensions.html#half-precision-floating-point
            const auto k0 = e->ty->getFloatKind().asEnum();
            switch (k0) {
            case F_Half:
            case F_BFloat:
            case F_Float: return (void)(e = float_cast(e, context.getDobule(), F_Double, k0));
            default: return;
            }
        }
        integer_promotions(e);
    }
    bool checkInteger(CType ty) { return ty->getKind() == TYPRIM && ty->isInteger(); }
    bool checkInteger(Expr e) { return checkInteger(e->ty); }
    void checkInteger(Expr a, Expr b) {
        if (!(checkInteger(a->ty) && checkInteger(b->ty)))
            type_error(getLoc(), "integer types expected");
    }
    static bool checkArithmetic(CType ty) { return ty->getKind() == TYPRIM && (!ty->hasTags(TYVOID)); }
    void checkArithmetic(Expr a, Expr b) {
        if (!(checkArithmetic(a->ty) && checkArithmetic(b->ty)))
            type_error(getLoc(), "arithmetic types expected");
    }
    void checkSpec(Expr &a, Expr &b) {
        if (a->ty->getKind() != b->ty->getKind())
            return (void)type_error(
                getLoc(),
                "bad operands to binary expression:\n    operands type mismatch: %E(has type %T) and %E(has type %T)",
                a, b);
        if (!(a->ty->isScalar() && b->ty->isScalar()))
            return (void)type_error(getLoc(), "scalar types expected");
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
        assert(e->ty->getKind() == TYPOINTER && "expect a pointer");
        if (e->ty->p->hasTag(TYVOID) || (e->k == EUnary && e->uop == AddressOf && e->ty->p->getKind() == TYFUNCTION)) {
            e = context.clone(e);
            e->ty = context.getChar();
        }
    }
    Expr make_add_pointer(Expr rhs, Expr lhs) {
        Expr ptrPart = lhs;
        Expr intPart = rhs;
        if (rhs->ty->getKind() == TYPOINTER) {
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
                        warning(ptrPart->loc, "overflow when doing addition on pointers, the result is %A", &result->cidx);
                    return result;
                }
            }
        }
        return binop(ptrPart, SAddP, intPart, ptrPart->ty);
    }
    void make_add(Expr &result, Expr &r) {
        if (result->ty->getKind() == TYPOINTER || r->ty->getKind() == TYPOINTER) {
            if (result->ty->getKind() == r->ty->getKind())
                return (void)type_error(result->loc, "adding two pointers are not allowed");
            result = make_add_pointer(result, r);
            return;
        }
        if (result->ty->isImaginary() || r->ty->isImaginary()) {
            assert(result->ty->getKind() == TYPRIM);
            assert(r->ty->getKind() == TYPRIM);
            if (result->ty->isImaginary()) {
                // imag + complex => complex
                if (r->ty->isComplex()) {
                    result = castto(result, context.tryGetComplexTypeFromNonComplex(result->ty));
                    make_add(result, r);
                    return;
                }
                Expr e2 = context.clone(result);
                e2->ty = context.getImaginaryElementType(e2->ty);
                // imag + imag => imag
                if (r->ty->isImaginary()) {
                    Expr e3 = context.clone(r);
                    e3->ty = context.getImaginaryElementType(e3->ty);
                    make_add(e2, e3);
                    result = bit_cast(e2, context.make(e2->ty->getTags() | TYIMAGINARY));
                    return;
                }
                // imag + real => complex
                conv(e2, r);
                return (void)(result = complex_pair(e2, r));
            }
            assert(r->ty->isImaginary());
            assert(!result->ty->isImaginary());
            // complex + imag
            if (result->ty->isComplex()) {
                r = castto(r, context.tryGetComplexTypeFromNonComplex(r->ty));
                make_add(result, r);
                return;
            }
            Expr e2 = context.clone(r);
            e2->ty = context.getImaginaryElementType(e2->ty);
            // real + imag => complex
            conv(result, e2);
            return (void)(result = complex_pair(result, e2));
        }
        checkSpec(result, r);
        if (result->ty->isComplex()) {
            if (result->k == EConstant && r->k == EConstant) {
                if (isa<ConstantAggregateZero>(result->C))
                    return (void)(result = r);
                if (isa<ConstantAggregateZero>(r->C))
                    return;
            }
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                        if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                            const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                            const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                            const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                            const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                            APFloat REAL = a;
                            handleOpStatus(REAL.add(c, APFloat::rmNearestTiesToEven));

                            APFloat IMAG = b;
                            handleOpStatus(IMAG.add(d, APFloat::rmNearestTiesToEven));
        
                            result = wrap(r->ty, llvm::ConstantStruct::get(irgen.wrapComplex(r->ty),  ConstantFP::get(irgen.ctx, REAL), ConstantFP::get(irgen.ctx, IMAG)), result->loc);
                            return;
                        }
                    }
                }
                result = binop(result, CFAdd, r, r->ty);
                return;
            }
            if (result->k == EConstant && r->k == EConstant) {
                if (isa<ConstantAggregateZero>(result->C))
                    return (void)(result = r);
                if (isa<ConstantAggregateZero>(r->C))
                    return /* (void)(result = result) */;

                if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                    if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                        const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                        const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                        const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                        const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();
        
                        result = wrap(r->ty, llvm::ConstantStruct::get(
                            irgen.wrapComplex(r->ty), 
                            ConstantInt::get(irgen.ctx, a + c), 
                            ConstantInt::get(irgen.ctx, b + d)),
                            result->loc
                        );
                        return;
                    }
                }
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
        bool p1 = result->ty->getKind() == TYPOINTER;
        bool p2 = r->ty->getKind() == TYPOINTER;
        if (p1 && p2) {
            if (!compatible(result->ty->p, r->ty->p))
                warning(getLoc(), "incompatible type when substract two pointers");
            CType ty = context.getPtrDiff_t();
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
        if (result->ty->isImaginary() || r->ty->isImaginary()) {
            assert(result->ty->getKind() == TYPRIM);
            assert(r->ty->getKind() == TYPRIM);
            if (result->ty->isImaginary()) {
                // imag - complex => complex
                if (r->ty->isComplex()) {
                    result = castto(result, context.tryGetComplexTypeFromNonComplex(result->ty));
                    make_sub(result, r);
                    return;
                }
                Expr e2 = context.clone(result);
                e2->ty = context.getImaginaryElementType(e2->ty);
                // imag - imag => imag
                if (r->ty->isImaginary()) {
                    Expr e3 = context.clone(r);
                    e3->ty = context.getImaginaryElementType(e3->ty);
                    make_sub(e2, e3);
                    result = bit_cast(e2, context.tryGetImaginaryTypeFromNonImaginary(e2->ty));
                    return;
                }
                // imag - real => complex
                // e.g., (3i) - (4) => -4 + 3i
                conv(e2, r);
                if (e2->k == EConstant) {
                    return (void)(result = complex_pair(wrap(e2->ty, llvm::ConstantExpr::getNeg(e2->C), e2->loc), r));
                } else {
                    return (void)(result = complex_pair(unary(e2, e2->ty->isFloating() ? FNeg : UNeg, e2->ty), r));
                }
            }
            assert(r->ty->isImaginary());
            assert(!result->ty->isImaginary());
            // complex - imag
            if (result->ty->isComplex()) {
                r = castto(r, context.tryGetComplexTypeFromNonComplex(r->ty));
                make_sub(result, r);
                return;
            }
            Expr e2 = context.clone(r);
            e2->ty = context.getImaginaryElementType(e2->ty);
            // real - imag => complex
            // e.g, 4 - 3i
            conv(result, e2);
            if (e2->k == EConstant) {
                e2 = wrap(e2->ty, llvm::ConstantExpr::getNeg(e2->C), e2->loc);
            } else {
                e2 = unary(e2, e2->ty->isFloating() ? FNeg : UNeg, e2->ty);
            }
            return (void)(result = complex_pair(result, e2));
        }
        checkSpec(result, r);
        if (result->ty->isComplex()) {
            if (result->k == EConstant && r->k == EConstant) {
                if (isa<ConstantAggregateZero>(result->C)) {
                    if (isa<ConstantAggregateZero>(r->C))
                        // zero - zero => zero
                        return (void)(result = complex_zero(result->ty));
                    const auto CS = cast<ConstantStruct>(r->C);
                    if (auto CI = dyn_cast<ConstantInt>(CS->getOperand(0))) {
                        auto CI2 = cast<ConstantInt>(CS->getOperand(1));
                        return (void)(result = wrap(r->ty, llvm::ConstantStruct::get(irgen.wrapComplexForInteger(r->ty), {ConstantInt::get(irgen.ctx, -CI->getValue()), ConstantInt::get(irgen.ctx, -CI2->getValue())}), result->loc));
                    }
                    auto CF = cast<ConstantFP>(CS->getOperand(0));
                    auto CF2 = cast<ConstantFP>(CS->getOperand(1));
                    return (void)(result = wrap(r->ty, llvm::ConstantStruct::get(irgen.wrapComplex(r->ty), {ConstantFP::get(irgen.ctx, -CF->getValue()), ConstantFP::get(irgen.ctx, -CF2->getValue())}), result->loc));
                }
                if (isa<ConstantAggregateZero>(r->C))
                    return /* (void)(result = result) */;
            }
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                        if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                            const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                            const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                            const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                            const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                            APFloat REAL = a;
                            handleOpStatus(REAL.subtract(c, APFloat::rmNearestTiesToEven));
        
                            APFloat IMAG = b;
                            handleOpStatus(IMAG.subtract(d, APFloat::rmNearestTiesToEven));
        
                            result = wrap(r->ty, llvm::ConstantStruct::get(irgen.wrapComplex(r->ty), {ConstantFP::get(irgen.ctx, REAL), ConstantFP::get(irgen.ctx, IMAG)}), result->loc);
                            return;
                        }
                    }

                }
                result = binop(result, CFSub, r, r->ty);
                return;
            }
            if (result->k == EConstant && r->k == EConstant) {
                if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                    if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                        const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                        const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                        const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                        const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();
        
                        result = wrap(r->ty, llvm::ConstantStruct::get(
                            irgen.wrapComplex(r->ty), 
                            ConstantInt::get(irgen.ctx, a - c), 
                            ConstantInt::get(irgen.ctx, b - d)),
                            result->loc
                        );
                        return;
                    }
                }
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
            auto width = result->ty->getIntegerKind().getBitWidth();
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
            auto width = result->ty->getIntegerKind().getBitWidth();
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
    result = wrap(result->ty, ConstantInt::get(irgen.ctx, result->ty->getIntegerKind().getZero()), result->loc);
    return;
}
ONE : {
    const StringRef allOnes = "all ones(all bits set, -1)";
    if (op == Or) {
        warning(result->loc, "'bitsize or' to %R is always all ones", allOnes);
        result = result =
            wrap(result->ty, ConstantInt::get(irgen.ctx, APInt::getAllOnes(result->ty->getIntegerKind().getBitWidth())), result->loc);
        return;
    }
    warning(result->loc, "'bitsize and' to %R is always itself", allOnes);
}
    }
    void make_mul(Expr &result, Expr &r) {
        checkArithmetic(result, r);
        if (result->ty->isImaginary() || r->ty->isImaginary()) {
            assert(result->ty->getKind() == TYPRIM);
            assert(r->ty->getKind() == TYPRIM);
            if (result->ty->isImaginary()) {
                // imag * complex => complex
                if (r->ty->isComplex()) {
                    result = castto(result, context.tryGetComplexTypeFromNonComplex(result->ty));
                    make_mul(result, r);
                    return;
                }
                Expr e2 = context.clone(result);
                e2->ty = context.getImaginaryElementType(result->ty);
                // imag * imag => real
                if (r->ty->isImaginary()) {
                    Expr e3 = context.clone(r);
                    e3->ty = context.getImaginaryElementType(e3->ty);
                    make_mul(e2, e3);
                    if (e2->k == EConstant)
                        result = wrap(e2->ty, llvm::ConstantExpr::getNeg(e2->C), e2->loc);
                     else 
                        result = unary(e2, e2->ty->isFloating() ? FNeg : UNeg , e2->ty);
                    return;
                }
                // imag * real => imag
                result = bit_cast(r, context.getImaginaryElementType(r->ty));
                make_mul(result, r);
                result->ty->addTag(TYIMAGINARY);
                return;
            }
            assert(r->ty->isImaginary());
            assert(!result->ty->isImaginary());
            // complex * imag => complex
            if (result->ty->isComplex()) {
                r = castto(r, context.tryGetComplexTypeFromNonComplex(r->ty));
                make_mul(result, r);
                return;
            }
            // real * imag => imag
            Expr tmp = bit_cast(r, context.getImaginaryElementType(r->ty));
            make_mul(result, tmp);
            return;
        }
        conv(result, r);
        if (result->ty->isComplex()) {
            if (result->k == EConstant && r->k == EConstant) {
                if (isa<ConstantAggregateZero>(result->C))
                    return (void)(result = complex_zero(r->ty));
                if (isa<ConstantAggregateZero>(r->C))
                    return (void)(result = complex_zero(r->ty));
            }
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                        if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                            const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                            const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                            const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                            const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                            result = wrap(r->ty, llvm::ConstantStruct::get(
                                irgen.wrapComplex(r->ty),  
                                ConstantFP::get(irgen.ctx, a * c - b * d), 
                                ConstantFP::get(irgen.ctx, b * c + a * d)), 
                                result->loc
                            );
                            return;
                        }
                    }
                }
                result = binop(result, CFMul, r, r->ty);
                return;
            }
            if (result->k == EConstant && r->k == EConstant) {
                if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                    if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                        const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                        const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                        const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                        const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();
        
                        result = wrap(r->ty, llvm::ConstantStruct::get(
                            irgen.wrapComplex(r->ty), 
                            ConstantInt::get(irgen.ctx, a * c - b * d), 
                            ConstantInt::get(irgen.ctx, b * c + a * d)),
                            result->loc
                        );
                        return;
                    }
                }
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
        if (result->ty->isImaginary() || r->ty->isImaginary()) {
            assert(result->ty->getKind() == TYPRIM);
            assert(r->ty->getKind() == TYPRIM);
            if (result->ty->isImaginary()) {
                // imag / complex => complex
                if (r->ty->isComplex()) {
                    result = castto(result, context.tryGetComplexTypeFromNonComplex(result->ty));
                    make_div(result, r);
                    return;
                }
                Expr e2 = context.clone(result);
                e2->ty = context.getImaginaryElementType(result->ty);
                // imag / imag => real
                if (r->ty->isImaginary()) {
                    Expr e3 = context.clone(r);
                    e3->ty = context.getImaginaryElementType(e3->ty);
                    make_div(e2, e3);
                    result = e2;
                    return;
                }
                // imag / real => imag
                result = bit_cast(r, context.getImaginaryElementType(r->ty));
                make_div(result, r);
                result->ty->addTag(TYIMAGINARY);
                return;
            }
            assert(r->ty->isImaginary());
            assert(!result->ty->isImaginary());
            // complex / imag => complex
            if (result->ty->isComplex()) {
                r = castto(r, context.tryGetComplexTypeFromNonComplex(r->ty));
                make_div(result, r);
                return;
            }
            // real / imag => imag
            Expr tmp = bit_cast(r, context.getImaginaryElementType(r->ty));
            make_div(result, tmp);
            return;
        }
        conv(result, r);
        if (result->ty->isComplex()) {
            // if the first operand is a nonzero finite number or an infinity and the second operand is a zero then the result of the / operator is an infinity.
            if (result->k == EConstant && r->k == EConstant) {
                if (isa<ConstantAggregateZero>(result->C)) {
                    if (isa<ConstantAggregateZero>(r->C)) {
                        if (r->ty->isFloating()) 
                            return (void)(result = complex_neg_nan_pair(result->ty));
                        goto CINT_ZERO;
                    }
                    goto NEXT_NEXT;
                }
                if (isa<ConstantAggregateZero>(r->C)) {
                    if (r->ty->isFloating())
                        return (void)(result = complex_inf_pair(r->ty));
                    goto CINT_ZERO;
                }
                goto NEXT_NEXT;
                CINT_ZERO:
                warning(result->loc, "complx int division by zero, the result is zero");
                result = complex_zero(r->ty);
                return;
            }
            NEXT_NEXT:
            if (result->ty->isFloating()) {
                if (result->k == EConstant && r->k == EConstant) {
                    if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                        if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                        const auto &a = cast<ConstantFP>(X->getOperand(0))->getValue();
                        const auto &b = cast<ConstantFP>(X->getOperand(1))->getValue();
                        const auto &c = cast<ConstantFP>(Y->getOperand(0))->getValue();
                        const auto &d = cast<ConstantFP>(Y->getOperand(1))->getValue();
                        APFloat tmp = c * c + d * d;
                        result = wrap(r->ty, llvm::ConstantStruct::get(
                            irgen.wrapComplex(r->ty),  
                            ConstantFP::get(irgen.ctx, (a * c + b * d) / tmp), 
                            ConstantFP::get(irgen.ctx, (b * c - a * d) / tmp)
                            ), 
                            result->loc
                        );
                        return;
                        }
                    }
                }
                result = binop(result, CFDiv, r, r->ty);
                return;
            }
            const bool isSigned = r->ty->isSigned();
            if (result->k == EConstant && r->k == EConstant) {
                if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                    if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
                        const auto &a = cast<ConstantInt>(X->getOperand(0))->getValue();
                        const auto &b = cast<ConstantInt>(X->getOperand(1))->getValue();
                        const auto &c = cast<ConstantInt>(Y->getOperand(0))->getValue();
                        const auto &d = cast<ConstantInt>(Y->getOperand(1))->getValue();
                        APInt tmp0 = c * c + d * d;
                        APInt tmp1 = a * c + b * d;
                        APInt tmp2 = b * c - a * d;
                        result = wrap(r->ty, llvm::ConstantStruct::get(
                            irgen.wrapComplex(r->ty), 
                            ConstantInt::get(irgen.ctx, isSigned ? tmp1.sdiv(tmp0) : tmp1.udiv(tmp0)), 
                            ConstantInt::get(irgen.ctx, isSigned ? tmp2.sdiv(tmp0) : tmp2.udiv(tmp0))),
                            result->loc
                        );
                        return;
                    }
                }
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
        if ((result->ty->isComplex()) && !isEq)
            return (void)type_error(result->loc, "complex numbers unsupported in relational-expression");
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
            if (result->ty->isComplex()) {
                if (isa<ConstantAggregateZero>(result->C)) {
                    if (isa<ConstantAggregateZero>(r->C))
                        return (void)(result = getBool(tok == TEq)); // equal!
                    return (void)(result = getBool(tok != TEq)); // not equal!
                }
                if (isa<ConstantAggregateZero>(r->C)) // not equal!
                    return (void)(result = getBool(tok != TEq));
                if (auto X = dyn_cast<llvm::ConstantStruct>(result->C)) {
                    if (auto Y = dyn_cast<llvm::ConstantStruct>(r->C)) {
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
            }
            
        }
        if (result->ty->isComplex())
            result = boolToInt(binop(result, tok == TEq ? CEQ : CNE, r, context.getBool()));
        else
            result =
                boolToInt(binop(result, get_relational_expression_op(tok, result->ty->isFloating(), result->ty->isSigned2()),
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
        // make preprocessor identfiers to normal identfier
        if (l.tok.tok > TIdentifier)
            l.tok.tok = TIdentifier;
    }
    SourceMgr &SM() { return l.SM; }
    Location getLoc() { return l.getLoc(); }
    void checkSemicolon() {
        if (l.tok.tok != TSemicolon)
            return (void)warning(getLoc(), "missing ';'");
        consume();
    }
    void type_qualifier_list(CType &ty) {
        for (;;)
            switch (l.tok.tok) {
            case Kconst: ty->addTag(TYCONST), consume(); continue;
            case Krestrict: ty->addTag(TYRESTRICT), consume(); continue;
            case Kvolatile: ty->addTag(TYVOLATILE), consume(); continue;
            case K_Atomic: ty->addTag(TYATOMIC), consume(); continue;
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
            type_error(getLoc(), "at most one storage-class specifier may be given in the declaration specifiers in a declaration");
    }
    CType specifier_qualifier_list() {
        Location loc = getLoc();
        CType eat_typedef = nullptr;
        bool no_typedef = false;
        type_tag_t tags = 0;
        unsigned b = 0, L = 0, s = 0, f = 0, d = 0, i = 0, c = 0, v = 0, numsigned = 0, numunsigned = 0, numcomplex = 0, numimaginary = 0;
        type_tag_t old_tag;
        const Token firstTok = l.tok.tok;
        unsigned count = 0;
        for (;;++count) {
            const Token theTok = l.tok.tok;
            old_tag = tags;
            switch (theTok) {
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
            case Ksigned: ++numsigned; goto TYPE_SPEC;
            case Kunsigned: ++numunsigned; goto TYPE_SPEC;
            case Klong: ++L; goto TYPE_SPEC;
            case Kshort: ++s; goto TYPE_SPEC;
            case Kint: ++i; goto TYPE_SPEC;
            case Kdouble: ++d; goto TYPE_SPEC;
            case Kfloat: ++f; goto TYPE_SPEC;
            case Kvoid: ++v; goto TYPE_SPEC;
            case Kchar: ++c; goto TYPE_SPEC;
            case K_Complex: ++numcomplex; goto TYPE_SPEC;
            case K_Imaginary: ++numimaginary; goto TYPE_SPEC;
            case K_Bool: ++b; goto TYPE_SPEC;
TYPE_SPEC:
            no_typedef = true;
            break;
            case Ktypeof:
            case Ktypeof_unqual:
                if (no_typedef)
                    goto BREAK;
                no_typedef = true;
                consume();
                if (l.tok.tok != TLbracket) {
                    parse_error(loc, "expect '(' after typedef");
                    return context.getInt();
                }
                consume();
                {
                    if (istype()) {
                        CType ty = type_name();
                        if (!ty) return context.getInt();
                        eat_typedef = ty;
                    } else {
                        Expr e = expression();
                        if (!e) return context.getInt();
                        eat_typedef = e->ty;
                    }
                    if (theTok == Ktypeof_unqual) {
                        eat_typedef = context.clone(eat_typedef);
                        eat_typedef->clearQualifiers();
                    }
                    if (l.tok.tok != TRbracket)
                        parse_error(loc, "missing ')'");
                    break;
                }
            case TIdentifier:
                if (no_typedef) // we cannot eat double typedef
                    goto BREAK;
                no_typedef = true;
                eat_typedef = gettypedef(l.tok.s);
                if (!eat_typedef)
                    goto BREAK;
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
                intTy->addTags(tags);
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
                warning(loc, "plain '_Complex' requires a type specifier; assuming '_Complex double'");
                return context.getComplexDouble();
            case K_Imaginary:
                warning(loc, "plain '_Imaginary' requires a type specifier; assuming '_Imaginary double'");
                return context.getImaginaryDouble();
            default: llvm_unreachable("unhandled type in specifier_qualifier_list()");
            }
        }
        verify_one_storage_class(tags);
        FloatKind F = F_Invalid;
        IntegerKind I = 0;
        bool isa_integer = i || numunsigned || numsigned || c || b;
        bool ias_float = d || f;
        if (isa_integer && ias_float)
            type_error(loc, "both integer and float type specifiers");
        else if (v && (isa_integer || ias_float))
            type_error(loc, "'void' never combine with other type specifiers");
        else if ((numunsigned && numsigned) || numsigned > 1 || numunsigned > 1) {
            type_error(loc, "duplicate 'signed/unsigned'");
            return context.getInt();
        }
        if (numcomplex || numimaginary) {
            if (numcomplex > 1)
                warning(loc, "duplicate '_Complex'");
            if (numimaginary > 1)
                warning(loc, "duplicate _Imaginary");
            if (numcomplex && numimaginary) {
                type_error(loc, "both '_Complex' and '_Imaginary' is invalid");
                return context.getComplexDouble();
            }
            if (b || v) {
                type_error("'%s %s' is invalid", numcomplex ? "_Complex" : "_Imaginary", b ? "_Bool" : "void");
                return context.getComplexDouble();
            }
            if (numcomplex)
                tags |= TYCOMPLEX;
            else
                tags |= TYIMAGINARY;
        }
        if (f) {
            if (f > 1)
                warning(loc, "duplicate 'float'");
            F = F_Float;
        } else if (d) {
            if (d > 1)
                warning(loc, "duplicate 'double'");
            if (L > 1)
                warning(loc, "too many 'long's for 'double'");
            F = F_Double;
        } else if (s) {
            if (s > 1)
                warning(loc, "duplicate 'short'");
            I = context.getShortLog2();
        } else if (L) {
            switch (L) {
            case 1: I = context.getLongLog2(); break;
            case 2: I = context.getLongLongLog2(); break;
            case 3: type_error(loc, "'long long long' is too long for XCC"); break;
            default: warning(loc, "too many 'long'");
            }
        } else if (c) {
            if (c > 1)
                warning(loc, "duplicate 'char'");
            I = context.getCharLog2();
        } else if (i) {
            if (i > 1)
                warning(loc, "duplicate 'int'");
            I = context.getIntLog2();
        } else if (v) {
            if (v > 1)
                warning(loc, "duplicate 'void'");
            tags |= TYVOID;
        } else if (numunsigned || numsigned) {
            I = context.getIntLog2();
        } else if (b) { 
            I = context.getBoolLog2();
        } else {
            if (!eat_typedef) {
                warning(loc, "type-specifier missing, defaults to 'int'");
                if (tags) 
                    return context.make_signed(context.getIntLog2(), tags);
                return context.getInt();
            }
        }
        if (!eat_typedef) {
            if (F) {
                tags |= build_float(F);
            } else {
                if (!numunsigned && I)
                    tags |= OpaqueCType::sign_bit;
                tags |= build_integer(I);
            }
            return context.make(tags);
        }
        // try to merge typedef with type qualifiers and storage class specifers
        // If tags has 'void', '_Complex', '_Imaginary', or we have meet int(I) or float(F), than we cannot merge.
        if (I || F || tags & (TYVOID | TYIMAGINARY | TYCOMPLEX)) {
            type_error(loc, "cannot mix typedef(%T) with other type-specifiers", eat_typedef);
            tags &= type_qualifiers_and_storage_class_specifiers;
        }
        eat_typedef = context.clone(eat_typedef);
        eat_typedef->clearTag(TYTYPEDEF);
        eat_typedef->addTags(tags);
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
            CType ty = sema.currentInitTy;
            if (!ty)
                return assignment_expression();
            auto k = ty->getKind();
            if (k == TYARRAY && l.tok.tok == TStringLit) {
                xstring s = l.tok.str;
                auto enc = l.tok.getStringPrefix();
                Location loc1 = getLoc();
                consume();
                while (l.tok.tok == TStringLit) {
                    auto enc2 = l.tok.getStringPrefix();
                    s.push_back(l.tok.str);
                    if (enc2 != enc)
                        type_error(getLoc(), "unsupported non-standard concatenation of string literals");
                    consume();
                }
                s.make_eos();
                switch (enc) {
                    case Prefix_none:
                    case Prefix_u8:
                        if (!(ty->arrtype->isInteger() && ty->arrtype->getIntegerKind().asLog2() == 3))
                            type_error(loc1, "initializing %T array with string literal", ty->arrtype);
                        return wrap(context.getFixArrayType(enc == Prefix_none ? context.getChar() : context.getChar8_t(), s.size() + 1), enc::getUTF8(s, irgen.ctx), loc1);
                    case Prefix_L:
                        if (!(ty->arrtype->isInteger() && ty->arrtype->getIntegerKind().asLog2() == 5))
                            type_error(loc1, "initializing %T array with wide string literal", ty->arrtype);
                        return wrap(context.getFixArrayType(context.getWChar(), s.size() + 1), context.getWCharLog2() == 5 ? enc::getUTF16As32Bit(s, irgen.ctx) : enc::getUTF16As16Bit(s, irgen.ctx), loc1);
                    case Prefix_u:
                        if (!(ty->arrtype->isInteger() && ty->arrtype->getIntegerKind().asLog2() == 5))
                            type_error(loc1, "initializing %T array with UTF-16 string literal", ty->arrtype);
                        return wrap(context.getFixArrayType(context.getChar16_t(), s.size() + 1), enc::getUTF16As16Bit(s, irgen.ctx), loc1);
                    case Prefix_U:
                        if (!(ty->arrtype->isInteger() && ty->arrtype->getIntegerKind().asLog2() == 5))
                            type_error(loc1, "initializing %T array with UTF-32 string literal", ty->arrtype);
                        return wrap(context.getFixArrayType(context.getUChar(), s.size() + 1), enc::getUTF32(s, irgen.ctx), loc1);
                    default: llvm_unreachable("bad string encoding");
                }
            }
            if (k != TYPRIM && k != TYPOINTER && k != TYENUM)
                return type_error(getLoc(), "expect bracket initializer"), nullptr;
            if (!(e = assignment_expression()))
                return nullptr;
            return castto(e, ty, Implict_Init);
        }
        auto k = sema.currentInitTy->getKind();
         result = k == TYSTRUCT
                     ? ENEW(StructExpr){.ty = sema.currentInitTy, .arr2 = xvector<Expr>::get()}
                     : ENEW(ArrayExpr){.ty = sema.currentInitTy, .arr = xvector<Expr>::get()};
        result->loc = getLoc();
        consume();
        if (k == TYARRAY) {
            if (sema.currentInitTy->hassize) {
                m = sema.currentInitTy->arrsize;
            } else {
                result->ty->hassize = true, result->ty->arrsize = 0, m = -1;
            }
        } else if (k == TYSTRUCT) {
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
            if (k == TYSTRUCT) {
                ty = i < (unsigned)m ? sema.currentInitTy->selems[i].ty : nullptr;
            } else if (k == TYARRAY) {
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
            Declator st, st2;
            CType dummy = context.createDummyType();
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
            memcpy(reinterpret_cast<void *>(dummy), reinterpret_cast<const void *>(st2.ty), ctype_size_map[st2.ty->getKind()]);
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
            ty->setKind(TYARRAY);
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
                if ((!e->ty->getKind()) == TYPRIM && e->ty->isInteger())
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
            ty->setKind(TYFUNCTION);
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
            Declator res = Declator(nullptr, TNEW(BitfieldType){.bittype = base, .bitsize = bitsize});
            res.ty->setKind(TYBITFIELD);
            return res;
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
            Declator res = Declator(nullptr, TNEW(BitfieldType){.bittype = base, .bitsize = bitsize});
            res.ty->setKind(TYBITFIELD);
            return res;
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
        result->setKind(tok == Kstruct ? TYSTRUCT : TYUNION);
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
        result->setKind(TYENUM);
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
            if (!istype() && l.tok.tok != TIdentifier) // TODO: old style
                return type_error(getLoc(), "expect a type or old-style variable name"), false;
            CType base = declaration_specifiers();
            if (!base)
                return expect(getLoc(), "declaration-specifiers"), false;
            Location full_loc = getLoc();
            Declator nt = declarator(base, Function);
            if (!nt.ty)
                return expect(full_loc, "abstract-declarator"), false;
            params.push_back(nt);
            if (nt.ty->getKind() == TYINCOMPLETE)
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
            if (i == 0 && (params[0].ty->hasTag(TYVOID)))
                zero = true;
            if (params[i].ty->getKind() == TYARRAY)
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
    bool parse_alignas() {
        consume();
        if (l.tok.tok != TLbracket)
            return expectLB(getLoc()), false;
        consume();
        if (istype()) {
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
            if (var_info.ty->hasTag(TYCONST)) {
                type_error(getLoc(), "cannot modify const-qualified variable %R: %T", sema.typedefs.getSymName(e->sval),
                           var_info.ty);
                return false;
            }
            return true;
        }
        if (e->ty->getKind() == TYPOINTER) {
            if ((e->ty->hasTag(TYLVALUE)) && e->ty->p->getKind() == TYARRAY)
                return type_error(getLoc(), "array is not assignable"), false;
            return true;
        }
        if (e->ty->hasTag(TYLVALUE))
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
                base->setAlignInBytes(sema.currentAlign);
            }
        }
        if (l.tok.tok == TSemicolon) {
            consume();
            if (base->getAlignLog2Value())
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
                if (st.ty->getKind() != TYFUNCTION)
                    return (void)type_error(full_loc, "unexpected function definition");
                // function definition
#if 0
                print_cdecl(st.name->getKey(), st.ty, llvm::errs(), true);
#endif
                if (st.name->second.getToken() == PP_main) {
                    auto Size = st.ty->params.size();
                    if (Size) {
                        if (Size != 2) 
                            warning(full_loc, "'main' takes only zero or two arguments");
                        CType argcTy = st.ty->params.front().ty;
                        if (!(argcTy->getKind() == TYPRIM && argcTy->isInteger() && argcTy->getIntegerKind().asLog2() == context.getIntLog2()))
                            warning(full_loc, "first argument of 'main' should be 'int'");                        
                        if (Size >= 2) {
                            const auto argvTy = st.ty->params[1].ty;
                            if (argvTy->getKind() == TYPOINTER) {
                                const auto pArgvTy = argvTy->p;
                                if (pArgvTy->getKind() == TYPOINTER) {
                                    const auto CharTy = pArgvTy->p;
                                    if (CharTy->getKind() == TYPRIM && CharTy->isInteger() && CharTy->getIntegerKind().asLog2() == context.getCharLog2()) {
                                        goto ARGV_OK;
                                    }
                                }
                            }
                            warning(full_loc, "second argument of 'main' should be 'char **'");
                            ARGV_OK: ;
                        }
                    }
                }
                size_t idx = putsymtype2(st.name, st.ty, full_loc);
                if (!isTopLevel())
                    return (void)parse_error(full_loc, "function definition is not allowed here");
                Stmt res = SNEW(FunctionStmt){.loc = full_loc,
                                              .func_idx = idx,
                                              .funcname = st.name,
                                              .functy = st.ty,
                                              .args = xvector<size_t>::get_with_length(st.ty->params.size())};
                sema.currentfunction = st.ty;
                sema.pfunc = st.name;
                res->funcbody = function_body(st.ty->params, res->args, loc);
                sema.pfunc = nullptr;
                sema.currentfunction = nullptr;
                res->numLabels = jumper.cur;
                jumper.cur = 0;
                sreachable = true;
                for (auto it = sema.typedefs.begin(); it != sema.typedefs.end(); ++it)
                    if (!(it->info.ty->hasTag(TYCONST)))
                        it->info.val = nullptr;
                return insertStmt(res);
            }
            st.ty->noralize();
#if 1
            print_cdecl(st.name->getKey(), st.ty, llvm::errs(), true);
#endif
            size_t idx = putsymtype(st.name, st.ty, full_loc);

            result->vars.push_back(VarDecl{.name = st.name, .ty = st.ty, .init = nullptr, .idx = idx});
            if (st.ty->hasTag(TYINLINE))
                warning(full_loc, "inline can only used in function declaration");
            if (!(st.ty->hasTag(TYEXTERN))) {
                if ((st.ty->isVoid()) && !(st.ty->hasTag(TYTYPEDEF)))
                    return (void)type_error(full_loc, "variable %I declared void", st.name);
                if (st.ty->getKind() == TYINCOMPLETE)
                    return (void)type_error(full_loc, "variable %I has imcomplete type %T", st.name);
            }
            if (l.tok.tok == TAssign) {
                Expr init;
                auto &var_info = sema.typedefs.getSym(idx);
                var_info.tags |= ASSIGNED;
                if (st.ty->getKind() == TYARRAY && st.ty->vla) {
                    return (void)type_error(full_loc, "variable length array may not be initialized");
                } else if (st.ty->getKind() == TYFUNCTION)
                    return (void)type_error(full_loc, "function may not be initialized");
                if (st.ty->hasTag(TYTYPEDEF))
                    return (void)type_error(full_loc, "'typedef' may not be initialized");
                consume();
                {
                    llvm::SaveAndRestore<CType> saved_ctype(sema.currentInitTy, st.ty);
                    init = initializer_list();
                }
                if (!init)
                    return expect(full_loc, "initializer-list");
                result->vars.back().init = init;
                if (st.ty->getKind() == TYARRAY && !st.ty->hassize && !st.ty->vla) 
                    result->vars.back().ty = init->ty;
                if (init->k == EConstant && st.ty->hasTags(TYCONST | TYCONSTEXPR)) {
                    var_info.val = init->C; // for const and constexpr, their value can be fold to constant
                    var_info.tags |= CONST_VAR;
                }
                if ((isTopLevel() || st.ty->isGlobalStorage()) && init->k != EConstant && init->k != EConstantArray &&
                    init->k != EConstantArraySubstript) {
                    // take address of globals is constant!
                    if (!
                        (init->k == EVar && 
                            (
                                sema.typedefs.isInGlobalScope(init->sval) || 
                                sema.typedefs.getSym(init->sval).ty->hasTag(TYEXTERN | TYSTATIC)
                            )
                        )
                        ) {
                        type_error(full_loc, "global initializer is not constant");
                    }
                }
            } else {
                if (st.ty->getKind() == TYARRAY) {
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
            if (istype()) {
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
                if (ty->hasTag(TYVOID))
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
            if (e->ty->getKind() != TYPOINTER)
                return type_error(loc, "pointer expected"), nullptr;
            make_deref(e, loc);
            return e;
        }
        case TBitNot: {
            Expr e;
            consume();
            if (!(e = cast_expression()))
                return nullptr;
            if (!(checkInteger(e->ty) || (e->ty->isComplex())))
                return type_error(loc, "integer type expected"), nullptr;
            integer_promotions(e);
            if (e->ty->isComplex()) {
                if (e->k == EConstant) {
                    if (auto CS = dyn_cast<llvm::ConstantStruct>(e->C)) {
                        if (auto REAL = dyn_cast<ConstantFP>(CS->getOperand(0))) {
                            const auto &i = cast<ConstantFP>(CS->getOperand(1))->getValue();
                            auto IMAG = ConstantFP::get(irgen.ctx, -i);
                            return wrap(e->ty, llvm::ConstantStruct::get(irgen.wrapComplex(e->ty), {REAL, IMAG}), e->loc);
                        }
                        const auto REAL = cast<ConstantInt>(CS->getOperand(0));
                        const auto &i = cast<ConstantInt>(CS->getOperand(1))->getValue();
                        auto IMAG = ConstantInt::get(irgen.ctx, -i);
                        return wrap(e->ty, llvm::ConstantStruct::get(irgen.wrapComplexForInteger(e->ty), {REAL, IMAG}), e->loc);
                    }
                    if (isa<ConstantAggregateZero>(e->C)) {
                        return e->ty->isFloating() ? complex_pos_neg_zero(e->ty) : complex_zero(e->ty);
                    }
                }
                return unary(e, CConj, e->ty);
            }
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
                assert(e->ty->getKind() == TYPOINTER);
                if (auto CA = dyn_cast<llvm::ConstantDataArray>(e->array)) {
                    e->ty = context.getPointerType(context.getFixArrayType(
                    e->ty->p, CA->getType()->getNumElements()));
                } else {
                    auto AZ = cast<ConstantAggregateZero>(e->C);
                    e->ty = context.getPointerType(
                        context.getFixArrayType(e->ty->p, cast<llvm::ArrayType>(AZ->getType())->getNumElements())
                    );
                }
                return e;
            }
            if (e->k == EArrToAddress)
                return e->ty = context.getPointerType(e->arr3->ty), e;
            if (e->ty->getKind() == TYBITFIELD)
                return type_error(loc, "cannot take address of bit-field"), e;
            if (e->ty->hasTag(TYREGISTER))
                return type_error(loc, "take address of register variable"), e;
            if (e->k == EUnary && e->uop == AddressOf && e->ty->p->getKind() == TYFUNCTION) {
                e->ty = context.clone(e->ty);
                e->ty->clearTag(TYLVALUE);
                return e;
            }
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
                if (e->ty->isComplex()) {
                    if (isa<ConstantAggregateZero>(e->C)) 
                        return e->ty->isFloating() ? complex_neg_zero(e->ty) : complex_zero(e->ty);
                    if (auto CS = dyn_cast<llvm::ConstantStruct>(e->C)) {
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
                } else {
                    return wrap(e->ty,
                            e->ty->isSigned() ? llvm::ConstantExpr::getNSWNeg(e->C) : llvm::ConstantExpr::getNeg(e->C),
                            e->loc);
                }
            }
            return unary(e, (e->ty->isComplex()) ? CNeg : (e->ty->isSigned() ? SNeg : UNeg), e->ty);
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
            if (!e->ty->isScalar())
                return type_error(e->loc, "expect scalar operand to '++'/'--'"), e;
            CType ty = e->ty->getKind() == TYPOINTER ? context.getSize_t() : e->ty;
            Expr obj = e;
            Expr one = wrap(ty, ConstantInt::get(irgen.ctx, APInt(
                 ty->getBitWidth(), 1)
            ), e->loc);
            (tok == TAddAdd) ? make_add(e, one) : make_sub(e, one);
            return binop(obj, Assign, e, e->ty);
        }
        case Ksizeof: {
            Expr e;
            consume();
            if (l.tok.tok == TLbracket) {
                consume();
                if (istype()) {
                    CType ty = type_name();
                    if (!ty)
                        return expect(getLoc(), "type-name or expression"), nullptr;
                    if (l.tok.tok != TRbracket)
                        return expectRB(getLoc()), nullptr;
                    consume();
                    return wrap(context.getSize_t(),
                                ConstantInt::get(irgen.ctx, APInt(context.getSize_tBitWidth(), getsizeof(ty))),
                                loc);
                }
                if (!(e = unary_expression()))
                    return nullptr;
                if (l.tok.tok != TRbracket)
                    return expectRB(getLoc()), nullptr;
                consume();
                return wrap(context.getSize_t(),
                            ConstantInt::get(irgen.ctx, APInt(context.getSize_tBitWidth(), getsizeof(e))), loc);
            }
            if (!(e = unary_expression()))
                return nullptr;
            return wrap(context.getSize_t(),
                        ConstantInt::get(irgen.ctx, APInt(context.getSize_tBitWidth(), getsizeof(e))), loc);
        }
        case K_Alignof: {
            Expr result;
            consume();
            if (l.tok.tok != TLbracket)
                return expectLB(getLoc()), nullptr;
            consume();
            if (istype()) {
                CType ty = type_name();
                if (!ty)
                    return expect(getLoc(), "type-name"), nullptr;
                result =
                    wrap(context.getSize_t(),
                         ConstantInt::get(irgen.ctx, APInt(context.getSize_tBitWidth(), getAlignof(ty))), loc);
            } else {
                Expr e = constant_expression();
                if (!e)
                    return nullptr;
                result =
                    wrap(context.getSize_t(),
                         ConstantInt::get(irgen.ctx, APInt(context.getSize_tBitWidth(), getAlignof(e))), loc);
            }
            if (l.tok.tok != TRbracket)
                return expectRB(getLoc()), nullptr;
            consume();
            return result;
        }
        case K__real:
        case K__imag:
        {
            consume();
            // lvalue if the expression has lvalue type
            Expr e = expression();
            if (!e)
                return parse_error(loc, "expect expression after '__real'/'__imag' operator"), nullptr;
            CType ty = context.getComplexElementType(e->ty);
            if (!checkArithmetic(e->ty))
                type_error("expect arithmetic type to '__real'/'__imag' operator");
            if (e->ty->isComplex())
                return unary(e, tok == K__imag ? C__imag__ : C__real__, ty);
            if (e->ty->isImaginary())
                return tok == K__real ? wrap(ty, llvm::Constant::getNullValue(irgen.wrapNoComplexScalar(ty)), e->loc) : e;
            return tok == K__real ? e : wrap(ty, llvm::Constant::getNullValue(irgen.wrapNoComplexScalar(ty)), e->loc);
        }
        case K__extension__:
        {
            consume();
            Expr e = cast_expression();
            if (!e)
                parse_error(loc, "expect expression after '__extension__'");
            return e;
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
            if (su.isFloat128) 
                ty = context.getFloat128();
            else if (su.isFloat) 
                ty = context.getFloat();
            const auto &Format = ty->getFloatKind().getFltSemantics();
            APFloat F(Format);
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
                        APFloat::getLargest(Format).toString(buffer);
                        diag = "floating point constant overflow: %R";
                    } else {
                        APFloat::getSmallest(Format).toString(buffer);
                        diag = "float point constant opUnderflow: %R";
                    }
                    lex_error(loc, diag, buffer.str());
                }
            }
            if (su.isImaginary)
                ty->addTag(TYIMAGINARY);
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
            if (bigVal >> static_cast<unsigned>(ty->getIntegerKind().getBitWidth()))
                warning("integer constant is too large with 'll' suffix");
        } else if (su.isLong) {
            ty = su.isUnsigned ? context.getULong() : context.getLong();
            if (bigVal >> static_cast<unsigned>(ty->getIntegerKind().getBitWidth()))
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
        auto bit_width = ty->getIntegerKind().getBitWidth();
        ConstantInt *CI = H ? 
                ConstantInt::get(irgen.ctx, APInt(bit_width, {H, L})) :
                ConstantInt::get(irgen.ctx, APInt(bit_width, L));
        if (su.isImaginary) 
            ty = context.tryGetImaginaryTypeFromNonImaginary(ty);
        return wrap(ty, CI, loc);
    }
    Expr wrap(CType ty, llvm::Constant *C, Location loc = Location()) {
        return ENEW(ConstantExpr){.loc = loc, .ty = ty, .C = C};
    }
    Expr getIntZero() { return intzero; }
    Expr getIntOne() { return intone; }
    Expr getFunctionNameExpr(Location loc) {
        return ENEW(ConstantArrayExpr) {.loc = loc, .ty = context.stringty, .array = string_pool.getAsUTF8(sema.pfunc->getKey())};
    }
    Expr primary_expression() {
        // primary expressions:
        //      constant
        //      `(` expression `)`
        //      identfier
        Expr result;
        Location loc = getLoc();
        const Token theTok = l.tok.tok;
        switch (theTok) {
        case TCharLit: {
            CType ty;
            switch (l.tok.itag) {
            case Prefix_none:
                ty = context.getInt();
                break;
            case Prefix_u8:
                ty = context.getChar8_t();
                break;
            case Prefix_L:
                ty = context.getWChar();
                break;
            case Prefix_u:
                ty = context.getChar16_t();
                break;
            case Prefix_U:
                ty = context.getChar32_t();
                break;
            default: llvm_unreachable("bad encoding");
            }
            result = wrap(ty, ConstantInt::get(irgen.ctx, APInt(ty->getIntegerKind().getBitWidth(), l.tok.i)), loc);
            consume();
        } break;
        case TStringLit: {
            xstring s = l.tok.str;
            auto enc = l.tok.getStringPrefix();
            consume();
            while (l.tok.tok == TStringLit) {
                auto enc2 = l.tok.getStringPrefix();
                s.push_back(l.tok.str);
                if (enc2 != enc)
                    type_error(getLoc(), "unsupported non-standard concatenation of string literals");
                consume();
            }
            s.make_eos();
            switch (enc) {
            case Prefix_none:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.stringty, .array = string_pool.getAsUTF8(s)};
                break;
            case Prefix_u8:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.str8ty, .array = string_pool.getAsUTF8(s)};
                break;
            case Prefix_L:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.wstringty, .array = string_pool.getAsUTF16(s, context.getWCharLog2() == 5)};
                break;
            case Prefix_u:
                result = ENEW(ConstantArrayExpr){
                    .loc = loc, .ty = context.str16ty, .array = string_pool.getAsUTF16(s, false)};
                break;
            case Prefix_U:
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
            if (LLVM_UNLIKELY(l.want_expr))
                result = getIntZero();
            else {
                IdentRef sym = l.tok.s;
                size_t idx;
                Variable_Info *it = sema.typedefs.getSym(sym, idx);
                consume();
                if (!it)
                    return type_error(loc, "use of undeclared identifier %I", sym), getIntZero();
                if (it->ty->hasTag(TYTYPEDEF))
                    return type_error(loc, "typedefs are not allowed here %I", sym), getIntZero();
                it->tags |= USED;
                CType ty = context.clone(it->ty);
                // lvalue conversions
                ty->lvalue_cast();
                if (it->val) {
                    if (it->tags & CONST_VAR) {
                        ty->addTags(TYREPLACED_CONSTANT | TYLVALUE);
                        return ENEW(ReplacedExpr){.loc = loc, .ty = ty, .C = it->val, .id = idx};
                    }
                    return wrap(ty, it->val, loc);
                }
                ty->addTag(TYLVALUE);
                switch (ty->getKind()) {
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
        case Knullptr:
            consume();
            return null_ptr_expr;
        case Kfalse:
            consume();
            return cfalse;
        case Ktrue:
            consume();
            return ctrue;
        case K__func__:
            consume();
            if (isTopLevel()) {
                warning(loc, "predefined identifier is only valid inside function");
                return ENEW(ConstantArrayExpr) {
                    .loc = loc,
                    .ty = context.stringty,
                    .array = string_pool.getEmptyString()
                };
            }
            return getFunctionNameExpr(loc);
        case K__PRETTY_FUNCTION__:
        {
            consume();
            if (isTopLevel()) {
                warning(loc, "predefined identifier is only valid inside function");
                return ENEW(ConstantArrayExpr) 
                    {
                        .loc = loc,
                        .ty = context.stringty,
                        .array = string_pool.getAsUTF8(StringRef("top level", 10))
                    };
            }
            SmallString<64> Str;
            llvm::raw_svector_ostream OS(Str);
            OS << sema.currentfunction;
            return ENEW(ConstantArrayExpr) 
                    {
                        .loc = loc, 
                        .ty = context.stringty, 
                        .array = string_pool.getAsUTF8(Str.str())
                    };
        }
        case K__builtin_FILE:
        case K__builtin_FUNCTION:
        case K__builtin_LINE:
        case K__builtin_COLUMN:
        {
            consume(); // eat __builtin_xxx
            if (l.tok.tok != TLbracket) {
                parse_error(loc, "expect '(' after %s", show(theTok));
            }
            consume(); // eat '('
            if (l.tok.tok != TRbracket) {
                parse_error(loc, "expect ')' after %s", show(theTok));
            }
            consume(); // eat ')'
            switch (theTok) {
            case K__builtin_FUNCTION:
                if (isTopLevel()) {
                    warning(loc, "predefined identifier is only valid inside function");
                    return ENEW(ConstantArrayExpr) {
                        .loc = loc, 
                        .ty = context.stringty, 
                        .array = string_pool.getEmptyString()
                    };
                }
                return getFunctionNameExpr(loc);
            case K__builtin_LINE:
                return wrap(context.getInt(), ConstantInt::get(irgen.integer_types[context.getIntLog2()], loc.line), loc);
            case K__builtin_COLUMN:
                return wrap(context.getInt(), ConstantInt::get(irgen.integer_types[context.getIntLog2()], loc.col), loc);
            case K__builtin_FILE:
            default: llvm_unreachable("");
            }
        }
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
                auto k = ty->getKind();
                bool isLvalue = false;
                consume();
                if (l.tok.tok != TIdentifier)
                    return expect(getLoc(), "identifier"), nullptr;
                if (isarrow) {
                    if (k != TYPOINTER)
                        return type_error(getLoc(), "member reference type %T is not a pointer; did you mean to use '.'"), result;
                    ty = result->ty->p;
                    isLvalue = true;
                } else {
                    if (k == TYPOINTER) {
                        return type_error(getLoc(), "member reference type %T is a pointer; did you mean to use '->'", result->ty), result;
                    }
                }
                if (k != TYSTRUCT && k != TYUNION)
                    return type_error(getLoc(), "member access is not struct or union"), result;
                for (size_t i = 0; i < ty->selems.size(); ++i) {
                    Declator pair = ty->selems[i];
                    if (l.tok.s == pair.name) {
                        result = ENEW(MemberAccessExpr) {
                            .loc = result->loc, .ty = pair.ty, .obj = result, .idx = (unsigned)i};
                        if (isLvalue) {
                            result->ty = context.clone(result->ty);
                            result->ty->addTag(TYLVALUE);
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
                               : ((result->ty->getKind() == TYPOINTER) ? result->ty->p : result->ty);
                Expr f = result;
                result =
                    ENEW(CallExpr){.loc = result->loc, .ty = ty->ret, .callfunc = f, .callargs = xvector<Expr>::get()};
                if (ty->getKind() != TYFUNCTION)
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
                if (result->ty->getKind() != TYPOINTER && rhs->ty->getKind() != TYPOINTER) {
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
        assert(e->ty->getKind() == TYPOINTER && "bad call to make_deref: expect a pointer");
        if ((e->ty->p->getKind() == TYINCOMPLETE) || e->ty->p->isVoid()) {
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
        ty->addTag(TYLVALUE);
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
            assert((e->ty->isComplex()) && "only complex number/structures are able to convert to bool");
            e = getBool(reverse);
        } break;
        case llvm::Value::ConstantPointerNullVal: {
            e = getBool(reverse);
        } break;
        default: break;
        }
    }
    const APInt *want_integer_constant() {
        Expr e = constant_expression();
        Location loc = e->loc;
        if (!e)
            return nullptr;
        if (e->k != EConstant)
            return type_error(loc, "case value is not a integer constant expression"), nullptr;
        if (auto CI = dyn_cast<ConstantInt>(e->C)) {
            CType to = sema.currentswitch->itest->ty;
            auto rA = to->getIntegerKind().getBitWidth();
            auto rB = e->ty->getIntegerKind().getBitWidth();
            if (rA < rB) {
                auto NEW_CI = ConstantInt::get(irgen.ctx, CI->getValue().trunc(rA));
                if (NEW_CI->getValue() != CI->getValue())
                    warning(loc, "overflow converting case value to switch condition type (%A to %A)", &CI->getValue(), &NEW_CI->getValue());
                CI = NEW_CI;
            } else if (rA > rB) {
                CI = ConstantInt::get(irgen.ctx, e->ty->isSigned() ? CI->getValue().sext(rA) : CI->getValue().zext(rA));
            }
            return &CI->getValue();
        }
        return type_error(loc, "statement requires expression of integer type (%T invalid)", e->ty), nullptr;
    }
    void valid_condition(Expr &e, bool reverse = false) {
        if (!e->ty->isScalar()) {
            type_error(e->loc, "conditions requires a scalar expression");
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
        case Kasm: return parse_asm(), checkSemicolon();
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
                parse_error(loc, "'case' statement not in switch statement");
            else {
                if (CaseStart) {
                    label_t L = jumper.createLabel();
                    insertLabel(L);
                    if (!CaseEnd) {
                        sema.currentswitch->switchs.push_back(SwitchCase(loc, L, CaseStart));
                    }
                    else {
                        if (*CaseStart == *CaseEnd) {
                            ONE_CASE:
                            sema.currentswitch->switchs.push_back(SwitchCase(loc, L, CaseStart));
                        }
                        else {
                            if (CaseStart->ugt(*CaseEnd))
                                warning(loc, "empty case range specified");
                            else if (*CaseStart == *CaseEnd) {
                                goto ONE_CASE;
                            } else {
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
                return insertBr(jumper.topBreak, loc);
            }
            return;
        }
        case Kreturn: {
            consume();
            if (l.tok.tok == TSemicolon) {
                consume();
                if (sema.currentfunction->ret->isVoid())
                    return insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = nullptr});

                return warning(loc, "function should not return void in a function return %T", sema.currentfunction->ret),
                       note("%s", "A return statement without an expression shall only appear in a function whose "
                                  "return type is void"),
                       insertStmt(SNEW(ReturnStmt){
                           .loc = loc,
                           .ret = wrap(sema.currentfunction->ret,
                                       llvm::UndefValue::get(irgen.wrap2(sema.currentfunction->ret)), loc)});
            }
            Expr e;
            if (!(e = expression()))
                return;
            checkSemicolon();
            if (sema.currentfunction->ret->isVoid())
                return warning(loc, "function should return a value in a function return void"),
                       note("A return statement with an expression shall not appear in a function whose return type is "
                            "void"),
                       insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = nullptr});
            return insertStmt(SNEW(ReturnStmt){.loc = loc, .ret = castto(e, sema.currentfunction->ret, Implict_Return)});
        }
        case Kwhile:
        case Kswitch: {
            Expr test;
            Token tok = l.tok.tok;
            if (!(test = Bexpression()))
                return;
            if (tok == Kswitch) {
                if (test->ty->isBool()) 
                    warning(test->loc, "switch condition has boolean value");
                CType origintestTy = test->ty;
                integer_promotions(test);
                if (test->ty->getKind() != TYPRIM && !test->ty->isInteger()) {
                    type_error(test->loc, "switch requires expression of integer type (%T invalid)", test->ty);
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
                bool hasDefault = true;
                if (!sema.currentswitch->sw_default_loc.isValid()) {
                    // no default found: insert `default: break;`
                    sema.currentswitch->sw_default_loc = getLoc();
                    insertLabel(sema.currentswitch->sw_default);
                    hasDefault = false;
                }
                insertLabel(LEAVE);
                // clang::Sema::ActOnFinishSwitchStmt
                std::stable_sort(sw->switchs.begin(), sw->switchs.end(), SwitchCase::equals);
                for (unsigned i = 1;i < sw->switchs.size();++i) {
                    const SwitchCase 
                       &A = sw->switchs[i],
                       &B = sw->switchs[i - 1];
                    if (SwitchCase::equals(A, B)) {
                        type_error(A.loc, "duplicate case value %A", A.CaseStart);
                        note(B.loc, "previous case defined here");
                        return;
                    }
                }
                // this method may be slow if GNU switchs are many.
                for (unsigned i = 0;i < sw->gnu_switchs.size();++i) {
                    const GNUSwitchCase &G = sw->gnu_switchs[i];
                    for (unsigned j = 0;j < sw->switchs.size();++j) {
                        const SwitchCase &A = sw->switchs[i];
                        if (G.contains(A)) {
                            Location startLoc = A.loc, endLoc = G.loc;
                            if (startLoc > endLoc)
                                std::swap(startLoc, endLoc);
                            const APInt sw_end = *G.CaseStart + G.range;
                            type_error(endLoc, "duplicate (or overlapping) case value %A in range (%A - %A)", A.CaseStart, G.CaseStart, &sw_end);
                            note(startLoc, "previous case defined here");
                            return;
                        }
                    }
                    for (unsigned j = 0;j < sw->gnu_switchs.size();++j) {
                        if (i == j)
                            continue;
                        const GNUSwitchCase &B = sw->gnu_switchs[j];
                        if (G.overlaps(B)) {
                            Location startLoc = G.loc, endLoc = B.loc;
                            if (startLoc > endLoc)
                                std::swap(startLoc, endLoc);
                            const APInt 
                                sw_end1 = *G.CaseStart + G.range,
                                sw_end2 = *B.CaseStart + B.range;
                            type_error(endLoc, "duplicate (or overlapping) case range: (%A - %A) and (%A - %A)", G.CaseStart, &sw_end1, B.CaseStart, &sw_end2);
                            note(startLoc, "previous case defined here");
                            return;
                        }
                    }
                }
                if (origintestTy->getKind() == TYENUM && !hasDefault) {
                    const auto &eelems = origintestTy->eelems;
                    llvm::BitVector handled_enums(eelems.size(), false);
                    for (const auto &S : sw->switchs) {
                        uint64_t val = S.CaseStart->getLimitedValue();
                        for (unsigned i = 0;i < eelems.size();++i) {
                            if (eelems[i].val == val) {
                                handled_enums.set(i);
                                continue;
                            }
                        }
                        warning(S.loc, "case value (%A) not in enumerated type %T", S.CaseStart, origintestTy);
                    }
                    for (const auto &G : sw->gnu_switchs) {
                        uint64_t start = G.CaseStart->getLimitedValue();
                        uint64_t range = G.range.getLimitedValue();
                        for (unsigned i = 0;i < eelems.size();++i) {
                            const uint64_t it = eelems[i].val;
                            if ((it >= start) && (it <= start + range)) {
                                handled_enums.set(i);
                                break;
                            }
                        }
                        const APInt sw_end = *G.CaseStart + G.range;
                        warning(G.loc, "case range (%A - %A) not in enumerated type %T", G.CaseStart, &sw_end, origintestTy);
                    }
                    unsigned indexs[3];
                    unsigned indexs_len = 0;
                    for (unsigned i = 0;i < handled_enums.size();++i) {
                        if (!handled_enums.test(i)) {
                            if (indexs_len < 3)
                                indexs[indexs_len++] = i;
                        }
                    }
                    switch (indexs_len) {
                    case 0:
                        // Good. All enumerations handled.
                        break;
                    case 1:
                        warning("enumeration value %I not handled in switch", eelems[indexs[0]].name);
                        break;
                    case 2:
                        warning("enumeration value %I and %I not handled in switch", eelems[indexs[0]].name, eelems[indexs[1]].name);
                    case 3:
                        warning("enumeration values %I, %I, and %I not handled in switch", eelems[indexs[0]].name, eelems[indexs[1]].name, eelems[indexs[2]].name); 
                    default: llvm_unreachable("logic error");
                    }
                }
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
            if (istype()) {
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
            label_t BODY = jumper.createLabel();
            llvm::SaveAndRestore<label_t> saved_b(jumper.topBreak, LEAVE);
            llvm::SaveAndRestore<label_t> saved_c(jumper.topContinue, CMP);
            insertLabel(CMP);
            if (cond)
                condJump(cond, BODY, LEAVE);
            else
                insertBr(BODY);
            insertLabel(BODY);
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
                            if (auto CI2 = dyn_cast<ConstantInt>(r->C)) {
                                warning(result->loc, "use of logical '&&' with constant operand");
                                note(result->loc, "use '&' for a bitwise operation");
                                result = CI2->isZero() ? getIntZero() : getIntOne();
                            }
                            else {
                                result = r;
                            }
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
                            if (auto CI2 = dyn_cast<ConstantInt>(r->C)) {
                                warning(result->loc, "use of logical '||' with constant operand");
                                note(result->loc, "use '|' for a bitwise operation");
                                result = CI2->isZero() ? getIntZero() : getIntOne();
                            }
                            else {
                                result = r;
                            }
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
        // GCC extension
        // x ? : y => x ? x : y
        if (l.tok.tok == TSemicolon)
            rhs = start;
        else if (!(rhs = conditional_expression()))
            return nullptr;
        if (!compatible(lhs->ty, rhs->ty))
            warning(getLoc(), "incompatible type for conditional-expression: (%T and %T)", lhs->ty, rhs->ty);
        conv(lhs, rhs);
        valid_condition(start);
        if (start->k == EConstant)
            if (auto CI = dyn_cast<ConstantInt>(start->C))
                return CI->isZero() ? rhs : lhs;
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
    static enum BinOp getAtomicrmwOp(Token tok) {
        switch (tok) {
        default: return static_cast<BinOp>(0);
        case TAsignAdd: return AtomicrmwAdd;
        case TAsignSub: return AtomicrmwSub;
        case TAsignBitOr: return AtomicrmwOr;
        case TAsignBitAnd: return AtomicrmwAnd;
        case TAsignBitXor: return AtomicrmwXor;
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
            if (result->ty->hasTag(TYATOMIC)) {
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
            if (istype())
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
                    if (!(sema.currentfunction->ret->isVoid())) {
                        if (s->k != SLabel) {
                            warning(loc2, "control reaches end of non-void function");
                            note("in the definition of function %I", sema.pfunc);
                        }
                        else if (s->labelName != nullptr) {
                            warning(loc2, "control reaches end of non-void function");
                            note("in label %I of function %I", s->labelName, sema.pfunc);
                        }
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
        leaveBlock();
        sema.typedefs.pop_function();
        consume();
        return head;
    }
/*
 * public iterface to Parser and Sema
 */
public: 
    // constructor
    Parser(SourceMgr &SM, IRGen &irgen, DiagnosticsEngine &Diag, xcc_context &theContext)
        : EvalHelper{Diag},l(SM, *this, theContext, Diag), context{theContext}, irgen{irgen},
          intzero{wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt::getZero(context.getInt()->getBitWidth())))},
          intone{wrap(context.getInt(), ConstantInt::get(irgen.ctx, APInt(context.getInt()->getBitWidth(), 1)))},
          cfalse{wrap(context.getBool(), ConstantInt::getFalse(irgen.ctx))},
          ctrue{wrap(context.getBool(), ConstantInt::getTrue(irgen.ctx))},
          string_pool{irgen}, null_ptr{llvm::ConstantPointerNull::get(irgen.pointer_type)}, null_ptr_expr{wrap(context.getNullPtr_t(), null_ptr)} { }
    // used by Lexer
    Expr constant_expression() { return conditional_expression(); }
    // the main entry to run parser
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
}; // end class Parser
