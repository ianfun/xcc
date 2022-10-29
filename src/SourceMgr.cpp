/* SourceMgr.cpp - Source Code manager */

/*
Translation phases
1. Multibyte characters are mapped to the source character set, introducing new-line characters and end-of-line indicators.Trigraphs sequences are replaced by single character.
2. The `'\' newline` is deleted
3. Source file is decomposed into preprocessing tokens, comment replaced by one space character
4. Preprocessing directives are executed, _Pragma are executed.A #include directives causes translation phase 1 through phase 4, recursively.
5. Convert escaped string literals to the execution charset.
6. Concat string literals.
7. Ignore white spaces, preprocessing is converted into token.The resulting tokens are syntactically and semantically analyzed and translated as a translation unit.
8. resolve external object and functions...
*/

#if WINDOWS
  typedef HANDLE fd_t;
#else
  typedef int fd_t;
#endif

enum StreamType: uint8_t {
    AFileStream, AStringStream, AStdinStream
};

struct Stream /*FileStream*/ {
    const char *name; // maybe any of <stdin>, <built-in>, <string>, ../x.c, /x.c, ~/x.c
    uint32_t line = 1, column = 1;
    enum StreamType k;
    union {
        struct {
            fd_t fd;
            unsigned p, pos;
        };
        struct /*StdinStream*/ {
            unsigned i;
    #if WINDOWS
            DWORD readed;
    #else
            unsigned readed;
    #endif
        };
        struct /*StringStream*/{
            unsigned idx, max;
            const char *s;
        };
    };
};

struct SourceMgr: public DiagnosticHelper {
    llvm::SmallString<STREAM_BUFFER_SIZE> buf;
    SmallVector<const char*> sysPaths;
    SmallVector<xstring> userPaths;
    SmallVector<Stream> streams;
    SmallVector<fileid_t> includeStack;
    uint32_t getLine() {
        return streams[includeStack.back()].line;
    }
    void setLine(uint32_t line) {
        streams[includeStack.back()].line = line;
    }
    const char *getFileName(unsigned i) {
        return streams[i].name;
    }
    const char *getFileName() {
        return streams[includeStack.back()].name;
    }
    void setFileName(const char *name) {
        streams[includeStack.back()].name = name;
    }
    Location getLoc() {
        const Stream &f = streams[includeStack.back()];
        return Location {.line = (line_t)f.line, .col = (column_t)f.column, .id = (fileid_t)includeStack.back()};
    }
    bool is_tty;
    SourceMgr(xcc_context &context): DiagnosticHelper{context}, buf{} {
        buf.resize_for_overwrite(STREAM_BUFFER_SIZE);
#if WINDOWS
        DWORD dummy;
        is_tty = GetConsoleMode(hStdin, &dummy) != 0;
#else
        is_tty = isatty(STDIN_FILENO) != 0;
#endif
    }
    void addUsernIcludeDir(xstring path) {
        userPaths.push_back(path);
    }
    void addSysIncludeDir(const char *path) {
        sysPaths.push_back(path);
    }
    bool addFile(const char *path, bool verbose = true) {
#if WINDOWS
        llvm::SmallVector<WCHAR, 128> convertBuffer;
        if (llvm::sys::windows::CurCPToUTF16(path, convertBuffer))
            return false;
        HANDLE fd = ::CreateFileW(convertBuffer.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fd == INVALID_HANDLE_VALUE) {
            if (verbose)
                error("cannot open %s: %w", path, (uint64_t)GetLastError());
            return false;
        }
#else
        int fd = ::open(path, O_RDONLY);
        if (fd < 0) {
            if (verbose)
                error("cannot open %s: %o", path, errno);
            return false;
        }
        /*{
            struct stat sb;
            if (fstat(fd, &sb))
                return error("fstat: %o", errno), false;
            if (!S_ISREG(sb.st_mode)) {
                pp_error("not a regular file: %s", path);
                return false;
            }
        }*/
#endif
        Stream f = Stream {.name = path, .k = AFileStream};
        f.p = 0;
        f.pos = 0;
        f.fd = fd;
        includeStack.push_back(streams.size());
        streams.push_back(f);
        return true;
    }
    bool addFile(xstring str) {
        bool res;
        assert(str.size() && str.back() == '\0');
        res = addFile(str.data(), false);
        str.free();
        return res;
    }
    // read a char from a stream
    LLVM_NODISCARD char raw_read(Stream &f) {
        switch (f.k) {
            case AFileStream:
                {
                AGAIN:
                    if (f.pos == 0) {
                #if WINDOWS
                        DWORD L = 0;
                        if (!::ReadFile(f.fd, buf.data(), STREAM_BUFFER_SIZE, &L, nullptr)){
                            return '\0';
                        }
                        if (L == 0)
                            return '\0';
                #else
                    READ_AGAIN:
                        ssize_t L = ::read(f.fd, buf.data(), buf.size());

                        if (L == 0)
                            return '\0';

                        if (L < 0) {
                            if (errno == EINTR)
                                goto READ_AGAIN;
                            pp_error("error reading file: %o", errno);
                            return '\0';
                        }
                #endif
                        f.pos = L;
                        f.p = 0;
                    }
                    char c = buf[f.p++];
                    f.pos--;
                    if (c)
                        return c;
                    warning("null character(s) ignored");
                    goto AGAIN;
                }
            case AStdinStream:
            {
                if (f.i >= f.readed)
                {
                    READ:
            #if CC_NO_RAEADLINE
              #if WINDOWS /* Windows */
                    SAGAIN:
                    if (is_tty) {
                        ::WriteConsoleW(hStdin, WSTDIN_PROMPT, cstrlen(WSTDIN_PROMPT), nullptr, nullptr);
                        if (::ReadConsoleW(hStdin, reinterpret_cast<PWCHAR>(buf.data()), buf.size() / 2, &f.readed, nullptr) == 0){
                            return '\0';
                        }
                        for(DWORD i = 0;i<f.readed;f.i++){
                            // ^D = 4
                            // ^X = 24
                            // ^Z = 26
                            if (buf[i] == 26 || buf[i] == 4) {
                                return '\0';
                            }
                        }
                    }else{
                        if (!::ReadFile(hStdin, buf.data(), buf.size(), &f.readed, nullptr)){
                            return '\0';
                        }
                    }
                    if (f.readed == 0)
                        goto SAGAIN;
              #else /* Unix */
                    char c;
                    ssize_t L;
                    for(unsigned idx = 0;;) {
                        L = ::read(STDIN_FILENO, &c, 1);
                        if (L <= 0){
                            if (errno == EINTR)
                                continue;
                            if (L)
                                pp_error("reading from stdin: %o", errno);
                            return '\0';
                        }
                        buf[idx++] = c;
                        if (c == '\n'){
                            f.readed = idx;
                            break;
                        }
                    }
              #endif
            #else /* GNU Readline */
                    REPEAT:
                    buf.clear();
                    char* line = ::readline(STDIN_PROMPT);
                    if (!line)
                        return '\0';
                    if (!*line)
                        goto REPEAT;
                    ::add_history(line);
                    {
                        StringRef line_str = StringRef(line);
                        buf += line_str;
                        ::free(line_str);
                        f.readed = line_str.size();
                    }
            #endif
            
            
                    if (buf[0] == ':'){
                        enum Command command = NotACommand;
            
            #if WINDOWS
                        TinyCommandParser<WCHAR> P(reinterpret_cast<PWCHAR>(buf.data()+1), f.readed-1);
                        std::wstring str;
            #else
                        TinyCommandParser<char> P(buf.data()+1, f.readed);
                        std::string str;
            #endif
                        if (P.isChar('?')) {
                            command = Command::Help;
                        } else {
                            P.readIdent(str);
                            if (str.empty()){
                                P.raw("cc: no command given.try ':help'\n");
                                goto READ;
                            }
                        }
            #if WINDOWS
                #define XCMD(s) L##s
            #else
                #define XCMD(s) s
            #endif
                        if (str == XCMD("quit")) command = Command::Quit;
                        else if (str == XCMD("help")) command = Command::Help;
                        else if (str == XCMD("quit")) command = Command::Quit;
                        switch(command){
                            case Command::NotACommand:
#if WINDOWS
                                P.fmt("cc: not a command '%s'\n", str.data());
#else
                                P.fmt("cc: not a command '%L'\n", str.data());
#endif
                                break;
                            case Command::Quit:
                                llvm::llvm_shutdown();
                                exit(CC_EXIT_SUCCESS);
                            case Command::Help:
                                P.raw(
                                    "cc command help\n"
                                    "Commands is always follows by a ':':, for example: ':quit', and may has arguments after command"
                                    "\nAvailable commands:\n"
                                    "   help:, ':?': print help\n"
                                    "   quit: exit'\n"
                                );
                                break;
                            default:
                                llvm_unreachable("invaid command value");
                        } 
                        goto READ;
                    }
                    f.i = 0;
                }
                return buf[f.i++];
            }
            case AStringStream:
            {
                return f.idx <= f.max ? f.s[f.idx++] : '\0';
            }
            default:
                llvm_unreachable("bad stream kind");
        }
    }
    LLVM_NODISCARD char raw_read_from_id(unsigned id) {
        return raw_read(streams[id]);
    }
    bool searchFileInDir(xstring &result, StringRef path, xstring *dir, size_t len) {
        for (size_t i = 0;i < len;++i)
        {
            result += dir[i];
            if (dir[i].empty() || dir[i].front() == '/' || dir[i].front() == '\\')
                result += '/';
            result += path;
            result.make_eos();
            if (addFile(result.data()), false)
                return true;
            result.clear();
        }
        return false;
    }
    bool searchFileInDir(xstring &result, StringRef path, const char** dir, size_t len) {
        for (size_t i = 0;i < len;++i)
        {
            result += dir[i];
            if (!*(dir[i]) || dir[i][0] == '/' || dir[i][0] == '\\')
                result += '/';
            result += path;
            result.make_eos();
            if (addFile(result.data()), false)
                return true;
            result.clear();
        }
        return false;
    }
    bool addIncludeFile(StringRef path, bool isAngled) {
        // https://stackoverflow.com/q/21593/15886008
        // path must be null terminated
        if (!isAngled && addFile(path.data()), false) {
            return true;
        }
        xstring result = xstring::get_with_capacity(256);
        if (searchFileInDir(result, path, userPaths.data(), userPaths.size()))
            return addFile(result, false);
        if (searchFileInDir(result, path, sysPaths.data(), sysPaths.size()))
            return addFile(result, false);
        result.free();
        return false;
    }
    void closeAllFiles() {
        for (const auto &f: streams){
            if (f.k == AFileStream)
#if WINDOWS
                ::CloseHandle(f.fd);
#else
                ::close(f.fd);
#endif
        }
    }
    void addStdin(const char *name = "<stdin>") {
        Stream x = Stream {.name = name, .k = AStdinStream};
        x.i = 0;
        x.readed = 0;
        includeStack.push_back(streams.size());
        streams.push_back(x);
    }
    void addString(StringRef s, const char *fileName = "<string>") {
        Stream x = Stream {.name = fileName, .k = AStringStream};
        x.idx = 0;
        x.max = s.size();
        x.s = s.data();
        includeStack.push_back(streams.size());
        streams.push_back(x);
    }
    void addCol() {
        ++streams[includeStack.back()].column;
    }
    void resetLine() {
        Stream &f = streams[includeStack.back()];
        ++f.line;
        f.column = 1;
    }
    uint16_t lastc = 256;
    LLVM_NODISCARD char raw_read_from_stack() {
        return raw_read(streams[includeStack.back()]);
    }
    // Translation phases 1
    LLVM_NODISCARD char stream_read() {
        // for support '\r' end-of-lines
        if (LLVM_UNLIKELY(lastc <= 0xFF)) {
            // trunc to i8
            char res = lastc;
            lastc = 256;
            return res;
        }
        if (includeStack.empty())
            return '\0';
        else {
            char c = raw_read_from_stack();
            addCol();
            switch (c) {
                default:
                    return c;
                case '\0':
                    includeStack.pop_back();
                    return stream_read();
                case '\n':
                    resetLine();
                    return '\n';
                case '\r':
                {
                    char c2;
                    resetLine();
                    c2 = raw_read_from_stack();
                    if (c2 != '\n')
                        lastc = (uint16_t)(unsigned char)c;
                    return '\n';
                }
            }
        }
    }
    // Translation phases 2, 3
    LLVM_NODISCARD char skip_read() {
        char c = stream_read();
        if (c == '/') {
            char c2 = stream_read();
            if (c2 == '*') {
                do {
                    c = stream_read();
                    if (c == '*') {
                        c = stream_read();
                        if (c == '/')
                            break;
                    } else if (c == '\0')
                        return pp_error("unterminated comment '/*'"), '\0';
                } while (1);
                return ' ';
            }
            if (c2 == '/') {
                do {
                    c = stream_read();
                } while (c != '\n' && c != '\0');
                return ' ';
            }
            lastc = (uint16_t)(unsigned char)c2;
            return c;
        }
        if (c == '\\') {
            char c2 = stream_read();
            if (c2 == '\n')
                return skip_read();
            if (c2 == ' ') {
                do 
                    c = stream_read();
                while (c != ' ' && c != '\0');
                if (c != '\n') {
                    if (c == '\0') {
                        warning("'\\' at end of input");
                        goto RET;
                    }
                    pp_error("%s", "bad '\\' and space without newline");
                    do 
                        c = stream_read();
                    while (c != '\n' && c != '\0');
                    RET:
                    return c;
                }
                return skip_read();
            }
            lastc = (uint16_t)(unsigned char)c2;
            return c;
        }
        return c;
    }
};

