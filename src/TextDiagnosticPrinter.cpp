void TextDiagnosticPrinter::printSource(source_location &loc, const ArrayRef<FixItHint> FixItHints = {}) {
    {
        char buf[13];
        int len = snprintf(buf, sizeof(buf), "%5d", loc.line);
        OS << StringRef(buf, len) << " | ";
    }
    OS << loc.source_line.sourceLine.str();
    OS << "\n      | ";
    while (loc.source_line.CaretLine.size() && loc.source_line.CaretLine.back() == ' ')
        loc.source_line.CaretLine.pop_back();
    if (loc.source_line.CaretLine.size()) {
        for (const char c : loc.source_line.CaretLine) {
            if (c == '~' || c == '^')
                OS.changeColor(llvm::raw_ostream::Colors::GREEN);
            OS << c;
            if (c == '~' || c == '^')
                OS.resetColor();
        }
    }
    if (!FixItHints.empty()) {
        std::string line = loc.source_line.buildFixItInsertionLine(FixItHints);
        OS << "\n        " << line;
    }
    OS << '\n';
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
    if (SM && Diag.loc != 0) {
        locValid = SM->translateLocation(Diag.loc, loc, Diag.ranges);
        if (!locValid)
            goto NO_LOC;
        begin_macro = loc.tree;
        LocTree *ptr = begin_macro;
        for (; ptr && ptr->isAInclude; ptr = ptr->getParent()) {
            auto it = ptr->include;
            OS << StringRef(ptr == begin_macro ? "In file included from " : "                      ", 22);
            OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, true);
            OS << SM->getFileName(it->fd) << ':' << it->line;
            OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, false);
            OS << ":\n";
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
        printSource(loc, Diag.FixItHints);
        for (LocTree *ptr = begin_macro; ptr; ptr = ptr->getParent()) {
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
