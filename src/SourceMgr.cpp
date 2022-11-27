/* SourceMgr.cpp - Source Code manager */

#if WINDOWS
typedef HANDLE fd_t;
#else
typedef int fd_t;
#endif

enum StreamType : uint8_t {
    AFileStream,
    AStringStream,
    AStdinStream
};
struct Stream {
    enum StreamType k;
    const char *name;
    line_t line = 1;
    location_t startLoc, loc = 0;
    Stream(enum StreamType k, const char *Name) :k{k}, name{Name} {}    
    union {
        struct /*FileStream*/  {
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
        struct /*StringStream*/ {
            unsigned idx, max;
            const char *s;
        };
    };
};

struct SourceMgr : public DiagnosticHelper {
    SmallString<STREAM_BUFFER_SIZE> buf;
    SmallVector<xstring, 16> sysPaths;
    SmallVector<xstring, 16> userPaths;
    std::vector<Stream> streams;
    SmallVector<fileid_t, 16> includeStack;
    LocTree *tree = nullptr;
    std::map<location_t, LocTree*, std::less<location_t>> location_map;
    StringRef lastDir = StringRef();
    bool trigraphs;
    bool is_tty;
    void setLine(uint32_t line) { streams[includeStack.back()].line = line; }
    const char *getFileName(unsigned i) const { return streams[i].name; }
    const char *getFileName() const { return includeStack.size() ? streams[includeStack.back()].name : "(unknown file)"; }
    unsigned getLine() const { return streams.empty() ? 0 : streams[includeStack.back()].line; }
    unsigned getLine(unsigned i) const { return streams[i].line; }
    void setFileName(const char *name) { streams[includeStack.back()].name = name; }
    location_t getCurrentLocation() const { return streams.back().loc;}
    location_t getCurrentLocationSafe() const {
        return streams.empty() ? 0 : getCurrentLocation();
    }
    location_t getLoc() const { return includeStack.empty() ? 0 : streams[includeStack.back()].loc; }
    inline LocTree *getLocTree() const { return tree; }
    void beginExpandMacro(PPMacroDef *M, xcc_context &context) {
        location_map[getCurrentLocation()] = (tree = new (context.getAllocator()) LocTree(tree, M));
    }
    void beginInclude(xcc_context &context, fileid_t theFD) {
        assert(includeStack.size() >= 2 && "call beginInclude() without previous include position!");
        Include_Info *info = new (context.getAllocator()) Include_Info{.line = getLine(theFD), .fd = theFD};
        location_map[streams[includeStack.back()].startLoc] = (tree = new (context.getAllocator()) LocTree(tree, info));
    }
    void endTree() {
        location_map[getCurrentLocation()] = (tree = tree->getParent());
    }
    SourceMgr(DiagnosticsEngine &Diag) : DiagnosticHelper{Diag}, buf{}, trigraphs{false} {
        buf.resize_for_overwrite(STREAM_BUFFER_SIZE);
#if WINDOWS
        DWORD dummy;
        is_tty = GetConsoleMode(hStdin, &dummy) != 0;
#else
        is_tty = isatty(STDIN_FILENO) != 0;
#endif
    }
    void setTrigraphsEnabled(bool enable) {
        trigraphs = enable;
    }
    void addUsernIcludeDir(xstring path) { userPaths.push_back(path); }
    void addSysIncludeDir(xstring path) { sysPaths.push_back(path); }
    void addUsernIcludeDir(StringRef path) { userPaths.push_back(xstring::get(path)); }
    void addSysIncludeDir(StringRef path) { sysPaths.push_back(xstring::get(path)); }
    bool addFile(const char *path, bool verbose = true) {
        location_t fileSize;
#if WINDOWS
        llvm::SmallVector<WCHAR, 128> convertBuffer;
        if (llvm::sys::windows::CurCPToUTF16(path, convertBuffer))
            return false;
        HANDLE fd = ::CreateFileW(convertBuffer.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fd == INVALID_HANDLE_VALUE) {
            if (verbose)
                pp_error("cannot open %s: %w", path, static_cast<uint32_t>(GetLastError()));
            return false;
        }
        /*
        DWORD highPart;
        fileSize = ::GetFileSize(fd, &highPart);
        if (fileSize == INVALID_FILE_SIZE )
            return error("failed to get file size for %s: %w", path, (DWORD)GetLastError()), false;
        if (highPart) {
            // this can done much better in assembly code
            uint64_t total = highPart;
            total <<= 32;
            total |= fileSize;
            return errno("file size %Z is too large for XCC", total), false;
        }*/
#else
        int fd = ::open(path, O_RDONLY);
        if (fd < 0) {
            if (verbose)
                error("cannot open %s: %o", path, errno);
            return false;
        }
        /*{
            struct stat sb;
            if (::fstat(fd, &sb))
                return error("fstat: %o", errno), false;
            if (!S_ISREG(sb.st_mode)) {
                error("not a regular file: %s", path);
                return false;
            }
            fileSize = sb.st_size;
        }*/
#endif  
        auto Id = streams.size();
        streams.emplace_back(AFileStream, path);
        Stream &f = streams.back();
        f.p = 0;
        f.pos = 0;
        lastDir = llvm::sys::path::parent_path(path, llvm::sys::path::Style::native);
        if (lastDir.empty())
            lastDir = ".";
        f.fd = fd;
        f.startLoc = getCurrentLocationSafe();
        includeStack.push_back(Id);
        return true;
    }
    fileid_t getFileFromLocation(location_t loc, uint64_t &offset) const {
        if (streams.empty()) return -1;
        for (size_t i = 0;i < streams.size();++i) {
            if (loc < streams[i].startLoc) {
                offset = streams[i - 1].startLoc - loc;
                return i - 1;
            }
        }
        offset = loc;
        return 0;
    }
    bool translateLocation(location_t loc, source_location &out) {
        if (loc == 0) return false;
        uint64_t offset;
        fileid_t fd = getFileFromLocation(loc, offset);
        if (fd == (fileid_t)-1)
            return false;
        out.fd = fd;
        auto it = location_map.lower_bound(loc);
        if (it == location_map.end()) goto NO_TREE;
        if (it == location_map.begin()) goto NO_TREE;
        it--;
        if (it == location_map.begin()) goto NO_TREE;
        out.tree = it->second;
        return translateLocationLineAndColumn(loc, out);
NO_TREE:
        if (streams.size()) {
            out.tree = nullptr;
            return translateLocationLineAndColumn(loc, out);
        }
        return false;
    }
    bool translateLocationLineAndColumn(uint64_t offset, source_location &out) {
        Stream &f = streams[out.fd];
        if (f.k != AFileStream) {
            out.line = 0;
            out.col = 0;
            return true;
        }
#if WINDOWS
        LARGE_INTEGER saved_posl;
        {
            LARGE_INTEGER seek_pos;
            seek_pos.QuadPart = -static_cast<ULONGLONG>(f.pos);
            if (!::SetFilePointerEx(f.fd, seek_pos, &saved_posl, FILE_CURRENT)) 
                return false;
            seek_pos.QuadPart = 0;
            ::SetFilePointerEx(f.fd, seek_pos, nullptr, FILE_BEGIN);
        }
#else
        off_t saved_posl;
        if ((saved_posl = ::lseek(f.fd, -static_cast<off_t>(f.pos), SEEK_CUR)) < 0) 
            return false;
        ::lseek(f.fd, 0, SEEK_SET);
#endif
        location_t old_loc = f.loc;
        uint64_t readed = 0, last_line_offset = 0;
        unsigned line = 1;
        constexpr size_t read_size = 1024;
        char *buffer = new char[read_size];
        for (;;) {
            uint64_t to_read = offset - readed;
            if (LLVM_LIKELY(to_read > read_size))
                to_read = read_size;
            if (LLVM_UNLIKELY(to_read == 0))
                break;
#if WINDOWS
            DWORD L = 0;
            if (!::ReadFile(f.fd, buffer, to_read, &L, nullptr) || L != to_read)
                break;
#else
            size_t L = ::read(f.fd, buffer, to_read);
            if (L != (size_t)to_read)
                break;
#endif
            for (uint64_t j = 0;j < to_read;++j) {
                if (buffer[j] == '\n') {
                    line++;
                    last_line_offset = readed + j;
                }
            }
            readed += to_read;
        }
        out.line = line;
        out.col = offset - last_line_offset;
        if (out.col) {
#if WINDOWS
            LARGE_INTEGER seek_pos;
            seek_pos.QuadPart = -static_cast<uint64_t>(out.col);
            ::SetFilePointerEx(f.fd, seek_pos, nullptr, FILE_CURRENT);
#else
            ::lseek(f.fd, -static_cast<off_t>(out.col), SEEK_CUR);
#endif
        }
        // now, read source line
        for (;;) {
#if WINDOWS
            DWORD L = 0;
            if (!::ReadFile(f.fd, buffer, read_size, &L, nullptr) || L == 0) 
                break;
#else
            size_t L = ::read(f.fd, buffer, read_size);
            if (L == 0)
                break;
#endif
            for (unsigned i = 0;i < L;++i) {
                const char c = buffer[i];
                if (i == 0 && (c == '\n' || c == '\r'))
                    continue;
                if (out.source_line.push_back(c)) {
                    goto BREAK;
                }
            }
        }
        BREAK: ;
        f.pos = 0;
        f.loc = old_loc;
        delete [] buffer;
#if WINDOWS
        ::SetFilePointerEx(f.fd, saved_posl, nullptr, FILE_BEGIN);
#else
        ::lseek(f.fd, saved_posl, SEEK_SET);
#endif
        return true;
    }
    bool empty() const {
        return includeStack.empty();
    }
    // read a char from a stream
    char raw_read(Stream &f) {
        f.loc++;
        switch (f.k) {
        case AFileStream: {
AGAIN:
            if (f.pos == 0) {
#if WINDOWS
                DWORD L = 0;
                if (!::ReadFile(f.fd, buf.data(), STREAM_BUFFER_SIZE, &L, nullptr) || L == 0)
                    return '\0'; // xxx: ReadFile error report ?
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
        case AStdinStream: {
            if (f.i >= f.readed) {
READ:
#if CC_NO_RAEADLINE
#if WINDOWS /* Windows */
SAGAIN:
                if (is_tty) {
                    ::WriteConsoleW(hStdin, WSTDIN_PROMPT, cstrlen(WSTDIN_PROMPT), nullptr, nullptr);
                    if (::ReadConsoleW(hStdin, reinterpret_cast<PWCHAR>(buf.data()), buf.size() / 2, &f.readed,
                                       nullptr) == 0) {
                        return '\0';
                    }
                    for (DWORD i = 0; i < f.readed; f.i++) {
                        // ^D = 4
                        // ^X = 24
                        // ^Z = 26
                        if (buf[i] == 26 || buf[i] == 4) {
                            return '\0';
                        }
                    }
                } else {
                    if (!::ReadFile(hStdin, buf.data(), buf.size(), &f.readed, nullptr)) {
                        return '\0';
                    }
                }
                if (f.readed == 0)
                    goto SAGAIN;
#else /* Unix */
                char c;
                ssize_t L;
                for (unsigned idx = 0;;) {
                    L = ::read(STDIN_FILENO, &c, 1);
                    if (L <= 0) {
                        if (errno == EINTR)
                            continue;
                        if (L)
                            pp_error("reading from stdin: %o", errno);
                        return '\0';
                    }
                    buf[idx++] = c;
                    if (c == '\n' || idx == buf.size()) {
                        f.readed = idx;
                        break;
                    }
                }
#endif
#else /* GNU Readline */
REPEAT:
                buf.clear();
                char *line = ::readline(STDIN_PROMPT);
                if (!line)
                    return '\0';
                if (!*line)
                    goto REPEAT;
                ::add_history(line);
                {
                    StringRef line_str = StringRef(line);
                    size_t max_ch = std::max(line_str.size(), STREAM_BUFFER_SIZE);
                    memcpy(buf.data(), line_str.data(), max_ch);
                    ::free(line_str);
                    f.readed = line_str.size();
                }
#endif
                if (buf[0] == ':') {
                    enum Command command = NotACommand;

#if WINDOWS
                    TinyCommandParser<WCHAR> P(reinterpret_cast<PWCHAR>(buf.data() + 1), f.readed - 1);
                    std::wstring str;
#else
                    TinyCommandParser<char> P(buf.data() + 1, f.readed);
                    std::string str;
#endif
                    if (P.isChar('?')) {
                        command = Command::Help;
                    } else {
                        P.readIdent(str);
                        if (str.empty()) {
                            P.raw("cc: no command given.try ':help'\n");
                            goto READ;
                        }
                    }
#if WINDOWS
#define XCMD(s) L##s
#else
#define XCMD(s) s
#endif
                    if (str == XCMD("quit"))
                        command = Command::Quit;
                    else if (str == XCMD("help"))
                        command = Command::Help;
                    else if (str == XCMD("quit"))
                        command = Command::Quit;
                    switch (command) {
                    case Command::NotACommand:
#if WINDOWS
                        P.fmt("cc: not a command '%s'\n", str.data());
#else
                        P.fmt("cc: not a command '%L'\n", str.data());
#endif
                        break;
                    case Command::Quit: llvm::llvm_shutdown(); exit(CC_EXIT_SUCCESS);
                    case Command::Help:
                        P.raw("cc command help\n"
                              "Commands is always follows by a ':':, for example: ':quit', and may has arguments after "
                              "command"
                              "\nAvailable commands:\n"
                              "   help:, ':?': print help\n"
                              "   quit: exit'\n");
                        break;
                    default: llvm_unreachable("invaid command value");
                    }
                    goto READ;
                }
                f.i = 0;
            }
            return buf[f.i++];
        }
        case AStringStream: {
            return f.idx <= f.max ? f.s[f.idx++] : '\0';
        }
        default: llvm_unreachable("bad stream kind");
        }
    }
    char raw_read_from_id(unsigned id) { return raw_read(streams[id]); }
    void resetBuffer() {
        Stream &target = streams[includeStack[includeStack.size() - 2]];
        switch (target.k) {
        case AFileStream:
#if WINDOWS
            LARGE_INTEGER I;
            I.QuadPart = -static_cast<LONGLONG>(target.pos);
            ::SetFilePointerEx(target.fd, I, nullptr, FILE_CURRENT);
#else
            ::lseek(target.fd, -static_cast<off_t>(target.pos), SEEK_CUR);
#endif
            target.pos = 0;
            break;
        case AStringStream: break;
        case AStdinStream: target.readed = 0; break;
        }
    }
    bool searchFileInDir(xstring &result, StringRef path, xstring *dir, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            result += dir[i];
            if (dir[i].empty() || dir[i].front() == '/' || dir[i].front() == '\\')
                result += '/';
            result += path;
            result.make_eos();
            if (addFile(result.data()), false)
                return resetBuffer(), true;
            result.clear();
        }
        return false;
    }
    bool searchFileInDir(xstring &result, StringRef path, const char **dir, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            result += dir[i];
            if (!*(dir[i]) || dir[i][0] == '/' || dir[i][0] == '\\')
                result += '/';
            result += path;
            result.make_eos();
            if (addFile(result.data()), false)
                return resetBuffer(), true;
            result.clear();
        }
        return false;
    }
    bool addIncludeFile(StringRef path, bool isAngled, location_t loc) {
        // https://stackoverflow.com/q/21593/15886008
        // path must be null terminated
        xstring result = xstring::get_with_capacity(256);
        if (!isAngled && (!lastDir.empty())) {
            result += lastDir;
            result += '/';
            result += path;
            if (addFile(result.data(), false))
                return resetBuffer(), true;
            result.clear();
        }
        if (searchFileInDir(result, path, userPaths.data(), userPaths.size()))
            return true;
        if (searchFileInDir(result, path, sysPaths.data(), sysPaths.size()))
            return true;
        result.free();
        pp_error(loc, "#include file not found: %r", StringRef(path.data(), path.size() - 1));
        return suggestPath(path, isAngled), false;
    }
    void suggestPath(StringRef path, bool isAngled) {
        path = llvm::sys::path::filename(path);
        if (path.empty() || path.back() == '/' || path.back() == '\\')
            return;
        SmallVector<std::string, 12> suggestions;
        if (!isAngled && (!lastDir.empty())) {
            std::error_code EC;
            llvm::sys::fs::directory_iterator iter(lastDir, EC);
            llvm::sys::fs::directory_iterator end;
            while (iter != end) {
                if (EC)
                    break;
                if (iter->type() ==  llvm::sys::fs::file_type::regular_file) {
                    if (path.edit_distance_insensitive(llvm::sys::path::filename(iter->path())) < 3) {
                        suggestions.push_back(iter->path());
                        if (suggestions.size() == 12)
                            goto BREAK;
                    }
                }
                iter.increment(EC);
            }
        }
        for (const auto &it: userPaths) {
            std::error_code EC;
            llvm::sys::fs::directory_iterator iter(it.str(), EC);
            llvm::sys::fs::directory_iterator end;
            while (iter != end) {
                if (EC)
                    break;
                if (iter->type() ==  llvm::sys::fs::file_type::regular_file) {
                    if (path.edit_distance_insensitive(llvm::sys::path::filename(iter->path())) < 3) {
                        suggestions.push_back(iter->path());
                        if (suggestions.size() == 12)
                            goto BREAK;
                    }
                }
                iter.increment(EC);
            }
        }
        for (const auto &it: sysPaths) {
            std::error_code EC;
            llvm::sys::fs::directory_iterator iter(it.str(), EC);
            llvm::sys::fs::directory_iterator end;
            while (iter != end) {
                if (EC)
                    break;
                if (iter->type() ==  llvm::sys::fs::file_type::regular_file) {
                    if (path.edit_distance_insensitive(llvm::sys::path::filename(iter->path())) < 3) {
                        suggestions.push_back(iter->path());
                        if (suggestions.size() == 12)
                            goto BREAK;
                    }
                }
                iter.increment(EC);
            }
        }
        if (suggestions.size())
            goto BREAK;
        return;
        BREAK:
        std::string Msg = "Maybe you meant:\n";
        for (const auto &it: suggestions) {
            Msg += "  - ";
            Msg += it;
        }
        Msg += " ";
        note("%r", StringRef(Msg));
    }
    void closeAllFiles() {
        for (const auto &f : streams) {
            if (f.k == AFileStream)
#if WINDOWS
                ::CloseHandle(f.fd);
#else
                ::close(f.fd);
#endif
        }
    }
    void addStdin(const char *Name = "<stdin>") {
        auto Id = streams.size();
        streams.emplace_back(AStdinStream, Name);
        Stream &f = streams.back();
        f.i = 0;
        f.readed = 0;
        f.startLoc = getCurrentLocationSafe();
        includeStack.push_back(Id);
    }
    void addString(StringRef s, const char *Name = "<string>") {
        auto Id = streams.size();
        streams.emplace_back(AStringStream, Name);
        Stream &f = streams.back();
        f.idx = 0;
        f.max = s.size();
        f.s = s.data();
        f.startLoc = getCurrentLocationSafe();
        includeStack.push_back(Id);
    }
    void resetLine() {
        Stream &f = streams[includeStack.back()];
        ++f.line;
    }
    uint16_t lastc = 256, lastc2 = 256;
    char raw_read_from_stack() { return raw_read(streams[includeStack.back()]); }
    // Translation phases 1
    char stream_read() {
        // for support '\r' end-of-lines
        if (LLVM_UNLIKELY(lastc <= 0xFF)) {
            if (LLVM_UNLIKELY(lastc2 <= 0xFF)) {
                char res = lastc;
                lastc2 = 256;
                return res;
            }
            char res = lastc;
            lastc = 256;
            return res;
        }
        if (includeStack.empty()) {
            return '\0';
        } else {
            char c = raw_read_from_stack();
            switch (c) {
            default: return c;
            case '\0':
                location_map[streams[includeStack.back()].loc] = tree ? tree->getParent() : nullptr;
                includeStack.pop_back();
                if (includeStack.empty())
                    return '\0';
                return stream_read();
            case '\n': resetLine(); return '\n';
            case '\r': {
                char c2;
                resetLine();
                c2 = raw_read_from_stack();
                if (c2 != '\n')
                    lastc = (unsigned char)c;
                return '\n';
            }
            }
        }
    }
    char skip_read() {
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
                        return pp_error(getLoc(), "unterminated comment '/*'"), '\0';
                } while (1);
                return ' ';
            }
            if (c2 == '/') {
                do {
                    c = stream_read();
                } while (c != '\n' && c != '\0');
                return ' ';
            }
            lastc = (unsigned char)c2;
            return c;
        }
        BACKSLASH:
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
                        warning(getLoc(), "'\\' at end of input");
                        goto RET;
                    }
                    pp_error(getLoc(), "expect newline after '\\'");
                    do
                        c = stream_read();
                    while (c != '\n' && c != '\0');
RET:
                    return c;
                }
                return skip_read();
            }
            lastc = (unsigned char)c2;
            return c;
        }
        // trigraphs
        if (c == '?') {
            char c2 = stream_read();
            if (c2 == '?') {
                char c3 = stream_read();
                char t = GetTrigraphCharForLetter(c3);
                if (t) {
                    if (trigraphs) {
                        warning(getLoc(), "trigraph '??%c' converted to '%c'", c3, t);
                        if (t == '\\')
                            goto BACKSLASH;
                        return t;
                    }
                    warning(getLoc(), "trigraph '??%c' ignored, use -trigraphs to enable", c3);
                }
                lastc = (unsigned char)c3;
                lastc2 = c2;
                return c;
            }
            lastc = (unsigned char)c2;
            return c;
        }
        return c;
    }
    static char GetTrigraphCharForLetter(char Letter) {
      switch (Letter) {
      default:   return 0;
      case '=':  return '#';
      case ')':  return ']';
      case '(':  return '[';
      case '!':  return '|';
      case '\'': return '^';
      case '>':  return '}';
      case '/':  return '\\';
      case '<':  return '{';
      case '-':  return '~';
      }
    }
    void dump(raw_ostream &OS = llvm::errs()) const {
        OS <<
        "Dumping class SourceMgr:\n" << 
        "Number of Streams:" << streams.size() << '\n';
        if (streams.empty())
            OS << "  (empty)\n";
        for (unsigned i = 0;i < streams.size();++i) {
            const Stream &f = streams[i];
            OS << "  stream[" << i << "]: Name = " << f.name << ", kind = ";
            switch (f.k) {
            case AFileStream:
                OS << "(file stream, fd = " << f.fd << ", size = " << ")\n";
                break;
            case AStdinStream:
                OS << "(stdin stream)\n";
                break;
            case AStringStream:
                OS << "(string stream)\n";
                break;
            }
        }
        OS << "\nInclude stack:\n";
        if (includeStack.empty())
            OS << "  (empty)\n";
        for (unsigned i = 0;i < includeStack.size();++i)
            OS << "include_stack[" << i << "] => fd " << includeStack[i] << '\n';
        OS << "\n";
    }
};
