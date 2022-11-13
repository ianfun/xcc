void TextDiagnosticPrinter::printSource(LocationBase loc) {
#if WINDOWS
    LARGE_INTEGER saved_posl;
#else
    off_t saved_posl;
#endif
    Stream &stream_ref = SM->streams[loc.id];
    if (stream_ref.k != AFileStream)
        return;
    uint32_t old_line = stream_ref.line;
    uint32_t old_col = stream_ref.column;
#if WINDOWS
    LARGE_INTEGER seek_pos;
    seek_pos.QuadPart = -static_cast<ULONGLONG>(stream_ref.pos);
    if (!::SetFilePointerEx(stream_ref.fd, seek_pos, &saved_posl, FILE_CURRENT))
        return;
    seek_pos.QuadPart = 0;
    ::SetFilePointerEx(stream_ref.fd, seek_pos, nullptr, FILE_BEGIN);
#else
    if ((saved_posl = ::lseek(stream_ref.fd, -static_cast<off_t>(stream_ref.pos), SEEK_CUR)) < 0)
        return;
    ::lseek(stream_ref.fd, 0, SEEK_SET);
#endif
    stream_ref.pos = 0;
    stream_ref.line = 1;
    stream_ref.column = 1;
    {
        while (stream_ref.line < loc.line) {
            char c = SM->raw_read_from_id(loc.id);
            if (c == '\r') {
                ++stream_ref.line;
                c = SM->raw_read_from_id(loc.id);
                if (c != '\n')
                    ++stream_ref.line;
            } else if (c == '\n') {
                ++stream_ref.line;
            } else if (c == '\0') {
                dbgprint("ERROR: unexpected EOF!");
                goto EXIT;
            }
        }
        {
            char buf[13];
            int len = snprintf(buf, sizeof(buf), "%5d", loc.line);
            OS << StringRef(buf, (unsigned)len) << " | ";
        }
        unsigned line_chars = 0, displayed_chars = 0, real = 0;
        {
            llvm::SmallString<64> buffer;
            for (;;) {
                unsigned char c = SM->raw_read_from_id(loc.id);
                if (c == '\0' || c == '\n' || c == '\r') {
                    buffer += "\n      | ";
                    OS << buffer.str();
                    if (real == 0)
                        real = displayed_chars;
                    break;
                }
                bool cond = line_chars == loc.col;
                if (cond) {
                    OS << buffer.str();
                    buffer.clear();
                    OS.changeColor(raw_ostream::RED);
                    real = displayed_chars;
                }
                if (c == '\t') {
                    displayed_chars += CC_SHOW_TAB_SIZE;
                    buffer.assign(CC_SHOW_TAB_SIZE, ' ');
                } else if (!llvm::isPrint(c)) { /*std::iscntrl(c)*/ 
                    buffer.push_back('<');
                    buffer.push_back('0');
                    buffer.push_back('x');
                    buffer.push_back(hexed(c) >> 4);
                    buffer.push_back(hexed(c));
                    buffer.push_back('>');
                    displayed_chars += 6;
                } else {
                    displayed_chars++;
                    buffer.push_back(c);
                }
                if (cond)
                    OS.resetColor();
                ++line_chars;
            }
        }
        {
            const SmallVector<char> buffer(real, ' ');
            OS << buffer;
        }
        OS.changeColor(raw_ostream::RED);
        OS << '^';
        OS.resetColor();
        OS << '\n';
    }

EXIT:
    stream_ref.pos = 0;
    stream_ref.line = old_line;
    stream_ref.column = old_col;
#if WINDOWS
    ::SetFilePointerEx(stream_ref.fd, saved_posl, nullptr, FILE_BEGIN);
#else
    ::lseek(stream_ref.fd, saved_posl, SEEK_SET);
#endif
}
static void write_loc(raw_ostream &OS, const LocationBase &loc, SourceMgr *SM) {
    OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, true);
    OS << SM->getFileName(loc.id) << ':' << loc.line << ':' << loc.col << ": ";
    OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, false);
}
void TextDiagnosticPrinter::realHandleDiagnostic(enum DiagnosticLevel level, const Diagnostic &Info) {
    LocTree *begin_macro = Info.loc.getParent();
    if (SM) { 
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
        if (Info.loc.isValid()) {
            write_loc(OS, Info.loc, SM);
            goto OK;
        }
    }
    OS << "xcc: ";
    OK:
    {
        raw_ostream::Colors color;
        switch (level) {
        case Note: color = noteColor; break;
        case Remark: color = remarkColor; break;
        case Warning: color = warningColor; break;
        default: color = errorColor; break;
        }
        OS.changeColor(color, true);
    }
    {
        StringRef msgtype;
        switch (level) {
        case Ignored: llvm_unreachable("");
        case Note: msgtype = "note: "; break;
        case Remark: msgtype = "remark: "; break;
        case Warning: msgtype = "warning: "; break;
        case Error: msgtype = "error: "; break;
        case PPError: msgtype = "preprocessor error: "; break;
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
        Info.FormatDiagnostic(OutStr);
        OutStr.push_back('\n');
        OS << OutStr.str();
    }
    if (SM) {
        if (Info.loc.isValid()) {
            printSource(Info.loc);
            for (LocTree *ptr = begin_macro;ptr;ptr = ptr->getParent()) {
                PPMacroDef *def = ptr->macro;
                OS.changeColor(noteColor); 
                OS << "note: ";
                OS.resetColor();
                OS << "in expansion of macro ";
                OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, true);
                OS << def->m.Name->getKey();
                OS.changeColor(llvm::raw_ostream::Colors::SAVEDCOLOR, false);
                OS << '\n';
                printSource(def->loc); // print where the macro is defined
            }
        }
    }
}
