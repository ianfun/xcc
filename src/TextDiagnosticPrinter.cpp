void TextDiagnosticPrinter::printSource(Location loc) {
#if WINDOWS
    LARGE_INTEGER saved_posl;
#else
    off_t saved_posl;
#endif
    Stream &stream_ref = SM.streams[loc.id];
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
        char lastc, c = '\0';
        while (stream_ref.line < loc.line) {
            lastc = c;
            c = SM.raw_read_from_id(loc.id);
            if (c == '\r') {
                ++stream_ref.line;
            } else if (c == '\n') {
                if (lastc != '\r')
                    ++stream_ref.line;
            } else if (c == '\0') {
                dbgprint("unexpected EOF!");
                goto EXIT;
            }
        }
        {
            char buf[13];
            int len = snprintf(buf, sizeof(buf), "%5d", loc.line);
            OS << StringRef(buf, (unsigned)len) << " | ";
        }
        unsigned line_chars = 0, displayed_chars = 0, real = 0;

        for(;;) {
            unsigned char c = SM.raw_read_from_id(loc.id);
            if (c == '\0' || c == '\n' || c == '\r') {
                if (real == 0)
                    real = displayed_chars;
                break;
            }
            bool cond = line_chars == loc.col;
            if (cond) {
                OS.changeColor(raw_ostream::RED);
                real = displayed_chars;
            }
            if (c == '\t') {
                displayed_chars += CC_SHOW_TAB_SIZE;
                for (unsigned i = 0;i < CC_SHOW_TAB_SIZE;++i)
                    OS << ' ';
            } else if (/*std::iscntrl(c)*/ !llvm::isPrint(c)) {
                OS << "<0x"
                   << (hexed(c) >> 4)
                   << hexed(c)
                   << '>';
                   displayed_chars += 6;
            } else {
                displayed_chars++;
                OS << c;
            }
            if (cond)
                OS.resetColor();
            ++line_chars;
        }
        OS << "\n      | ";
        {
            const std::string buffer(real, ' ');
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
void TextDiagnosticPrinter::realHandleDiagnostic(enum DiagnosticLevel level, const Diagnostic &Info) {
    bool locValid = Info.loc.isValid();
    if (locValid) {
        OS << SM.getFileName(Info.loc.id) << ':' << Info.loc.line << ':' << Info.loc.col << ": ";
    } else {
        OS << "xcc: ";
    }
    if (ShowColors) {
        raw_ostream::Colors color;
        switch (level) {
            case Note: color = noteColor; break;
            case Remark: color = remarkColor; break;
            case Warning: color = warningColor; break;
            default: color = errorColor; break;
        }
        OS.changeColor(color);
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
            case EvalError: msgtype = "evaluation error: ";break;
            case TypeError: msgtype = "type error: "; break;
            case Fatal: msgtype = "fatal error: "; break;
        }
        OS << msgtype;
    }
    if (ShowColors)
        OS.resetColor();
    {
        SmallString<100> OutStr;
        Info.FormatDiagnostic(OutStr);
        OutStr.push_back('\n');
        OS << OutStr.str();
    }
    if (locValid)
        printSource(Info.loc);
    // OS.flush()
}
