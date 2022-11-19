namespace detail {
static StringRef get_pointer_qual_str(type_tag_t a) {
    if (a & TYCONST)
        return "const";
    if (a & TYVOLATILE)
        return "volatile";
    if (a & TYRESTRICT)
        return "restrict";
    return StringRef();
}
// clang::BuiltinType::getName
static StringRef get_prim_str(const_CType ty) {
    if (ty->hasTag(TYVOID)) return "void";
    if (ty->isInteger()) {
        bool s = ty->isSigned();
        switch (ty->getIntegerKind().asLog2()) {
            case 1: return "_Bool"; // XXX: C23 is bool
            case 3: return s ? "char" : "unsigned char";
            case 4: return s ? "short" : "unsigned short";
            case 5: return s ? "int" : "unsigned int"; // XXX: 'unsigned' is ok
            case 6: return s ? "long long" : "unsigned long long";
            case 7: return s ? "__int128" : "unsigned __int128";
            default: llvm_unreachable("invalid IntegerKind");
        }
    }
    switch (ty->getFloatKind().asEnum()) {
        case F_Half: return "half";
        case F_BFloat: return "__bf16";
        case F_Float: return "float";
        case F_Double: return "double";
        case F_x87_80: return "__float80";
        case F_Quadruple: return "__float128";
        case F_PPC128: return "__ibm128";
        case F_Decimal32: return "_Decimal32";
        case F_Decimal64: return "_Decimal64";
        case F_Decimal128: return "_Decimal128";
    }
    llvm_unreachable("broken type: invalid FloatKindEnum");
}
raw_ostream &print_basic(raw_ostream &OS, const_CType ty) {
    if (ty->hasTag(TYCOMPLEX))
        OS << "_Complex ";
    else if (ty->hasTag(TYIMAGINARY))
        OS << "_Imaginary";
    return OS << get_prim_str(ty);
}
static StringRef get_storage_str(type_tag_t tags) {
    if (tags & TYSTATIC)
        return "static";
    if (tags & TYEXTERN)
        return "extern";
    if (tags & TYTHREAD_LOCAL)
        return "_Thread_local";
    if (tags & TYTYPEDEF)
        return "typedef";
    if (tags & TYREGISTER)
        return "register";
    if (tags & TYATOMIC)
        return "_Atomic";
    return StringRef();
}
} // end namespace detail

raw_ostream &operator<<(llvm::raw_ostream &, const CType);

raw_ostream &operator<<(llvm::raw_ostream &OS, const Expr e) {
    switch (e->k) {
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
    case EVar: return OS << e->sval;
    case ECondition: return OS << e->cond << " ? " << e->cleft << " : " << e->cright;
    case ECast: return OS << '(' << e->ty << ')' << e->castval;
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

raw_ostream &operator<<(llvm::raw_ostream &OS, const CType ty) {
    switch (ty->k) {
    case TYPRIM: {
        auto tags = ty->tags;
        auto str0 = detail::get_pointer_qual_str(tags);
        if (str0.size())
            OS << str0 << ' ';
        auto str = detail::get_storage_str(tags);
        if (str.size())
            OS << str << ' ';
        return print_basic(OS, ty);
    }
    case TYPOINTER: {
        auto str = get_pointer_qual_str(ty->tags);
        bool isArrType = ty->p->k == TYARRAY;
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
        if (ty->vla)
            OS << ty->vla;
        else if (ty->hassize)
            OS << ty->arrsize;
        return OS << ']';
    case TYFUNCTION: {
        auto str = detail::get_storage_str(ty->tags);
        OS << '(' << ty->ret;
        if (ty->tags & TYINLINE)
            OS << "inline" << ' ';
        if (ty->tags & TYNORETURN)
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

raw_ostream &operator>>(llvm::raw_ostream &OS, const CType ty) {
    switch (ty->k) {
    case TYPRIM: {
        auto tags = ty->getTags();
        auto str0 = detail::get_pointer_qual_str(tags);
        if (str0.size())
            OS << str0 << ' ';
        auto str = detail::get_storage_str(tags);
        if (str.size())
            OS << str << ' ';
        return detail::print_basic(OS, ty);
    }
    case TYPOINTER: {
        auto str = detail::get_pointer_qual_str(ty->getTags());
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
        if (ty->vla)
            OS << ty->vla;
        else if (ty->hassize)
            OS << ty->arrsize;
        return OS << "]";
    case TYFUNCTION: {
        auto str = detail::get_storage_str(ty->tags);
        OS << "Function[ret=" << ty->ret;
        if (ty->tags & TYINLINE)
            OS << "inline" << ' ';
        if (ty->tags & TYNORETURN)
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

void print_cdecl(const CType ty, raw_ostream &OS) {
    switch (ty->k) {
    case TYPRIM: {
        auto tags = ty->tags;
        auto str0 = detail::get_pointer_qual_str(tags);
        if (str0.size())
            OS << str0 << ' ';
        auto str = detail::get_storage_str(tags);
        if (str.size())
            OS << str << ' ';
        print_basic(OS, ty);
        break;
    }
    case TYPOINTER: {
        auto str = detail::get_pointer_qual_str(ty->tags);
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
        if (ty->vla)
            OS << ' ' << ty->vla;
        else if (ty->hassize)
            OS << ' ' << ty->arrsize;
        OS << " of ";
        print_cdecl(ty->arrtype, OS);
        break;
    case TYFUNCTION: {
        auto str = get_storage_str(ty->tags);
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
        if (ty->tags & TYINLINE)
            OS << "inline" << ' ';
        if (ty->tags & TYNORETURN)
            OS << "_Noreturn" << ' ';
        if (str.size())
            OS << str;
        print_cdecl(ty->ret, OS);
        break;
    }
    case TYINCOMPLETE: OS << show(ty->tag) << ' ' << ty->name->getKey(); break;
    }
}
void print_cdecl(StringRef name, const CType ty, raw_ostream &OS = llvm::errs(), bool addNewLine = false) {
    OS << "declare " << name << " as ";
    print_cdecl(ty, OS);
    if (addNewLine)
        OS << '\n';
}
