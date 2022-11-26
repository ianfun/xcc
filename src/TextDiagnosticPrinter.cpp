void TextDiagnosticPrinter::printSource(const source_location &loc) {
    OS << loc.source_line.str();
}
void TextDiagnosticPrinter::write_loc(const source_location &loc) {
    OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, true);
    OS << SM->getFileName(loc.fd) << ':' << loc.line << ':' << loc.col << ": ";
    OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, false);
}
void TextDiagnosticPrinter::realHandleDiagnostic(const Diagnostic &Diag) {
    bool locValid = false;
    LocTree *begin_macro = nullptr;
    source_location loc;
    if (SM) { 
        locValid = SM->translateLocation(Diag.loc, loc);
        if (!locValid)
            goto NO_LOC;
        begin_macro = loc.tree;
        LocTree *ptr = begin_macro;
        if (ptr && ptr->isAInclude) {
            for (;ptr && ptr->isAInclude;ptr = ptr->getParent()) {
                auto it = ptr->include;
                OS << StringRef(ptr == begin_macro ? "In file included from " : "                      ", 22);
                OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, true);
                OS << SM->getFileName(it->fd) << ':' << it->line;
                OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, false);
                OS << ":\n";
            }
        }
        begin_macro = ptr;
        write_loc(loc);
    } else {
        NO_LOC:
        OS << "xcc: ";
    }
    {
        raw_ostream::Colors color;
        switch (Diag.level) {
        case Note: color = noteColor; break;
        case Remark: color = remarkColor; break;
        case Warning: color = warningColor; break;
        default: color = errorColor; break;
        }
        OS.changeColor(color, true);
    }
    {
        StringRef msgtype;
        switch (Diag.level) {
        case Ignored: llvm_unreachable("");
        case Note: msgtype = "note: "; break;
        case Remark: msgtype = "remark: "; break;
        case Warning: msgtype = "warning: "; break;
        case Error: msgtype = "error: "; break;
        case PPError: msgtype = "cpp error: "; break;
        case LexError: msgtype = "lex error: "; break;
        case ParseError: msgtype = "parse error: "; break;
        case EvalError: msgtype = "evaluation error: "; break;
        case TypeError: msgtype = "type error: "; break;
        case Fatal: msgtype = "fatal error: "; break;
        }
        OS << msgtype;
    }
    OS.resetColor();
    {
        SmallString<64> OutStr;
        Diag.FormatDiagnostic(OutStr);
        OutStr.push_back('\n');
        OS << OutStr.str();
    }
    if (locValid) {
        printSource(loc);
        for (LocTree *ptr = begin_macro;ptr;ptr = ptr->getParent()) {
            PPMacroDef *def = ptr->macro;
            SM->translateLocation(def->loc, loc);
            write_loc(loc);
            OS.changeColor(noteColor, true);
            OS << "note: ";
            OS.resetColor();
            OS << "in expansion of macro ";
            OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, true);
            OS << def->m.Name->getKey();
            OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, false);
            OS << '\n';
            printSource(loc); // print where the macro is defined
        }
    }
}
