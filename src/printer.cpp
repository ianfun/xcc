raw_ostream &operator<<(llvm::raw_ostream &, const CType);

raw_ostream &operator<<(llvm::raw_ostream &OS, const Expr e) {
    switch (e->k) {
        case EConstant:
            e->C->print(OS /* IsForDebug=false */);
            return OS << '\n';
        case EBin:
            return OS << '(' << e->lhs  << ' ' << show(e->bop) << ' ' << e->rhs << ')';
        case EUnary:
            return OS << show(e->bop) << e->uoperand;
        case EVoid:
            return OS << e->voidexpr;
        case EVar:
            return OS << e->sval;
        case ECondition:
            return OS << e->cond << " ? " << e->cleft << " : " << e->cright;
        case ECast:
            return OS << '(' << e->ty << ')' << e->castval;
        case ECall:
            OS << e->callfunc << '(';
            if (e->callargs.empty())
                return OS << ')';
            for (size_t i = 0;i < e->callargs.size()-1;++i)
                OS << e->callargs[i] << ',';
            return OS << e->callargs.back() << ')';
        case ESubscript:
            return OS << e->left << '[' << e->right << ']';
        case EArray:
            OS << '{';
            for (const auto e: e->arr)
                OS << e << ',';
            return OS << '}';
        case EStruct:
            OS << '{';
            for (const auto e: e->arr2)
                OS << e << ',';
            return OS << '}';
        case EString:
            return OS.write_escaped(e->str.str());
        case EMemberAccess:
            return OS << e->obj << '.' << e->ty->selems[e->idx].name;
        case EArrToAddress:
            return OS << e->arr3;
        case EPostFix:
            return OS << e->poperand << show(e->pop);
    }
    llvm_unreachable("");
}
static StringRef get_pointer_qual_str(uint32_t a){
    if (a & TYCONST) return "const";
    if (a & TYVOLATILE) return "volatile";
    if (a & TYRESTRICT) return "restrict";
    return StringRef();
}
static StringRef get_prim_str(uint32_t tags){
    if (tags & TYVOID) return "void";
    if (tags & TYBOOL) return "_Bool";
    if (tags & TYCOMPLEX) return "_Complex";
    if (tags & TYINT8) return "char";
    if (tags & TYINT16) return "short";
    if (tags & TYINT32) return "int";
    if (tags & TYINT64) return "long long";
    if (tags & TYUINT8) return "unsigned char";
    if (tags & TYUINT16) return "unsigned short";
    if (tags & TYUINT32) return "unsigned int";
    if (tags & TYUINT64) return "unsigned long long";
    if (tags & TYINT128) return "__int128";
    if (tags & TYUINT128) return "__uint128";
    if (tags & TYFLOAT) return "float";
    if (tags & TYDOUBLE) return "double";
    if (tags & TYF128) return "__float128";
    return "(unknown basic type)";
}
static const char *get_type_name_str(enum CTypeKind t) {
    switch (t) {
        case TYSTRUCT: return "struct";
        case TYUNION: return "union";
        case TYENUM: return "enum";
        default: llvm_unreachable("bad arguments to get_type_name_str");
    }
}
static StringRef get_storage_str(uint32_t tags) {
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

raw_ostream &operator<<(llvm::raw_ostream &OS, const CType ty) {
    switch (ty->k) {
        case TYPRIM:
        {
            auto tags = ty->tags;
            auto str0 = get_pointer_qual_str(tags);
            if (str0.size())
                OS << str0 << ' ';
            auto str = get_storage_str(tags);
            if (str.size())
                OS << str << ' ';
            return OS << get_prim_str(ty->tags);
        }
        case TYPOINTER:
        {
            auto str = get_pointer_qual_str(ty->tags);
            bool isArrType = ty->p->k == TYARRAY;
            if (isArrType)
                OS << '(';
            OS << '*' << ty->p << ' ';
            if (!str.empty())
                OS << str << ' ';
            if (isArrType)
                OS << ')';
            return OS;
        }
        case TYSTRUCT:
            return OS << "struct " << ty->ename->getKey();
        case TYUNION:
            return OS << "union " << ty->ename->getKey();
        case TYENUM:
            return OS << "enum " << ty->ename->getKey();
        case TYBITFIELD:
            return OS << ty->bittype << " : " << ty->bitsize;
        case TYARRAY:
            OS << ty->arrtype << " [";
            if (ty->vla)
                OS << ty->vla;
            else if (ty->hassize)
                OS << ty->arrsize;
            return OS << ']';
        case TYFUNCTION:
        {
            auto str = get_storage_str(ty->tags);
            OS << '(' << ty->ret;
            if (ty->tags & TYINLINE)
                OS << "inline" << ' ';
            if (ty->tags & TYNORETURN)
                OS << "_Noreturn" << ' ';
            if (str.size())
                OS << str;
            OS << " (*)(";
            if (!ty->params.empty()) {
                for (const auto &e: ty->params) {
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
        case TYINCOMPLETE:
            return OS << get_type_name_str(ty->tag) << ' ' << ty->name->getKey();
    }
    llvm_unreachable("");
}

raw_ostream &operator>>(llvm::raw_ostream &OS, const CType ty) {
    switch (ty->k) {
        case TYPRIM:
        {
            auto tags = ty->tags;
            auto str0 = get_pointer_qual_str(tags);
            if (str0.size())
                OS << str0 << ' ';
            auto str = get_storage_str(tags);
            if (str.size())
                OS << str << ' ';
            return OS << get_prim_str(ty->tags);
        }
        case TYPOINTER:
        {
            auto str = get_pointer_qual_str(ty->tags);
            OS << "pointer[elementType=" << ty->p;
            if (!str.empty())
                OS << ", qualifiers=" << str;
            return OS << "]";
        }
        case TYSTRUCT:
            return OS << "struct " << ty->ename->getKey();
        case TYUNION:
            return OS << "union " << ty->ename->getKey();
        case TYENUM:
            return OS << "enum " << ty->ename->getKey();
        case TYBITFIELD:
            return OS << ty->bittype << " : " << ty->bitsize;
        case TYARRAY:
            OS << "array[elementType=" << ty->arrtype << ", size=";
            if (ty->vla)
                OS << ty->vla;
            else if (ty->hassize)
                OS << ty->arrsize;
            return OS << "]";
        case TYFUNCTION:
        {
            auto str = get_storage_str(ty->tags);
            OS << "Function[ret=" << ty->ret;
            if (ty->tags & TYINLINE)
                OS << "inline" << ' ';
            if (ty->tags & TYNORETURN)
                OS << "_Noreturn" << ' ';
            if (str.size())
                OS << str;
            OS << ", params=[";
            if (!ty->params.empty()) {
                for (const auto &e: ty->params) {
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
        case TYINCOMPLETE:
            return OS << get_type_name_str(ty->tag) << ' ' << ty->name->getKey();
    }
    llvm_unreachable("");
}

void print_cdecl(const CType ty, raw_ostream &OS) {
    switch (ty->k) {
        case TYPRIM:
        {
            auto tags = ty->tags;
            auto str0 = get_pointer_qual_str(tags);
            if (str0.size())
                OS << str0 << ' ';
            auto str = get_storage_str(tags);
            if (str.size())
                OS << str << ' ';
            OS << get_prim_str(ty->tags);
            break;
        }
        case TYPOINTER:
        {
            auto str = get_pointer_qual_str(ty->tags);
            if (!str.empty())
                OS << str << ' ';
            OS << "pointer to ";
            print_cdecl(ty->p, OS);
            break;
        }
        case TYSTRUCT:
            OS << "struct " << ty->ename->getKey();
            break;
        case TYUNION:
            OS << "union " << ty->ename->getKey();
            break;
        case TYENUM:
            OS << "enum " << ty->ename->getKey();
            break;
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
        case TYFUNCTION:
        {
            auto str = get_storage_str(ty->tags);
            OS << "function (";
            if (!ty->params.empty()) {
                for (const auto &e: ty->params) {
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
        case TYINCOMPLETE:
            OS << get_type_name_str(ty->tag) << ' ' << ty->name->getKey();
            break;
    }
}
void print_cdecl(StringRef name, const CType ty, raw_ostream &OS = llvm::errs(), bool addNewLine = false) {
    OS << "declare " << name << " as ";
    print_cdecl(ty, OS);
    if (addNewLine)
        OS << '\n';
}
