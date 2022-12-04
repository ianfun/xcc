/*
 * SourceMgr.cpp - Manage source streams and source locations
 * inspired by gcc's line map and clang's SourceManager
 */

// rustc_span
// https://doc.rust-lang.org/beta/nightly-rustc/rustc_span/index.html

// clang::SourceManager
// https://doc.rust-lang.org/beta/nightly-rustc/rustc_span/index.html
// https://clang.llvm.org/doxygen/classclang_1_1FileManager.html

// gcc line map
// https://github.com/gcc-mirror/gcc/blob/master/libcpp/include/line-map.h

// https://doc.rust-lang.org/beta/nightly-rustc/rustc_span/struct.SourceFile.html
// https://clang.llvm.org/doxygen/classclang_1_1SrcMgr_1_1ContentCache.html
struct Stream {
    Stream(llvm::MemoryBuffer *MemBuffer, location_t startLoc, const char *Name): MemBuffer{MemBuffer}, startLoc{startLoc}, loc{0}, endLoc{static_cast<location_t>(startLoc + MemBuffer->getBufferSize())}, Name{Name}, BufferStart{MemBuffer->getBufferStart()} {}
    Stream(const Stream &) = delete;
    ~Stream() { delete MemBuffer; }
    llvm::MemoryBuffer *MemBuffer;
    location_t startLoc, loc, endLoc;
    const char *Name;
    bool hasLineOffsetMapping = false;
    SmallVector<uint32_t> lineOffsetMapping;
    const char *BufferStart;
    const char *getBufferStart() const { return BufferStart; }
    size_t getFileSize() const { return MemBuffer->getBufferSize(); }
    auto getBufferEnd() const { return MemBuffer->getBufferEnd(); }
    const SmallVectorImpl<unsigned> &getLineOffsetMapping() const {
        assert(hasLineOffsetMapping);
        return lineOffsetMapping;
    }
    SmallVectorImpl<unsigned> &getLineOffsetMappingSafe() {
        if (!hasLineOffsetMapping)
            createLineOffsetMapping();
        return lineOffsetMapping;
    }
    void createLineOffsetMapping() {
        assert(!hasLineOffsetMapping && lineOffsetMapping.empty());
        uint32_t I = 0;
        size_t max = MemBuffer->getBufferSize();
        const char *s = BufferStart;
        lineOffsetMapping.push_back(0);
        for (size_t i = 0; i < max; ++i) {
            char c = s[i];
            if (c == '\n') {
                lineOffsetMapping.push_back(I + 1);
            } else if (c == '\r') {
                if (i + 1 < max && s[i + 1] == '\n')
                    ++I;
                lineOffsetMapping.push_back(I + 1);
            }
            ++I;
        }
    }
    char readChar() {
        return loc < endLoc ? BufferStart[loc++] : '\0';
    }
    location_t getEndLoc() const { return startLoc + getFileSize(); }
    void setName(const char *Name) { this->Name = Name; }
    const char *getName() const { return Name; }
    const char *getFileName() const { return getName(); }
    StringRef getFileNameFromMemoryBuffer() const { return MemBuffer->getBufferIdentifier(); }
    StringRef getBuffer() const { return MemBuffer->getBuffer(); }
};
struct SourceMgr : public DiagnosticHelper {
    char buf[STREAM_BUFFER_SIZE];
    SmallVector<xstring, 16> sysPaths;
    SmallVector<xstring, 16> userPaths;
    SmallVector<Stream*> streams;
    SmallVector<fileid_t, 16> includeStack;
    LocTree *tree = nullptr;
    std::map<location_t, LocTree *, std::less<location_t>> location_map;
    StringRef lastDir = StringRef();
    bool trigraphs;
    bool is_tty;
    unsigned current_line = 1;
    void setLine(unsigned line) { current_line = line; }
    const char *getFileName(unsigned i) const { return streams[i]->getFileName(); }
    const char *getFileName() const {
        return includeStack.size() ? streams[includeStack.back()]->getFileName() : "(unknown file)";
    }
    void setFileName(const char *Name) { streams[includeStack.back()]->setName(Name); }
    location_t getCurrentLocation() const { return streams.back()->loc; }
    location_t getEndLoc() const { return streams.empty() ? 0 : streams[includeStack.back()]->endLoc; }
    location_t getLoc() const { return streams[includeStack.back()]->loc; }
    inline LocTree *getLocTree() const { return tree; }
    void beginExpandMacro(PPMacroDef *M, xcc_context &context) {
        location_map[getCurrentLocation()] = (tree = new (context.getAllocator()) LocTree(tree, M));
    }
    void beginInclude(xcc_context &context, fileid_t theFD) {
        assert(includeStack.size() >= 2 && "call beginInclude() without previous include position!");
        Include_Info *info = new (context.getAllocator()) Include_Info{.line = current_line, .fd = theFD};
        location_map[getCurrentLocation()] = (tree = new (context.getAllocator()) LocTree(tree, info));
    }
    void endTree() { location_map[getCurrentLocation()] = (tree = tree->getParent()); }
    SourceMgr(DiagnosticsEngine &Diag) : DiagnosticHelper{Diag}, buf{}, trigraphs{false} {
#if WINDOWS
        DWORD dummy;
        is_tty = GetConsoleMode(hStdin, &dummy) != 0;
#else
        is_tty = isatty(STDIN_FILENO) != 0;
#endif
    }
    void setTrigraphsEnabled(bool enable) { trigraphs = enable; }
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
                pp_error("cannot open %s: %w", path, static_cast<unsigned>(GetLastError()));
            return false;
        }
        DWORD highPart;
        fileSize = ::GetFileSize(fd, &highPart);
        if (fileSize == INVALID_FILE_SIZE)
            return error("failed to get file size for %s: %w", path, static_cast<unsigned>(GetLastError())), false;
        if (highPart) {
            // this can done much better in assembly code
            uint64_t total = highPart;
            total <<= 32;
            total |= fileSize;
            return error("file size %Z is too large for XCC", total), false;
        }
#else
        int fd = ::open(path, O_RDONLY);
        if (fd < 0) {
            if (verbose)
                error("cannot open %s: %o", path, errno);
            return false;
        }
        {
            struct stat sb;
            if (::fstat(fd, &sb))
                return error("fstat: %o", errno), false;
            if (!S_ISREG(sb.st_mode)) {
                error("not a regular file: %s", path);
                return false;
            }
            fileSize = sb.st_size;
        }
#endif
        ErrorOr<std::unique_ptr<MemoryBuffer>> MemberBufferOrErr = llvm::MemoryBuffer::getOpenFile(fd, path, fileSize);
        if (!MemberBufferOrErr) {
#if WINDOWS
            CloseHandle(fd);
#else
            close(fd);
#endif
            std::string msg = MemberBufferOrErr.getError().message();
            StringRef msgStr(msg);
            error("error reading %s: %R", path, msgStr);
            return false;
        }
        const size_t Id = streams.size();
        streams.push_back(new Stream(MemberBufferOrErr->release(), getEndLoc(), path));
        lastDir = llvm::sys::path::parent_path(path, llvm::sys::path::Style::native);
        if (lastDir.empty())
            lastDir = ".";
        includeStack.push_back(Id);
#if WINDOWS
        CloseHandle(fd);
#else
        close(fd);
#endif
        return true;
    }
    fileid_t getFileID(location_t loc, location_t &offset) const {
        for (size_t i = 0; i < streams.size(); ++i) {
            if (loc < streams[i]->endLoc) {
                offset = loc - streams[i]->startLoc;
                return i;
            }
        }
        return (fileid_t)-1;
    }
    fileid_t lastQueryFileId_line = (fileid_t)-1;
    location_t lastQueryOffset_line = (location_t)-1;
    unsigned lastQueryResult_line = 0;
    unsigned getLineNumber(location_t offset, fileid_t FD) {
        Stream &f = *streams[FD];
        const auto &line_vec = f.getLineOffsetMappingSafe();
        const unsigned *lineStart = line_vec.data();
        const unsigned *lineEnd = lineStart + line_vec.size();
        const unsigned *searchBegin = lineStart;
        location_t QueriedFilePos = offset + 1;
        if (FD == lastQueryFileId_line) {
            if (offset > lastQueryOffset_line)
                searchBegin = lineStart + lastQueryResult_line - 1;
            if (searchBegin + 5 < lineEnd) {
                if (searchBegin[5] > QueriedFilePos)
                    lineEnd = searchBegin + 5;
                else if (searchBegin + 10 < lineEnd) {
                    if (searchBegin[10] > QueriedFilePos)
                        lineEnd = searchBegin + 10;
                    else if (searchBegin + 20 < lineEnd) {
                        if (searchBegin[20] > QueriedFilePos)
                            lineEnd = searchBegin + 20;
                    }
                }
            }
        }
        const unsigned *Pos = std::lower_bound(searchBegin, lineEnd, QueriedFilePos);
        return (lastQueryResult_line = Pos - lineStart);
    }
    unsigned getLineNumber(location_t loc) {
        location_t offset;
        fileid_t FD = getFileID(loc, offset);
        if (FD == (fileid_t)-1)
            return 1;
        return getLineNumber(offset, FD);
    }
    unsigned getColumnNumber(location_t offset, fileid_t FD) {
        Stream &f = *streams[FD];
        const char *Buffer = f.getBufferStart();
        unsigned lineStart = offset;
        while (lineStart && Buffer[lineStart - 1] != '\n' && Buffer[lineStart - 1] != '\r')
            --lineStart;
        return offset - lineStart + 1;
    }
    unsigned getColumnNumber(location_t loc) {
        location_t offset;
        fileid_t FD = getFileID(loc, offset);
        if (FD == (fileid_t)-1)
            return 1;
        return getColumnNumber(offset, FD);
    }
    std::pair<unsigned, unsigned> getLineAndColumn(location_t loc) {
        location_t offset;
        fileid_t fd = getFileID(loc, offset);
        if (fd == (fileid_t)-1)
            return std::make_pair<unsigned, unsigned>(1, 1);
        return std::make_pair<unsigned, unsigned>(getLineNumber(offset, fd), getColumnNumber(offset, fd));
    }
    bool translateLocation(location_t loc, source_location &out, const ArrayRef<SourceRange> ranges = {}) {
        assert(streams.size() && "cannot call translateLocation when empty stream");
        if (loc == 0)
            return false;
        location_t offset;
        fileid_t fd = getFileID(loc, offset);
        if (fd == (fileid_t)-1)
            return false;
        out.fd = fd;
        auto it = location_map.lower_bound(loc);
        if (it == location_map.begin())
            goto NO_TREE;
        it--;
        if (it == location_map.begin())
            goto NO_TREE;
        out.tree = it->second;
        get_source(offset, out, loc, ranges);
        return true;
NO_TREE:
        out.tree = nullptr;
        get_source(offset, out, loc, ranges);
        return true;
    }
    // translate location_t to 
    // 1. source line(string)
    // 2. line
    // 3. column
    static void get_source_for_string(const char *s, size_t max, source_location &out, location_t offset, location_t loc,
                                      const ArrayRef<SourceRange> ranges) {
        location_t last_line_offset = 0;
        unsigned line = 1;
        bool lastIsCL = false;
        for (location_t i = 0; i < offset; ++i) {
            const char c = s[i];
            if (c == '\n') {
                if (lastIsCL) { // skip CRLF('\r\n') sequence
                    last_line_offset = i + 1;
                    continue;
                }
                line++;
                last_line_offset = i + 1;
            } else if (c == '\r') {
                lastIsCL = true; // maybe ends with CR or CRLF
                line++;
                last_line_offset = i + 1;
            }
        }
        out.line = line;
        out.col = offset - last_line_offset;
        out.source_line.setInsertLoc((loc - offset) + last_line_offset);
        if (out.col)
            for (location_t i = last_line_offset; i < offset; ++i)
                out.source_line.push_back(s[i], loc, ranges);
        out.col++;
        for (location_t i = offset; i < max; ++i) {
            const char c = s[i];
            if (c == '\n' || c == '\r')
                break;
            out.source_line.push_back(c, loc, ranges);
        }
    }
    void get_source(location_t offset, source_location &out, location_t original_loc,
                    const ArrayRef<SourceRange> ranges = {}) {
        Stream &s = *streams[out.fd];
        get_source_for_string(s.getBufferStart(), s.getFileSize(), out, offset, original_loc, ranges);
    }
    bool empty() const { return includeStack.empty(); }
    // read a char from a stream
    char raw_read(Stream &stream) {
        return stream.readChar();
    }
    char raw_read_from_id(unsigned id) { return raw_read(*streams[id]); }
    bool searchFileInDir(xstring &result, StringRef path, xstring *dir, size_t len) {
        for (size_t i = 0; i < len; ++i) {
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
    bool searchFileInDir(xstring &result, StringRef path, const char **dir, size_t len) {
        for (size_t i = 0; i < len; ++i) {
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
    bool addIncludeFile(StringRef path, bool isAngled, location_t loc) {
        // https://stackoverflow.com/q/21593/15886008
        // path must be null terminated
        xstring result = xstring::get_with_capacity(256);
        if (!isAngled && (!lastDir.empty())) {
            result += lastDir;
            result += '/';
            result += path;
            if (addFile(result.data(), false))
                return true;
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
                if (iter->type() == llvm::sys::fs::file_type::regular_file) {
                    if (path.edit_distance_insensitive(llvm::sys::path::filename(iter->path())) < 3) {
                        suggestions.push_back(iter->path());
                        if (suggestions.size() == 12)
                            goto BREAK;
                    }
                }
                iter.increment(EC);
            }
        }
        for (const auto &it : userPaths) {
            std::error_code EC;
            llvm::sys::fs::directory_iterator iter(it.str(), EC);
            llvm::sys::fs::directory_iterator end;
            while (iter != end) {
                if (EC)
                    break;
                if (iter->type() == llvm::sys::fs::file_type::regular_file) {
                    if (path.edit_distance_insensitive(llvm::sys::path::filename(iter->path())) < 3) {
                        suggestions.push_back(iter->path());
                        if (suggestions.size() == 12)
                            goto BREAK;
                    }
                }
                iter.increment(EC);
            }
        }
        for (const auto &it : sysPaths) {
            std::error_code EC;
            llvm::sys::fs::directory_iterator iter(it.str(), EC);
            llvm::sys::fs::directory_iterator end;
            while (iter != end) {
                if (EC)
                    break;
                if (iter->type() == llvm::sys::fs::file_type::regular_file) {
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
        for (const auto &it : suggestions) {
            Msg += "  - ";
            Msg += it;
        }
        Msg += " ";
        note("%r", StringRef(Msg));
    }
    // you should call this only if you don't use any streams and don't use any reference with MemoryBuffer(Name and contents)
    void closeAllFiles() {
        for (Stream *s : streams)
            delete s;
        streams.clear();
    }
    ~SourceMgr() { closeAllFiles(); }
    void addStdin(const char *Name = "<stdin>") {
        ErrorOr<std::unique_ptr<MemoryBuffer>> MemberBufferOrErr = llvm::MemoryBuffer::getSTDIN();
        if (!MemberBufferOrErr) {
            std::string msg = MemberBufferOrErr.getError().message();
            StringRef msgStr(msg);
            error("error reading from stdin: %R", Name, msgStr);
            return;
        }
        const size_t Id = streams.size();
        streams.push_back(new Stream(MemberBufferOrErr->release(), getEndLoc(), Name));
        includeStack.push_back(Id);
    }
    void addString(StringRef s, const char *Name = "<string>") {
        const size_t Id = streams.size();
        streams.push_back(new Stream(llvm::MemoryBuffer::getMemBuffer(s, Name, false).release(), getEndLoc(), Name));
        includeStack.push_back(Id);
    }
    uint16_t lastc = 256, lastc2 = 256;
    char raw_read_from_stack() { return raw_read(*streams[includeStack.back()]); }
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
                if (tree) {
                    current_line = tree->getLastIncludeLine();
                    location_map[getCurrentLocation()] = tree->getParent();
                } else {
                    location_map[getCurrentLocation()] = nullptr;
                }
                includeStack.pop_back();
                if (includeStack.empty())
                    return '\0';
                return stream_read();
            case '\n':
                current_line++; 
                return '\n';
            case '\r': {
                char c2;
                c2 = raw_read_from_stack();
                current_line++;
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
        default: return 0;
        case '=': return '#';
        case ')': return ']';
        case '(': return '[';
        case '!': return '|';
        case '\'': return '^';
        case '>': return '}';
        case '/': return '\\';
        case '<': return '{';
        case '-': return '~';
        }
    }
    void dump(raw_ostream &OS = llvm::errs()) const {
        OS << "Dumping class SourceMgr:\n"
           << "Number of Streams:" << streams.size() << '\n';
        if (streams.empty())
            OS << "  (empty)\n";
        else {
            for (unsigned i = 0; i < streams.size(); ++i) {
                const Stream &s = *streams[i];
                OS << "  # stream " << i << ": " << s.getFileName() << ", " << s.getFileSize() << " bytes, startLoc =" << s.startLoc << ", endLoc ="  << s.endLoc << "\n";
            }
        }
        OS << "\nInclude stack:\n";
        if (includeStack.empty())
            OS << "  (empty)\n";
        else {
            for (unsigned i = 0; i < includeStack.size(); ++i)
                OS << "  # include_stack " << i << ": fd =" << includeStack[i] << '\n';
        }
        OS << "\n";
    }
};
