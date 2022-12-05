// clang::StringLiteral::outputString
void printCString(raw_ostream &OS, StringRef str) {
    OS.changeColor(raw_ostream::CYAN);
    if (str.empty()) {
        OS << '"';
        goto END;
    }
    if (str.back() == '\0')
        str = str.drop_back();
    OS << '"';
    for (const char c: str) {
        switch (c) {
    case '\\': OS << "\\\\"; break;
    case '"': OS << "\\\""; break;
    case '\a': OS << "\\a"; break;
    case '\b': OS << "\\b"; break;
    case '\f': OS << "\\f"; break;
    case '\n': OS << "\\n"; break;
    case '\r': OS << "\\r"; break;
    case '\t': OS << "\\t"; break;
    case '\v': OS << "\\v"; break;
    default: 
        if (std::isprint(c))
            OS << c;
        else {
            char buf[4] = 
            {'\\',
                static_cast<char>('0' + ((static_cast<unsigned>(c) >> 6) & 7)),
                static_cast<char>('0' + ((static_cast<unsigned>(c) >> 3) & 7)),
                static_cast<char>('0' + ((static_cast<unsigned>(c) >> 0) & 7))
            };
            OS << buf;
        }
    }
}
END:
    OS.resetColor();
    OS << '"';
}

raw_ostream &operator<<(llvm::raw_ostream &, const_CType);

void printConstant(const llvm::Constant *C, llvm::raw_ostream &OS) {
    if (const ConstantInt *CI = dyn_cast<ConstantInt>(C)) {
        OS.changeColor(raw_ostream::GREEN);
        CI->getValue().print(OS, true);
        OS.resetColor();
        return;
    }
    if (const ConstantFP *CFP = dyn_cast<ConstantFP>(C)) {
        OS.changeColor(raw_ostream::GREEN);
        return CFP->getValue().print(OS);
        OS.resetColor();
    }
    if (isa<llvm::ConstantAggregateZero>(C)) {
        OS << "{0}";
        return;
    }
    if (const llvm::ConstantAggregate *CAG = dyn_cast<llvm::ConstantAggregate>(C)) {
        unsigned max = CAG->getNumOperands();
        OS << "{";
        for (unsigned i = 0;i < max;++i) {
            printConstant(CAG->getOperand(i), OS);
            if (i != max) 
                OS << ", ";
        }
        OS << "}";
        return;
    }
    if (const llvm::ConstantDataArray *CDA = dyn_cast<llvm::ConstantDataArray>(C)) {
        if (CDA->isString())
            return printCString(OS, CDA->getAsString());
        auto T = CDA->getElementType();
        if (T->isIntegerTy()) {
            unsigned max = CDA->getNumElements();
            OS << "{";
            for (unsigned i = 0;i < max;++i) {
                OS << CDA->getElementAsInteger(i);
                if (i != max)
                    OS << ", ";
            }
            OS << "}";
            return;
        }
        if (T->isDoubleTy()) {
            unsigned max = CDA->getNumElements();
            OS << "{";
            for (unsigned i = 0;i < max;++i) {
                OS << CDA->getElementAsDouble(i);
                if (i != max)
                    OS << ", ";
            }
            OS << "}";
            return;
        }
        if (T->isFloatTy()) {
            unsigned max = CDA->getNumElements();
            OS << "{";
            for (unsigned i = 0;i < max;++i) {
                OS << CDA->getElementAsFloat(i);
                if (i != max)
                    OS << ", ";
            }
            OS << "}";
            return;
        }
    }
    if (isa<llvm::ConstantPointerNull>(C)) {
        OS.changeColor(raw_ostream::RED);
        OS << "nullptr"; // nullptr, NULL ?
        OS.resetColor();
        return;
    }
    if (const llvm::BlockAddress *BADR = dyn_cast<llvm::BlockAddress>(C)) {
        const llvm::BasicBlock *BB = BADR->getBasicBlock();
        OS << "label-address " << BB->getName();
    }
    if (const llvm::GlobalVariable *GV = dyn_cast<llvm::GlobalVariable>(C))
        return printConstant(GV->getInitializer(), OS);
    C->print(OS);
}

raw_ostream &operator<<(llvm::raw_ostream &OS, const_Expr e) {
    switch (e->k) {
    case EBitCast:
        return OS << "(" << raw_ostream::BLUE << e->ty << raw_ostream::RESET << ")" << e->src;
    case EConstantArraySubstript: 
        printConstant(e->carray->getInitializer(), OS);
        return OS;
    case EConstantArray:
        printConstant(e->array, OS);
        return OS;
    case EConstant:
        printConstant(e->C, OS);
        return OS;
    case EBin: return OS << '(' << e->lhs << ' ' << show(e->bop) << ' ' << e->rhs << ')';
    case EUnary:
        if (e->uop == AddressOf && e->ty->p->getKind() == TYFUNCTION && !e->ty->hasTag(TYLVALUE))
            return OS << e->uoperand;
        return OS << show(e->uop) << e->uoperand;
    case EVoid: return OS << e->voidexpr;
    case EVar: 
        {
            OS.changeColor(raw_ostream::MAGENTA);
            OS << e->varName->getKey();
            OS.resetColor();
            return OS;
        }
    case ECondition: return OS << e->cond << " ? " << e->cleft << " : " << e->cright;
    case ECast: return OS << '(' << raw_ostream::BLUE << e->ty << raw_ostream::RESET << ')' << e->castval;
    case ESizeof: return OS << "sizeof(" << raw_ostream::BLUE << e->theType << raw_ostream::RESET << ')';
    case ECall:
        OS << e->callfunc << '(';
        if (e->callargs.empty())
            return OS << ')';
        for (size_t i = 0; i < e->callargs.size() - 1; ++i)
            OS << e->callargs[i] << ", ";
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
        OS << ty->p << "*";
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
        OS << '(' << ty->ret << ' ';
        if (ty->hasTag(TYINLINE))
            OS << "inline ";
        if (ty->hasTag(TYNORETURN))
            OS << "_Noreturn ";
        if (str.size())
            OS << str << ' ';
        OS << "(*)(";
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
            OS << str << ' ';
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
            size_t end = ty->params.size() - 1;
            for (size_t i = 0;i <= end;++i) {
                const Declator &e = ty->params[i]; 
                print_cdecl(e.ty, OS);
                if (e.name)
                    OS << ' ' << e.name->getKey();
                if (i == end) {
                    if (ty->isVarArg)
                        OS << ", ...";
                } else {
                    OS << ", ";
                }
            }
        }
        OS << ") returning ";
        if (ty->ret->hasTag(TYINLINE))
            OS << "inline ";
        if (ty->ret->hasTag(TYNORETURN))
            OS << "_Noreturn ";
        if (str.size())
            OS << str << ' ';
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

enum AST_Dump_Format {
    AST_Default, AST_JSON
};
struct AstDumper {
    llvm::raw_ostream &OS;
    StringRef NL;
    StringRef tab;
    unsigned IndentLevel;
    unsigned Indentation;
    AstDumper(llvm::raw_ostream &OS, StringRef NL = "\n", StringRef tab = "    ", unsigned IndentLevel = 0, unsigned Indentation = 2) : OS{OS}, NL{NL}, tab{tab}, IndentLevel{IndentLevel}, Indentation{Indentation} {}
    void printExpr(Expr e) {
        if (e->k == EBin)
            OS << e->lhs << ' ' << show(e->bop) << ' ' << e->rhs;
        else
            OS << e;
    }
    void Indent() {
        for (unsigned i = 0; i < IndentLevel; ++i)
            OS << ' ';
    }
    void printCType(CType ty) {
        OS.changeColor(raw_ostream::BLUE);
        OS << ty;
        OS.resetColor();
    }
    void PrintStmt(Stmt s) {
        IndentLevel += Indentation;
        Indent();
        dump(s);
        IndentLevel -= Indentation;
    }
    void dump(Stmt s) {
        switch (s->k) {
            case SHead:
                for (Stmt ptr = s->next; ptr; ptr = ptr->next)
                    dump(ptr);
                return;
            case SCompound:
                OS << "{" << NL;
                IndentLevel += Indentation;
                for (Stmt ptr = s->inner; ptr; ptr = ptr->next)
                    PrintStmt(ptr);
                IndentLevel -= Indentation;
                Indent();
                OS << "}" << NL;
                return;
            case SLabel:
                OS << "%" << s->label << ':';
                if (s->labelName)
                    OS << " (" << s->labelName->getKey() << ")";
                OS << NL;
                return;
            case SGoto:
                OS.changeColor(raw_ostream::RED);
                OS << "goto";
                OS.resetColor();
                OS << " %" << s->location << ';' << NL;
                return;
            case SCondJump:
                OS.changeColor(raw_ostream::RED);
                OS << "branch ";
                OS.resetColor();
                printExpr(s->test);
                OS << " %" << s->T << ", %" << s->F << ';' << NL;
                return;
            case SSwitch:
            {
                bool isSigned = s->itest->ty->isSigned();
                OS.changeColor(raw_ostream::RED);
                OS << "switch";
                OS.resetColor();
                OS << s->itest << NL;
                for (const SwitchCase &it: s->switchs) {
                    OS << raw_ostream::RED << "  case " << raw_ostream::RESET;
                    it.CaseStart->print(OS, isSigned);
                    OS << " => %" << it.label;
                }
                for (const GNUSwitchCase &it: s->gnu_switchs) {
                    OS << raw_ostream::RED << "  case " << raw_ostream::RESET;
                    it.CaseStart->print(OS, isSigned);
                    OS << " ... ";
                    (*it.CaseStart + it.range).print(OS, isSigned);
                    OS << " => %" << it.label;
                }
                OS << raw_ostream::RED << "  default" << raw_ostream::RESET << " =>" << s->sw_default << NL;
            } return;
            case SReturn:
                OS.changeColor(raw_ostream::RED);
                OS << "return";
                OS.resetColor();
                if (s->ret) {
                    OS << ' ';
                    printExpr(s->ret);
                }
                OS << ';' << NL;
                return;
            case SExpr:
                printExpr(s->exprbody);
                OS << ';' << NL;
                return;
            case SNoReturnCall:
                OS << "(noreturn call) ";
                printExpr(s->call_expr);
                OS << ';' << NL;
                return;
            case SAsm:
                OS.changeColor(raw_ostream::RED);
                OS << "__asm__";
                OS.resetColor();
                OS << "(";
                printCString(OS, s->asms.str().drop_back());
                OS << ')' << NL;
                return;
            case SVarDecl:
            {
                size_t end = s->vars.size();
                for (size_t i = 0;i < end;++i) {
                    const VarDecl &it = s->vars[i];
                    printCType(it.ty);
                    OS << " " << raw_ostream::MAGENTA << it.name->getKey() << raw_ostream::RESET;
                    if (it.init) {
                        OS << " = ";
                        printExpr(it.init);
                    }
                    if (i == end)
                        OS << ";" << NL;
                    else 
                        (void)(OS << ',' << NL), Indent();
                }
            }
            return;
            case SFunction:
                OS << "Function ";
                OS.changeColor(raw_ostream::MAGENTA);
                OS << s->funcname->getKey();
                OS.resetColor();
                OS << ": ";
                printCType(s->functy);
                OS << " (";
                for (size_t i = 0;i < s->args.size();++i) {
                    OS << s->args[i];
                    if (i != s->args.size())
                        OS << ", ";
                }
                OS << ")" << NL << "{" << NL;
                PrintStmt(s->funcbody);
                OS << NL << "}" << NL;
                return;
            default: break;
        }
    }
};
void dumpAst(Stmt s, enum AST_Dump_Format format = AST_Default, llvm::raw_ostream &OS = llvm::errs()) {
    if (format == AST_Default)
        return AstDumper(OS).dump(s);
    llvm_unreachable("json not supported now!");
}
