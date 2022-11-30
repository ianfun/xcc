/*
 * SourceMgr.cpp - Manage source streams and source locations
 * inspired by gcc's line map and clang's SourceManager
*/

#if WINDOWS
typedef HANDLE fd_t;
#else
typedef int fd_t;
#endif
enum StreamKind : uint8_t {
    AFileStream,
    AStringStream,
    AStdinStream
};
struct Stream {
protected:
    Stream(enum StreamKind k, const char *name, location_t startLoc): k{k}, name{name}, line{1}, column{1}, startLoc{startLoc}, loc{startLoc} {}
    void setEndLocWithOffset(location_t offset) { endLoc = startLoc + offset; }
public:
    enum StreamKind k;
    const char *name;
    uint32_t line, column;
    location_t startLoc, loc, endLoc;
    enum StreamKind getKind() const {
        return k;
    }
    bool hasLineOffsetMapping = false;
    std::vector<uint32_t> lineOffsetMapping;
    void requestLineOffsetMapping();
};
// A stream that read from a system file handle
struct FileStream: public Stream {
    fd_t fd;
    unsigned p, pos;
#if WINDOWS
    using filepos_t = uint64_t;
#else
    using filepos_t = off_t;
#endif
    FileStream(const char *name, location_t startLoc, fd_t fd, location_t fileSize): Stream{AFileStream, name, startLoc}, fd{fd}, p{0}, pos{0} {
        setEndLocWithOffset(fileSize);
    }
    static bool classof(const Stream *other) {
        return other->getKind() == AFileStream;
    }
    void close() {
#if WINDOWS
        ::CloseHandle(fd);
#else
        ::close(fd);
#endif
    }
    filepos_t SavePos() {
#if WINDOWS
        LARGE_INTEGER res;
        LARGE_INTEGER seek_pos;
        seek_pos.QuadPart = -static_cast<ULONGLONG>(pos);
        if (!::SetFilePointerEx(fd, seek_pos, &res, FILE_CURRENT)) 
            return 0;
        seek_pos.QuadPart = 0;
        if (!::SetFilePointerEx(fd, seek_pos, nullptr, FILE_BEGIN))
            return 0;
        return res.QuadPart;
#else
        off_t res;
        if ((res = ::lseek(fd, -static_cast<off_t>(pos), SEEK_CUR)) < 0)
            return 0;
        if (::lseek(fd, 0, SEEK_SET) != 0)
            return 0;
        return res;
#endif
    }
    void RestorePos(filepos_t pos) {
#if WINDOWS
        LARGE_INTEGER seek_pos;
        seek_pos.QuadPart = pos;
        ::SetFilePointerEx(fd, seek_pos, nullptr, FILE_BEGIN);
#else
        ::lseek(fd, pos, SEEK_SET);
#endif
    }
    // clang::SrcMgr::LineOffsetMapping::get
    void createLineOffsetMapping() {
        constexpr size_t read_size = 4096;
        char *buffer = new char[read_size];
        assert(! hasLineOffsetMapping && "already has lineOffsetMapping!");
        bool lastCharIsCR = false;
        uint32_t I = 0;
        auto saved_location = SavePos();
        lineOffsetMapping.push_back(0);
        for (;;) {
#if WINDOWS
            DWORD L = 0;
            if (!::ReadFile(fd, buffer, read_size, &L, nullptr) || L == 0)
                break;
#else
            size_t L = ::read(fd, buffer, read_size);
            if (L == 0)
                break;
#endif
            for (size_t i = 0;i < read_size;++i) {
                char c = buffer[i];
                lastCharIsCR = false;
                if (c == '\n') {
                    if (!lastCharIsCR)
                        lineOffsetMapping.push_back(I + 1);
                } else if (c == '\r') {
                    lastCharIsCR = true;
                    lineOffsetMapping.push_back(I + 1);
                }
                I++;
            }
        }
        RestorePos(saved_location);
    }
};
static void createLineOffsetMappingForString(const char *s, size_t max, std::vector<uint32_t> &lineOffsetMapping) {
    uint32_t I = 0;
    lineOffsetMapping.push_back(0);
    for (size_t i = 0;i < max;++i) {
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
// A stream that read from stdin(fd 0)
struct StdinStream: public Stream {
    size_t idx;
    std::string contents;
    StdinStream(const char *name, location_t startLoc): Stream{AStdinStream, name, startLoc}, idx{0}, contents{} {
        setEndLocWithOffset(0);
    }
    static bool classof(const Stream *other) {
        return other->getKind() == AStdinStream;
    }
    unsigned getColumnNumber(location_t offset) {
        unsigned LineStart = offset;
        while (LineStart && contents[LineStart-1] != '\n' && contents[LineStart-1] != '\r')
          --LineStart;
        return offset-LineStart+1;
    }
    char read(char *buffer, bool is_tty) {
        if (idx == contents.size()) {
#if CC_NO_RAEADLINE
            for (;;) {
                char c;
                if (::read(0, &c, 1) <= 0)
                    return '\0';
                if (c == '\n')
                    break;
                contents += c;
            }
#else
            REPEAT:
            char *line = ::readline(STDIN_PROMPT);
            if (!line)
                return '\0';
            if (line[0] == '\0')
                goto REPEAT;
            ::add_history(line);
            contents += line;
            free(line);
#endif
        }
        return contents[idx++];
    }
    void createLineOffsetMapping() {
        return createLineOffsetMappingForString(contents.data(), contents.size(), lineOffsetMapping);
    }
};
// A stream read from string(e.g: define macro in command line, paste tokens)
struct StringStream: public Stream {
    size_t idx, max;
    const char *s;
    StringStream(const char *name, location_t startLoc, StringRef str) : Stream{AStringStream, name, startLoc}, idx{0}, max{str.size()}, s{str.data()} {
        setEndLocWithOffset(max);
    }
    static bool classof(const Stream *other) {
        return other->getKind() == AStringStream;
    }
    inline char read() {
        return idx <= max ? s[idx++] : '\0';
    }
    void createLineOffsetMapping() {
        return createLineOffsetMappingForString(s, max, lineOffsetMapping);
    }
    unsigned getColumnNumber(location_t offset) {
        unsigned LineStart = offset;
        while (LineStart && s[LineStart-1] != '\n' && s[LineStart-1] != '\r')
          --LineStart;
        return offset-LineStart+1;
    }
};
void Stream::requestLineOffsetMapping() {
    if (!hasLineOffsetMapping) {
        hasLineOffsetMapping = true;
        switch (getKind()) {
        case AFileStream: return cast<FileStream>(this)->createLineOffsetMapping();
        case AStringStream: return cast<StringStream>(this)->createLineOffsetMapping();
        case AStdinStream: return cast<StdinStream>(this)->createLineOffsetMapping();
        }
        llvm_unreachable("invalid stream");
    }
}
struct SourceMgr : public DiagnosticHelper {
    char buf[STREAM_BUFFER_SIZE];
    SmallVector<xstring, 16> sysPaths;
    SmallVector<xstring, 16> userPaths;
    SmallVector<Stream*> streams;
    SmallVector<fileid_t, 16> includeStack;
    LocTree *tree = nullptr;
    std::map<location_t, LocTree*, std::less<location_t>> location_map;
    StringRef lastDir = StringRef();
    bool trigraphs;
    bool is_tty;
    void setLine(uint32_t line) { streams[includeStack.back()]->line = line; }
    const char *getFileName(unsigned i) const { return streams[i]->name; }
    const char *getFileName() const { return includeStack.size() ? streams[includeStack.back()]->name : "(unknown file)"; }
    unsigned getLine() const { return includeStack.empty() ? 0 : streams[includeStack.back()]->line; }
    unsigned getLine(unsigned i) const { return streams[i]->line; }
    void setFileName(const char *name) { streams[includeStack.back()]->name = name; }
    location_t getCurrentLocation() const { return streams.back()->loc;}
    location_t getEndLoc() const {
        return includeStack.empty() ? 0 : streams[includeStack.back()]->endLoc;
    }
    location_t getLoc() const { return includeStack.empty() ? 0 : streams[includeStack.back()]->loc; }
    inline LocTree *getLocTree() const { return tree; }
    void beginExpandMacro(PPMacroDef *M, xcc_context &context) {
        location_map[getCurrentLocation()] = (tree = new (context.getAllocator()) LocTree(tree, M));
    }
    void beginInclude(xcc_context &context, fileid_t theFD) {
        assert(includeStack.size() >= 2 && "call beginInclude() without previous include position!");
        Include_Info *info = new (context.getAllocator()) Include_Info{.line = getLine(theFD), .fd = theFD};
        location_map[getCurrentLocation()] = (tree = new (context.getAllocator()) LocTree(tree, info));
    }
    void endTree() {
        location_map[getCurrentLocation()] = (tree = tree->getParent());
    }
    SourceMgr(DiagnosticsEngine &Diag) : DiagnosticHelper{Diag}, buf{}, trigraphs{false} {
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
        const size_t Id = streams.size();
        streams.push_back(new FileStream(path, getEndLoc(), fd, fileSize));
        lastDir = llvm::sys::path::parent_path(path, llvm::sys::path::Style::native);
        if (lastDir.empty())
            lastDir = ".";
        includeStack.push_back(Id);
        return true;
    }
    fileid_t getFileID(location_t loc, uint64_t &offset) const {
        for (size_t i = 0;i < streams.size();++i) {
            if (loc < streams[i]->endLoc) {
                offset = loc - streams[i]->startLoc;
                return i;
            }
        }
        return (fileid_t)-1;
    }
    fileid_t lastQueryFileId_line = (fileid_t)-1;
    location_t lastQueryOffset_line = (location_t)-1;
    uint32_t lastQueryResult_line = 0;
    uint32_t getLineNumber(location_t offset, fileid_t FD) {
        Stream *f = streams[FD];
        f->requestLineOffsetMapping();
        const uint32_t *lineStart = f->lineOffsetMapping.data();
        const uint32_t *lineEnd = f->lineOffsetMapping.data() + f->lineOffsetMapping.size();
        const uint32_t *searchBegin = lineStart;
        location_t QueriedFilePos = offset+1;
        if (FD == lastQueryFileId_line) {
            if (offset > lastQueryOffset_line)
                searchBegin = lineStart + lastQueryResult_line - 1;
            if (searchBegin+5 < lineEnd) {
                if (searchBegin[5] > QueriedFilePos)
                    lineEnd = searchBegin+5;
                else if (searchBegin+10 < lineEnd) {
                    if (searchBegin[10] > QueriedFilePos)
                        lineEnd = searchBegin+10;
                    else if (searchBegin+20 < lineEnd) {
                        if (searchBegin[20] > QueriedFilePos)
                            lineEnd = searchBegin+20;
                    }
                }
            }
        }
        const uint32_t *Pos = std::lower_bound(searchBegin, lineEnd, QueriedFilePos);
        return (lastQueryResult_line = Pos - lineStart);
    }
    uint32_t getColumnNumber(location_t offset, fileid_t FD) {
        Stream *f = streams[FD];
        if (StringStream *s = dyn_cast<StringStream>(f)) {
            return s->getColumnNumber(offset);
        }
        if (StdinStream *s = dyn_cast<StdinStream>(f)) {
            return s->getColumnNumber(offset);
        }
        FileStream *s = cast<FileStream>(f);
        uint32_t line = getColumnNumber(offset, FD);
        return offset - s->lineOffsetMapping[line] + 1;
    }
    bool translateLocation(location_t loc, source_location &out) {
        uint64_t offset;
        if (location_is_stdin(loc)) {
            for (size_t i = 0;i < streams.size();++i) {
                if (StdinStream *s = dyn_cast<StdinStream>(streams[i])) {
                    loc = location_as_index(loc);
                    offset = loc - s->startLoc;
                    out.fd = i;
                    goto FOUND;
                }
            }
            llvm_unreachable("no stdin stream found");
        } else {
            if (loc == 0) return false;
            fileid_t fd = getFileID(loc, offset);
            if (fd == (fileid_t)-1)
                return false;
            out.fd = fd;
        }
FOUND:
        auto it = location_map.lower_bound(loc);
        if (it == location_map.begin()) goto NO_TREE;
        it--;
        if (it == location_map.begin()) goto NO_TREE;
        out.tree = it->second;
        return get_source(offset, out);
NO_TREE:
        if (streams.size()) {
            out.tree = nullptr;
            return get_source(offset, out);
        }
        return false;
    }
    static void get_source_for_string(const char *s, size_t max, source_location &out, uint64_t offset) {
        size_t last_line_offset = 0;
        unsigned line = 0;
        for (size_t i = 0;i < offset;++i) {
            const char c = s[i];
            if (c == '\n') {
                line++;
                last_line_offset = i;
            }
        }
        out.line = line;
        out.col = offset - last_line_offset;
        if (out.col) 
            for (size_t i = last_line_offset;i < offset;++i)
                out.source_line.push_back(s[i]);
        for (size_t i = offset;i < max;++i) {
            const char c = s[i];
            if (c == '\n' || c == '\r')
                break;
            out.source_line.push_back(c);
        }
    }
    bool get_source(uint64_t offset, source_location &out) {
        Stream *target = streams[out.fd];
        if (StringStream *s = dyn_cast<StringStream>(target))
            return get_source_for_string(s->s, s->max, out, offset), true;
        if (StdinStream *s = dyn_cast<StdinStream>(target))
             return get_source_for_string(s->contents.data(), s->contents.size(), out, offset), true;
        FileStream *stream = cast<FileStream>(target);
        FileStream &f = *stream;
        auto saved_location = f.SavePos();
        location_t old_loc = f.loc;
        uint64_t readed = 0, last_line_offset = 0;
        unsigned line = 1;
        constexpr size_t read_size = 1024;
        char *buffer = new char[read_size];
        bool lastCharIsCR = false;
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
                lastCharIsCR = false;
                if (buffer[j] == '\n') {
                    if (!lastCharIsCR) {
                        line++;
                    }
                    last_line_offset = readed + j;
                } else if (buffer[j] == '\r') {
                    lastCharIsCR = true;
                    last_line_offset = readed + j;
                    line++;
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
                if (out.source_line.push_back(c) && i != 0) {
                    goto BREAK;
                }
            }
        }
        BREAK: ;
        f.pos = 0;
        f.loc = old_loc;
        delete [] buffer;
        f.RestorePos(saved_location);
        return true;
    }
    bool empty() const {
        return includeStack.empty();
    }
    // read a char from a stream
    char raw_read(Stream *stream) {
        stream->loc++;
        if (FileStream *f = dyn_cast<FileStream>(stream)) {
AGAIN:
        if (f->pos == 0) {
#if WINDOWS
            DWORD L = 0;
            if (!::ReadFile(f->fd, buf, STREAM_BUFFER_SIZE, &L, nullptr) || L == 0)
                return '\0'; // xxx: ReadFile error report ?
#else
READ_AGAIN:
            ssize_t L = ::read(f->fd, buf, STREAM_BUFFER_SIZE);
            if (L == 0)
                return '\0';
            if (L < 0) {
                if (errno == EINTR)
                    goto READ_AGAIN;
                pp_error("error reading file: %o", errno);
                return '\0';
            }
#endif
            f->pos = L;
            f->p = 0;
        }
        char c = buf[f->p++];
        f->pos--;
        if (c)
            return c;
        warning("null character(s) ignored");
        goto AGAIN;
        }
        if (StdinStream *f = dyn_cast<StdinStream>(stream))
            return f->read(buf, is_tty);
        return cast<StringStream>(stream)->read();
    }
    char raw_read_from_id(unsigned id) { return raw_read(streams[id]); }
    void resetBuffer() {
        Stream *stream = streams[includeStack[includeStack.size() - 2]];
        if (FileStream *f = dyn_cast<FileStream>(stream)) {
#if WINDOWS
            LARGE_INTEGER I;
            I.QuadPart = -static_cast<LONGLONG>(f->pos);
            ::SetFilePointerEx(f->fd, I, nullptr, FILE_CURRENT);
#else
            ::lseek(f->fd, -static_cast<off_t>(f->pos), SEEK_CUR);
#endif
            f->pos = 0;
            return;
        }
        return;
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
        for (const auto s : streams) {
            if (FileStream *f = dyn_cast<FileStream>(s))
                f->close();
            delete s;
        }
        streams.clear();
    }
    ~SourceMgr() {
        closeAllFiles();
    }
    void addStdin(const char *Name = "<stdin>") {
        const size_t Id = streams.size();
        streams.push_back(new StdinStream(Name, getEndLoc()));
        includeStack.push_back(Id);
    }
    void addString(StringRef s, const char *Name = "<string>") {
        const size_t Id = streams.size();
        streams.push_back(new StringStream(Name, getEndLoc(), s));
        includeStack.push_back(Id);
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
                location_map[getCurrentLocation()] = tree ? tree->getParent() : nullptr;
                includeStack.pop_back();
                if (includeStack.empty())
                    return '\0';
                return stream_read();
            case '\n': streams[includeStack.back()]->line++; return '\n';
            case '\r': {
                char c2;
                streams[includeStack.back()]->line++;
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
            Stream *f = streams[i];
            OS << "  stream[" << i << "]: Name = " << f->name << ", kind = ";
            switch (f->getKind()) {
            case AFileStream:
                OS << "(file stream, fd = " << cast<FileStream>(f)->fd << ", size = " << ")\n";
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
