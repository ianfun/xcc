void TextDiagnosticPrinter::printSource(Location loc) {

#if WINDOWS
    LARGE_INTEGER saved_posl;
#else
    off_t saved_posl;
#endif
    Stream saved_stream = SM.streams[loc.id];

    if (saved_stream.k != AFileStream)
        return;

    Stream &stream_ref = SM.streams[loc.id];
    
    bool shouldSeek = saved_stream.line >= loc.line;

    if (shouldSeek) {
#if WINDOWS
        if (!::SetFilePointerEx(saved_stream.fd, LARGE_INTEGER(), &saved_posl, FILE_CURRENT))
            return;

        if (!::SetFilePointerEx(saved_stream.fd, LARGE_INTEGER(), nullptr, FILE_BEGIN))
            return;
#else
        if ((saved_posl = ::lseek(saved_stream.fd, 0, SEEK_CUR)) < 0)
            return;

        if (::lseek(saved_stream.fd, 0, SEEK_SET) < 0)
            return;
#endif

        stream_ref.p = 0;
        stream_ref.pos = 0;
        stream_ref.line = 1;
        stream_ref.column = 1;
    }
    // saved_stream.p = 0;
    // saved_stream.pos = 0;

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
        unsigned line_chars = 0;
        for(;;) {
            char c = SM.raw_read_from_id(loc.id);
            if (c == '\0' || c == '\n' || c == '\r') 
                break;
            bool cond = line_chars == loc.col;
            if (cond)
                OS.changeColor(raw_ostream::RED);
            if (c == '\t' || isprint((unsigned char)c)) {
                OS << c;
            } else {
                (OS << "<0x").write_hex((unsigned char)c) << '>';
            }
            if (cond)
                OS.resetColor();
            ++line_chars;
        }
        OS << "\n      | ";
        for (unsigned i = 0;i < loc.col;i++)
            OS << ' ';
        OS.changeColor(raw_ostream::RED);
        OS << '^';
        OS.resetColor();
        OS << '\n';
    }
    EXIT:
    SM.streams[loc.id] = saved_stream;
    if (shouldSeek) {
#if WINDOWS
        ::SetFilePointerEx(saved_stream.fd, saved_posl, nullptr, FILE_BEGIN);
#else
        ::lseek(saved_stream.fd, saved_posl, SEEK_SET);
#endif
    }
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
