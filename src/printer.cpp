// TODO: someone can help me inprove spellings for my poor english

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
    for (const char c : str) {
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
                char buf[4] = {'\\', static_cast<char>('0' + ((static_cast<unsigned>(c) >> 6) & 7)),
                               static_cast<char>('0' + ((static_cast<unsigned>(c) >> 3) & 7)),
                               static_cast<char>('0' + ((static_cast<unsigned>(c) >> 0) & 7))};
                OS << buf;
            }
        }
    }
END:
    OS.resetColor();
    OS << '"';
}

raw_ostream &operator<<(llvm::raw_ostream &, const_CType);

raw_ostream &printTag(const_CType ty, raw_ostream &OS) {
    IdentRef Name = ty->getTagName();
    return OS << show(ty->getTagDeclTagType()) << " " << (Name ? Name->getKey() : "<anonymous>");
}

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
        for (unsigned i = 0; i < max; ++i) {
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
            for (unsigned i = 0; i < max; ++i) {
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
            for (unsigned i = 0; i < max; ++i) {
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
            for (unsigned i = 0; i < max; ++i) {
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

raw_ostream &operator<<(llvm::raw_ostream &OS, const_Expr e);

void maybe_print_paren(const_Expr e, raw_ostream &OS) {
    if (e->ty->hasTag(TYPAREN)) {
        OS << '(' << e << ')';
        return;
    }
    OS << e;
}
raw_ostream &operator<<(llvm::raw_ostream &OS, const_Expr e) {
    switch (e->k) {
    case EBlockAddress:
        return OS << "&&" << e->labelName->getKey();
    case EConstantArraySubstript: printConstant(e->carray->getInitializer(), OS); return OS;
    case EConstantArray: printConstant(e->array, OS); return OS;
    case EConstant: printConstant(e->C, OS); return OS;
    case EBin: 
        maybe_print_paren(e->lhs, OS);
        OS << ' ' << show(e->bop) << ' ';
        maybe_print_paren(e->rhs, OS);
        return OS;
    case EUnary:
        if (e->uop == AddressOf && e->ty->p->getKind() == TYFUNCTION && !e->ty->hasTag(TYLVALUE))
            return maybe_print_paren(e->uoperand, OS), OS;
        OS << show(e->uop);
        maybe_print_paren(e->uoperand, OS);
        return OS;
    case EVoid: return maybe_print_paren(e->voidexpr, OS), OS;
    case EVar: {
        OS.changeColor(raw_ostream::MAGENTA);
        OS << e->varName->getKey();
        OS.resetColor();
        return OS;
    }
    case ECondition:
        maybe_print_paren(e->cond, OS);
        OS << " ? ";
        maybe_print_paren(e->cleft, OS);
        OS << " : ";
        maybe_print_paren(e->cright, OS);
        return OS;
    case ECast: return OS << '(' << raw_ostream::BLUE << e->ty << raw_ostream::RESET << ')';maybe_print_paren(e->castval, OS);return OS;
    case ESizeof: return OS << "sizeof(" << raw_ostream::BLUE << e->theType << raw_ostream::RESET << ')';
    case ECall:
        maybe_print_paren(e->callfunc, OS);
        OS << '(';
        if (e->callargs.empty())
            return OS << ')';
        for (size_t i = 0; i < e->callargs.size() - 1; ++i) {
            maybe_print_paren(e->callargs[i], OS);
            OS << ", ";
        }
        maybe_print_paren(e->callargs.back(), OS);
        return OS << ')';
    case ESubscript: 
        maybe_print_paren(e->left, OS);
        OS << '[';
        maybe_print_paren(e->right, OS);
        return OS << ']';
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
    case EMemberAccess: return maybe_print_paren(e->obj, OS), OS << '.' << e->ty->getRecord()->fields[e->idx].name;
    case EArrToAddress: return maybe_print_paren(e->arr3, OS), OS;
    case EPostFix: 
        maybe_print_paren(e->poperand, OS);
        return OS << show(e->pop);
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
    case TYTAG: return printTag(ty, OS);
    case TYBITFIELD: return OS << ty->bittype << " : " << ty->bitsize;
    case TYARRAY:
        OS << ty->arrtype << " [";
        if (ty->hassize)
            OS << ty->arrsize;
        return OS << ']';
    case TYVLA: {
        Expr vla_expr = ty->vla_expr;
        if (vla_expr->k == ECast)
            vla_expr = vla_expr->castval;
        return OS << ty->vla_arraytype << " [" << vla_expr << ']';
    }
    case TYBITINT: return OS << ty->getBitIntBaseType() << " _BitInt(" << ty->getBitIntBits() << ")";
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
    }
    llvm_unreachable("");
}

raw_ostream &operator>>(raw_ostream &OS, const_CType ty) {
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
        OS << "pointer[base=" >> ty->p;
        if (!str.empty())
            OS << ", qualifiers=" << str;
        return OS << "]";
    }
    case TYTAG: return printTag(ty, OS);
    case TYBITFIELD: return OS >> ty->bittype << " : " << ty->bitsize;
    case TYARRAY:
        OS << "array[base=" >> ty->arrtype << ", size=";
        if (ty->hassize)
            OS << ty->arrsize;
        return OS << "]";
    case TYVLA: {
        Expr vla_expr = ty->vla_expr;
        if (vla_expr->k == ECast)
            vla_expr = vla_expr->castval;
        OS << "VLA[base=" >> ty->vla_arraytype << ", size=" << vla_expr << "]";
    } break;
    case TYBITINT:
        OS << "BitInt[base=";
        OS << ty->getBitIntBaseType();
        OS << ", bits=";
        OS << ty->getBitIntBits();
        return OS << "]";
    case TYFUNCTION: {
        auto str = ty->get_storage_str();
        OS << "Function[ret=" >> ty->ret;
        if (ty->hasTag(TYINLINE))
            OS << "inline" << ' ';
        if (ty->hasTag(TYNORETURN))
            OS << "_Noreturn" << ' ';
        if (str.size())
            OS << str << ' ';
        OS << ", params=[";
        if (!ty->params.empty()) {
            for (const auto &e : ty->params) {
                OS >> e.ty;
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
    case TYTAG: printTag(ty, OS); break;
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
    case TYVLA: {
        Expr vla_expr = ty->vla_expr;
        if (vla_expr->k == ECast)
            vla_expr = vla_expr->castval;
        OS << "variable length array " << vla_expr << " of ";
        print_cdecl(ty->vla_arraytype, OS);
    } break;
    case TYBITINT:
        OS << "bit int ";
        print_cdecl(ty->getBitIntBaseType(), OS);
        OS << " with " << ty->getBitIntBits() << " bits";
        break;
    case TYFUNCTION: {
        auto str = ty->get_storage_str();
        OS << "function (";
        if (!ty->params.empty()) {
            size_t end = ty->params.size() - 1;
            for (size_t i = 0; i <= end; ++i) {
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
    }
}
void print_cdecl(StringRef name, const_CType ty, raw_ostream &OS = llvm::errs(), bool addNewLine = false) {
    OS << "declare " << name << " as ";
    print_cdecl(ty, OS);
    if (addNewLine)
        OS << '\n';
}

enum AST_Dump_Format {
    AST_Default,
    AST_JSON
};
template <bool Recursive = true>
struct AstDumper {
    llvm::raw_ostream &OS;
    StringRef NL;
    StringRef tab;
    unsigned IndentLevel;
    unsigned Indentation;
    AstDumper(llvm::raw_ostream &OS, StringRef NL = "\n", StringRef tab = "    ", unsigned IndentLevel = 0,
              unsigned Indentation = 2)
        : OS{OS}, NL{NL}, tab{tab}, IndentLevel{IndentLevel}, Indentation{Indentation} { }
    void printExpr(Expr e) {
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
    void PrintStmt(const_Stmt s) {
        IndentLevel += Indentation;
        Indent();
        dump(s);
        IndentLevel -= Indentation;
    }
    inline void newline() {
        OS << NL;
    }
    void dump(const_Stmt s) {
        switch (s->k) {
        case SHead:
            if (Recursive) {
                for (const_Stmt ptr = s->next; ptr; ptr = ptr->next)
                    dump(ptr);
            } else {
                OS << "translation-unit";
            }
            return;
        case SCompound:
            OS << "{";
            newline();
            IndentLevel += Indentation;
            for (const_Stmt ptr = s->inner; ptr; ptr = ptr->next)
                PrintStmt(ptr);
            IndentLevel -= Indentation;
            Indent();
            OS << "}";
            newline();
            return;
        case SNamedLabel:
            OS << "%" << s->label << ':';
            OS << " (" << s->labelName->getKey() << ")";
            newline();
            return;
        case SLabel:
            OS << "%" << s->label << ':';
            newline();
            return;
        case SGotoWithLoc:
        case SGotoWithLocName:
        case SGoto:
            OS.changeColor(raw_ostream::RED);
            OS << "goto";
            OS.resetColor();
            OS << " %" << s->location << ';';
            newline();
            return;
        case SCondJump:
            OS.changeColor(raw_ostream::RED);
            OS << "branch ";
            OS.resetColor();
            printExpr(s->test);
            OS << " %" << s->T << ", %" << s->F << ';';
            newline();
            return;
        case SSwitch: {
            bool isSigned = s->itest->ty->isSigned();
            OS.changeColor(raw_ostream::RED);
            OS << "switch";
            OS.resetColor();
            OS << s->itest;
            newline();
            for (const SwitchCase &it : s->switchs) {
                OS << raw_ostream::RED << "  case " << raw_ostream::RESET;
                it.CaseStart->print(OS, isSigned);
                OS << " => %" << it.label;
            }
            for (const GNUSwitchCase &it : s->gnu_switchs) {
                OS << raw_ostream::RED << "  case " << raw_ostream::RESET;
                it.CaseStart->print(OS, isSigned);
                OS << " ... ";
                (*it.CaseStart + it.range).print(OS, isSigned);
                OS << " => %" << it.label;
            }
            OS << raw_ostream::RED << "  default" << raw_ostream::RESET << " =>" << s->sw_default;
            newline();
        }
            return;
        case SReturn:
            OS.changeColor(raw_ostream::RED);
            OS << "return";
            OS.resetColor();
            if (s->ret) {
                OS << ' ';
                printExpr(s->ret);
            }
            OS << ';';
            newline();
            return;
        case SExpr:
            printExpr(s->exprbody);
            OS << ';';
            newline();
            return;
        case SNoReturnCall:
            OS << "(noreturn call) ";
            printExpr(s->call_expr);
            OS << ';';
            newline();
            return;
        case SAsm:
            OS.changeColor(raw_ostream::RED);
            OS << "__asm__";
            OS.resetColor();
            OS << "(";
            printCString(OS, s->asms.str().drop_back());
            OS << ')';
            newline();
            return;
        case SVarDecl: {
            size_t end = s->vars.size();
            for (size_t i = 0; i < end; ++i) {
                const VarDecl &it = s->vars[i];
                printCType(it.ty);
                OS << " " << raw_ostream::MAGENTA << it.name->getKey() << raw_ostream::RESET;
                if (it.init) {
                    OS << " = ";
                    printExpr(it.init);
                }
                if (i == end) {
                    OS << ";";
                    newline();
                } else {
                    OS << ',';
                    newline();
                    Indent();
                }
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
            {
                size_t Size = s->functy->params.size();
                OS << " (";
                for (size_t i = 0; i < Size; ++i) {
                    OS << s->localStart + i;
                    if (i != Size - 1)
                        OS << ", ";
                }
                OS << ")" << NL << "{" << NL;
                OS << "{";
                newline();
                IndentLevel += Indentation;
                for (const_Stmt ptr = s->funcbody->next; ptr; ptr = ptr->next)
                    PrintStmt(ptr);
                IndentLevel -= Indentation;
                Indent();
                OS << "}";
                newline();
                OS << NL << "}" << NL;
            } return;
        default: break;
        }
    }
};
/*
struct AstJSONDumper {
    llvm::json::OStream &JOS;
    AstDumper(llvm::raw_ostream &OS, unsigned IndentSize = 0)
        : JOS{OS, OS} {}
    llvm::json::Value printExpr(Expr e, StringRef attrName = "expr") {
        SmallString<64> str;
        raw_svector_ostream OS{str};
        return str;
    }
    llvm::json::Value printCType(CType ty, StringRef attrName = "type") {
        SmallString<64> str;
        raw_svector_ostream OS{str};
        OS << ty;
        return str;
    }
    void dump(const_Stmt s) {
        switch (s->k) {
        case SHead:
            JOS.attributeBegin("body");
            JOS.arrayBegin();
            for (const_Stmt ptr = s->next; ptr; ptr = ptr->next)
                dump(ptr);
            JOS.arrayEnd();
            JOS.attributeEnd();
            return;
        case SCompound:
            OS << "{";
            JOS.arrayBegin();
            for (const_Stmt ptr = s->inner; ptr; ptr = ptr->next)
                dump(ptr);
            JOS.arrayEnd();
            return;
        case SNamedLabel:
            JOS.value("LabelStmt", llvm::json::Object(
                {
                    {"label", s->label},
                    {"labelName", s->labelName->getKey()}
                }
            ));
            return;
        case SLabel:
            JOS.value("LabelStmt", llvm::json::Object(
                {
                    {"label", s->label}
                }
            ));
            return;
        case SGotoWithLoc:
        case SGotoWithLocName:
        case SGoto:
            JOS.value("GotoStmt", llvm::json::Object(
                {
                    {"location", s->location}
                }
            ));
            return;
        case SCondJump:
            JOS.value("CondJumpStmt", llvm::json::Object(
                {
                    {"test", printExpr(s->test)},
                    {"T", s->T},
                    {"F", s->F}
                }
            ));
            return;
        case SSwitch: {
            return;
        case SReturn:
            auto V = llvm::json::Object(
                {"kind", "ReturnStmt"}
            );
            if (s->ret)
                V.insert("ret", printExpr(s->ret));
            JOS.value(V);
            return;
        case SExpr:
            JOS.value(llvm::json::Object(
                {"kind", "ExprStmt"},
                {"expr", printExpr(s->exprbody)}
            ));
            return;
        case SNoReturnCall:
            JOS.value(llvm::json::Object(
                {"kind", "NoReturnCallStmt"},
                {"expr", printExpr(s->call_expr)}
            ));
            return;
        case SAsm:
            JOS.value(llvm::json::Object(
                {"kind", "AsmStmt"},
                {"expr", s->asms}
            ));
            return;
        case SVarDecl: 
            return;
        case SFunction:
            JOS.valueBegin();
            JOS.objectBegin();
            JOS.objectEnd();
            JOS.attribute("Name", s->funcname->getKey());
            JOS.attribute("Type", printCType(s->functy));
            {
                JOS.attributeBegin("params");
                for (size_t i = 0; i < s->args.size(); ++i) {
                    OS << s->args[i];

                }
                JOS.attributeEnd();
                JOS.attributeBegin("Bbdy");
                JOS.arrayBegin();
                for (const_Stmt ptr = s->funcbody->next; ptr; ptr = ptr->next)
                    dump(ptr);
                JOS.arrayEnd();
                JOS.attributeEnd();
            }
            JOS.valueEnd(); 
            return;
        default: break;
        }
    }
};
*/
void dumpAst(const_Stmt s, enum AST_Dump_Format format = AST_Default, llvm::raw_ostream &OS = llvm::errs()) {
    if (format == AST_Default)
        return AstDumper<true>(OS).dump(s);
    // return AstJSONDumper(OS).dump(s);
}
