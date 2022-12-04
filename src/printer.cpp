raw_ostream &operator<<(llvm::raw_ostream &, const_CType);

raw_ostream &operator<<(llvm::raw_ostream &OS, const_Expr e) {
    switch (e->k) {
    case EBitCast:
        return OS << "(bit cast to " << e->ty << ")" << e->src;
    case EConstantArraySubstript: e->carray->getInitializer()->print(OS); return OS;
    case EConstantArray: e->array->print(OS); return OS;
    case EConstant:
        if (auto CI = dyn_cast<ConstantInt>(e->C))
            CI->getValue().print(OS, e->ty->isSigned());
        else if (auto CFP = dyn_cast<ConstantFP>(e->C))
            CFP->getValue().print(OS);
        else
            e->C->print(OS);
        return OS;
    case EBin: return OS << '(' << e->lhs << ' ' << show(e->bop) << ' ' << e->rhs << ')';
    case EUnary: return OS << show(e->bop) << e->uoperand;
    case EVoid: return OS << e->voidexpr;
    case EVar: return OS << e->varName->getKey();
    case ECondition: return OS << e->cond << " ? " << e->cleft << " : " << e->cright;
    case ECast: return OS << '(' << e->ty << ')' << e->castval;
    case ESizeof: return OS  << "sizeof(" << e->theType << ')';
    case ECall:
        OS << e->callfunc << '(';
        if (e->callargs.empty())
            return OS << ')';
        for (size_t i = 0; i < e->callargs.size() - 1; ++i)
            OS << e->callargs[i] << ',';
        return OS << e->callargs.back() << ')';
    case ESubscript: return OS << e->left << '[' << e->right << ']';
    case EArray:
        OS << '{';
        for (const auto e : e->arr)
            OS << e << ',';
        return OS << '}';
    case EStruct:
        OS << '{';
        for (const auto e : e->arr2)
            OS << e << ',';
        return OS << '}';
    case EMemberAccess: return OS << e->obj << '.' << e->ty->selems[e->idx].name;
    case EArrToAddress: return OS << e->arr3;
    case EPostFix: return OS << e->poperand << show(e->pop);
    }
    llvm_unreachable("");
}

raw_ostream &operator<<(llvm::raw_ostream &OS, const_CType ty) {
    switch (ty->getKind()) {
    case TYPRIM: {
        auto str0 = ty->get_pointer_qual_str();
        if (str0.size())
            OS << str0 << ' ';
        auto str = ty->get_storage_str();
        if (str.size())
            OS << str << ' ';
        return ty->print_basic(OS);
    }
    case TYPOINTER: {
        if (ty->isNullPtr_t())
            return OS << (ty->hasTag(TYTYPEDEF) ? "typedef nullptr_t" : "nullptr_t");
        auto str = ty->get_pointer_qual_str();
        bool isArrType = ty->p->getKind() == TYARRAY;
        if (isArrType)
            OS << '(';
        OS << ty->p << " *";
        if (!str.empty())
            OS << str << ' ';
        if (isArrType)
            OS << ')';
        return OS;
    }
    case TYSTRUCT: return OS << "struct " << ty->ename->getKey();
    case TYUNION: return OS << "union " << ty->ename->getKey();
    case TYENUM: return OS << "enum " << ty->ename->getKey();
    case TYBITFIELD: return OS << ty->bittype << " : " << ty->bitsize;
    case TYARRAY:
        OS << ty->arrtype << " [";
        if (ty->hassize)
            OS << ty->arrsize;
        return OS << ']';
    case TYVLA:
    {
        Expr vla_expr = ty->vla_expr;
        if (vla_expr->k == ECast)
            vla_expr = vla_expr->castval;
        return OS << ty->vla_arraytype << " [" << vla_expr << ']';
    }
    case TYFUNCTION: {
        auto str = ty->get_storage_str();
        OS << '(' << ty->ret;
        if (ty->hasTag(TYINLINE))
            OS << "inline" << ' ';
        if (ty->hasTag(TYNORETURN))
            OS << "_Noreturn" << ' ';
        if (str.size())
            OS << str;
        OS << " (*)(";
        if (!ty->params.empty()) {
            for (const auto &e : ty->params) {
                OS << e.ty;
                if (e.name)
                    OS << ' ' << e.name->getKey();
                OS << ", ";
            }
            OS << ty->params.back().ty;
            if (ty->params.back().name)
                OS << ' ' << ty->params.back().name->getKey();
            if (ty->isVarArg)
                OS << ", ...";
        }
        return OS << ')' << ')';
    }
    case TYINCOMPLETE: return OS << show(ty->tag) << ' ' << ty->name->getKey();
    }
    llvm_unreachable("");
}

raw_ostream &operator>>(llvm::raw_ostream &OS, const_CType ty) {
    switch (ty->getKind()) {
    case TYPRIM: {
        auto str0 = ty->get_pointer_qual_str();
        if (str0.size())
            OS << str0 << ' ';
        auto str = ty->get_storage_str();
        if (str.size())
            OS << str << ' ';
        return ty->print_basic(OS);
    }
    case TYPOINTER: {
        if (ty->isNullPtr_t())
            return OS << (ty->hasTag(TYTYPEDEF) ? "typedef nullptr_t" : "nullptr_t");
        auto str = ty->get_pointer_qual_str();
        OS << "pointer[elementType=" << ty->p;
        if (!str.empty())
            OS << ", qualifiers=" << str;
        return OS << "]";
    }
    case TYSTRUCT: return OS << "struct " << ty->ename->getKey();
    case TYUNION: return OS << "union " << ty->ename->getKey();
    case TYENUM: return OS << "enum " << ty->ename->getKey();
    case TYBITFIELD: return OS << ty->bittype << " : " << ty->bitsize;
    case TYARRAY:
        OS << "array[elementType=" << ty->arrtype << ", size=";
        if (ty->hassize)
            OS << ty->arrsize;
        return OS << "]";
    case TYVLA:
    {
        Expr vla_expr = ty->vla_expr;
        if (vla_expr->k == ECast)
            vla_expr = vla_expr->castval;
        OS << "variableLengthArray[elementType=" << ty->vla_arraytype << ", size=" << vla_expr << "]";
    } break;
    case TYFUNCTION: {
        auto str = ty->get_storage_str();
        OS << "Function[ret=" << ty->ret;
        if (ty->hasTag(TYINLINE))
            OS << "inline" << ' ';
        if (ty->hasTag(TYNORETURN))
            OS << "_Noreturn" << ' ';
        if (str.size())
            OS << str;
        OS << ", params=[";
        if (!ty->params.empty()) {
            for (const auto &e : ty->params) {
                OS << e.ty;
                if (e.name)
                    OS << ' ' << e.name->getKey();
                OS << ", ";
            }
            OS << ty->params.back().ty;
            if (ty->params.back().name)
                OS << ' ' << ty->params.back().name->getKey();
        }
        if (ty->isVarArg)
            OS << "], isVarArg=true]";
        else
            OS << "], isVarArg=false]";
        return OS << ']';
    }
    case TYINCOMPLETE: return OS << show(ty->tag) << ' ' << ty->name->getKey();
    }
    llvm_unreachable("");
}

void print_cdecl(const_CType ty, raw_ostream &OS) {
    switch (ty->getKind()) {
    case TYPRIM: {
        auto str0 = ty->get_pointer_qual_str();
        if (str0.size())
            OS << str0 << ' ';
        auto str = ty->get_storage_str();
        if (str.size())
            OS << str << ' ';
        ty->print_basic(OS);
        break;
    }
    case TYPOINTER: {
        if (ty->isNullPtr_t())
            return static_cast<void>(OS << (ty->hasTag(TYTYPEDEF) ? "typedef nullptr_t" : "nullptr_t"));
        auto str = ty->get_pointer_qual_str();
        if (!str.empty())
            OS << str << ' ';
        OS << "pointer to ";
        print_cdecl(ty->p, OS);
        break;
    }
    case TYSTRUCT: OS << "struct " << ty->ename->getKey(); break;
    case TYUNION: OS << "union " << ty->ename->getKey(); break;
    case TYENUM: OS << "enum " << ty->ename->getKey(); break;
    case TYBITFIELD:
        OS << "bitfield " << ty->bitsize << " of ";
        print_cdecl(ty->bittype, OS);
        break;
    case TYARRAY:
        OS << "array";
        if (ty->hassize)
            OS << ' ' << ty->arrsize;
        OS << " of ";
        print_cdecl(ty->arrtype, OS);
        break;
    case TYVLA:
    {
        Expr vla_expr = ty->vla_expr;
        if (vla_expr->k == ECast)
            vla_expr = vla_expr->castval;
        OS << "variable length array " << vla_expr << " of ";
        print_cdecl(ty->vla_arraytype, OS);
    } break;
    case TYFUNCTION: {
        auto str = ty->get_storage_str();
        OS << "function (";
        if (!ty->params.empty()) {
            for (const auto &e : ty->params) {
                print_cdecl(e.ty, OS);
                if (e.name)
                    OS << ' ' << e.name->getKey();
                OS << ", ";
            }
            print_cdecl(ty->params.back().ty, OS);
            if (ty->params.back().name)
                OS << ' ' << ty->params.back().name->getKey();
            if (ty->isVarArg)
                OS << ", ...";
        }
        OS << ") returning ";
        if (ty->hasTag(TYINLINE))
            OS << "inline ";
        if (ty->hasTag(TYNORETURN))
            OS << "_Noreturn ";
        if (str.size())
            OS << str;
        print_cdecl(ty->ret, OS);
        break;
    }
    case TYINCOMPLETE: OS << show(ty->tag) << ' ' << ty->name->getKey(); break;
    }
}
void print_cdecl(StringRef name, const_CType ty, raw_ostream &OS = llvm::errs(), bool addNewLine = false) {
    OS << "declare " << name << " as ";
    print_cdecl(ty, OS);
    if (addNewLine)
        OS << '\n';
}
