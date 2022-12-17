/*
 * SourceMgr.cpp - Manage source streams and source locations
 * inspired by gcc's line map and clang's SourceManager
 */

// rustc_span
// https://doc.rust-lang.org/beta/nightly-rustc/rustc_span/index.html

// clang::SourceManager
// https://doc.rust-lang.org/beta/nightly-rustc/rustc_span/index.html
// https://clang.llvm.org/doxygen/classclang_1_1FileManager.html

// llvm::SourceMgr
// https://llvm.org/doxygen/classllvm_1_1SourceMgr.html

// gcc line map
// https://github.com/gcc-mirror/gcc/blob/master/libcpp/include/line-map.h

// https://doc.rust-lang.org/beta/nightly-rustc/rustc_span/struct.SourceFile.html
// https://clang.llvm.org/doxygen/classclang_1_1SrcMgr_1_1ContentCache.html

struct ContentCache {
    llvm::MemoryBuffer *Buffer; // owns the buffer, never null
    void *lineOffsetMapping;
    StringRef Name;
    ContentCache() = delete;
    ContentCache(llvm::MemoryBuffer *Buffer, const StringRef &Name = StringRef())
        : Buffer{Buffer}, lineOffsetMapping{nullptr}, Name{Name} {
    }
    ContentCache(const ContentCache &) = delete;
    void setNameAsBufferName() {
        this->Name = Buffer->getBufferIdentifier();
    }
    void setName(StringRef Name) { this->Name = Name; }
    StringRef getName() const { return Name; }
    size_t getFileSize() const { return Buffer->getBufferSize(); }
    template <typename T> unsigned getLineNumberFor(location_t offset) const {
        const xvector<T> vec = xvector<T>::from_opache_pointer(lineOffsetMapping);
        const T *start = &vec.front();
        const T *end = &vec.back();
        return static_cast<unsigned>(std::lower_bound(start, end, static_cast<T>(offset)) - start);
    }
    template <typename T> location_t getLocation_tFromLineImpl(unsigned line) const {
        return xvector<T>::from_opache_pointer(lineOffsetMapping)[line];
    }
    location_t getLocation_tFromLine(unsigned line) const {
        assert(lineOffsetMapping);
        size_t Size = getFileSize();
        if (Size < size_t(0xFF))
            return getLocation_tFromLineImpl<uint8_t>(line);
        else if (Size <= size_t(0xFFFF))
            return getLocation_tFromLineImpl<uint16_t>(line);
        return getLocation_tFromLineImpl<uint32_t>(line);
    }
    unsigned getLineNumber(location_t offset) const {
        assert(lineOffsetMapping);
        size_t Size = getFileSize();
        if (Size < size_t(0xFF))
            return getLineNumberFor<uint8_t>(offset);
        else if (Size <= size_t(0xFFFF))
            return getLineNumberFor<uint16_t>(offset);
        return getLineNumberFor<uint32_t>(offset);
    }
    void createLineOffsetMapping() {
        if (!lineOffsetMapping) {
            size_t Size = getFileSize();
            if (Size < size_t(0xFF))
                return createLineOffsetMappingImpl<uint8_t>();
            if (Size <= size_t(0xFFFF))
                return createLineOffsetMappingImpl<uint16_t>();
            return createLineOffsetMappingImpl<uint32_t>();
        }
    }
    llvm::MemoryBuffer *getBuffer() { return Buffer; }
    const llvm::MemoryBuffer *getBuffer() const { return Buffer; }
    const char *getBufferStart() const { return Buffer->getBufferStart(); }
    template <typename T> void createLineOffsetMappingImpl() {
        xvector<T> vec = xvector<T>::get_with_capacity(256);
        T I = 0;
        size_t max = Buffer->getBufferSize();
        const char *s = Buffer->getBufferStart();
        vec.push_back(0);
        for (size_t i = 0; i < max; ++i) {
            char c = s[i];
            if (c == '\n') {
                vec.push_back(I + 1);
            } else if (c == '\r') {
                if (i + 1 < max && s[i + 1] == '\n')
                    ++I;
                vec.push_back(I + 1);
            }
            ++I;
        }
        lineOffsetMapping = vec.p;
    }
    ~ContentCache() {
        delete Buffer;
        if (lineOffsetMapping)
            xvectorBase::destroy_buffer(lineOffsetMapping);
    }
};
struct IncludeFile {
    ContentCache *cache;
    location_t where_I_included;
    location_t startLoc;
    location_t getFileSize() const { return cache->getFileSize(); }
    location_t getStartLoc() const { return startLoc; }
    location_t getEndLoc() const { return startLoc + getFileSize(); }
    location_t getIncludePos() const { return where_I_included; }
    const char *getBufferStart() const { return cache->getBufferStart(); }
    IncludeFile(ContentCache *cache, location_t startLoc = 0, location_t includePos = 0)
        : cache{cache}, where_I_included{includePos}, startLoc{startLoc} {}
};
struct SourceMgr : public DiagnosticHelper {
    char buf[STREAM_BUFFER_SIZE];
    SmallVector<xstring, 16> sysPaths;
    SmallVector<xstring, 16> userPaths;
    SmallVector<IncludeFile> includeStack; // incremental include stack
    SmallVector<unsigned> grow_include_stack; // the actual dynamic include stack
    llvm::StringMap<ContentCache *> cached_files;
    SmallVector<ContentCache *, 0> cached_strings;
    LocTree *tree = nullptr;
    std::map<location_t, LocTree * /*, std::less<location_t>*/> location_map;
    StringRef lastDir = StringRef();
    bool trigraphs = false;
    unsigned current_line = 1;
    location_t cur_loc = 0;
    const char *cur_buffer_ptr = nullptr;
    void setLine(unsigned line) { current_line = line; }
    StringRef getFileName() const { return includeStack[grow_include_stack.back()].cache->getName(); }
    ~SourceMgr() {
        for (ContentCache *it : cached_strings) {
            delete it;
        }
        for (const auto &it : cached_files) {
            ContentCache *c = it.second;
            delete c;
        }
    }
    void setFileName(StringRef Name) { includeStack[grow_include_stack.back()].cache->setName(Name); }
    bool isFileEOF() const { 
        const IncludeFile last = includeStack[grow_include_stack.back()];
        location_t endLoc = last.getEndLoc();
        return cur_loc >= endLoc || (cur_loc == endLoc - 1);
    }
    location_t getLoc() const { return cur_loc; }
    inline LocTree *getLocTree() const { return tree; }
    void beginExpandMacro(PPMacroDef *M, xcc_context &context) {
        location_map[getLoc()] = (tree = new (context.getAllocator()) LocTree(tree, M));
    }
    void endTree() { location_map[getLoc()] = (tree = tree->getParent()); }
    SourceMgr(DiagnosticsEngine &Diag) : DiagnosticHelper{Diag}, buf{}, trigraphs{false} {}
    void setTrigraphsEnabled(bool enable) { trigraphs = enable; }
    void addUsernIcludeDir(xstring path) { userPaths.push_back(path); }
    void addSysIncludeDir(xstring path) { sysPaths.push_back(path); }
    void addUsernIcludeDir(StringRef path) { userPaths.push_back(xstring::get(path)); }
    void addSysIncludeDir(StringRef path) { sysPaths.push_back(xstring::get(path)); }
    location_t getInsertPos() const {
        if (includeStack.empty())
            return 0;
        return includeStack.back().getEndLoc();
    }
    bool addFile(StringRef path, bool verbose = true, location_t includePos = 0) {
        auto it = cached_files.insert({path, static_cast<ContentCache *>(nullptr)});
        if (!it.second) {
PUSH:
            includeStack.emplace_back(it.first->second, getInsertPos(), includePos);
            addBuffer();
            return true;
        }
        location_t fileSize;
#if WINDOWS
        llvm::SmallVector<WCHAR, 128> convertBuffer;
        if (llvm::sys::windows::CurCPToUTF16(path, convertBuffer))
            return false;
        HANDLE fd = ::CreateFileW(convertBuffer.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fd == INVALID_HANDLE_VALUE) {
            if (verbose)
                pp_error("cannot open %R: %w", path, static_cast<unsigned>(GetLastError()));
            return false;
        }
        DWORD highPart;
        fileSize = ::GetFileSize(fd, &highPart);
        if (fileSize == INVALID_FILE_SIZE)
            return error("failed to get file size for %R: %w", path, static_cast<unsigned>(GetLastError())), false;
        if (highPart) {
            // this can done much better in assembly code
            uint64_t total = highPart;
            total <<= 32;
            total |= fileSize;
            return error("file size %Z is too large for XCC", total), false;
        }
#else
        int fd = ::open(path.data(), O_RDONLY);
        if (fd < 0) {
            if (verbose)
                error("cannot open %R: %o", path, errno);
            return false;
        }
        {
            struct stat sb;
            if (::fstat(fd, &sb))
                return error("fstat: %o", errno), false;
            if (!S_ISREG(sb.st_mode)) {
                error("not a regular file: %R", path);
                return false;
            }
            fileSize = sb.st_size;
        }
#endif
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> MemberBufferOrErr = llvm::MemoryBuffer::getOpenFile(fd, path, fileSize);
        if (!MemberBufferOrErr) {
            std::string msg = MemberBufferOrErr.getError().message();
            error("error reading file contents %R: %R", path, msg);
            return false;
        }
        ContentCache *cache = new ContentCache(MemberBufferOrErr->release(), it.first->getKey());
        it.first->second = cache;
        lastDir = llvm::sys::path::parent_path(path, llvm::sys::path::Style::native);
        if (lastDir.empty())
            lastDir = ".";
        goto PUSH;
    }
    const IncludeFile *searchIncludeFile(location_t loc, location_t &offset) const {
        for (const auto &entry : includeStack) {
            if (loc < entry.getEndLoc()) {
                offset = loc - entry.getStartLoc();
                return &entry;
            }
        }
        return nullptr;
    }
    unsigned getLineNumber(location_t loc) {
        location_t offset;
        const IncludeFile *entry = searchIncludeFile(loc, offset);
        if (!entry)
            return 1;
        return entry ? entry->cache->getLineNumber(offset) : 1;
    }
    unsigned getColumnNumber(location_t offset, const IncludeFile &stack) {
        const char *Buffer = stack.getBufferStart();
        unsigned lineStart = offset;
        while (lineStart && Buffer[lineStart - 1] != '\n' && Buffer[lineStart - 1] != '\r')
            --lineStart;
        return offset - lineStart + 1;
    }
    unsigned getColumnNumber(location_t loc) {
        location_t offset;
        const IncludeFile *entry = searchIncludeFile(loc, offset);
        if (!entry) return 1;
        entry->cache->createLineOffsetMapping();
        return getColumnNumber(offset, *entry);
    }
    std::pair<unsigned, unsigned> getLineAndColumn(location_t loc, const IncludeFile * &entry) const {
        location_t offset;
        entry = searchIncludeFile(loc, offset);
        if (!entry)
            return {0U, 0U};
        entry->cache->createLineOffsetMapping();
        unsigned line = entry->cache->getLineNumber(offset);
        unsigned column = offset - entry->cache->getLocation_tFromLine(line) + 1;
        return std::make_pair(line, column);
    }
    bool translateLocation(location_t loc, source_location &out, const ArrayRef<SourceRange> ranges = {}) {
        if (loc == 0)
            return false;
        location_t offset;
        const IncludeFile *fd = searchIncludeFile(loc, offset);
        if (!fd) 
            return false;
        auto it = location_map.lower_bound(loc);
        if (it == location_map.begin())
            goto NO_TREE;
        it--;
        if (it == location_map.begin())
            goto NO_TREE;
        out.tree = it->second;
        get_source(offset, out, loc, ranges, *fd);
        return true;
NO_TREE:
        out.tree = nullptr;
        get_source(offset, out, loc, ranges, *fd);
        return true;
    }
    const char *getBufferForLoc(location_t loc) const {
        location_t offset;
        const IncludeFile * entry = searchIncludeFile(loc, offset);
        if (!entry) return nullptr;
        return entry->getBufferStart() + offset;
    }
    // translate location_t to
    // 1. source line(string)
    // 2. line
    // 3. column
    static void get_source(location_t offset, source_location &out, location_t loc, const ArrayRef<SourceRange> ranges,
                           const IncludeFile &entry) {
        location_t last_line_offset = 0;
        const char *s = entry.getBufferStart();
        size_t max = entry.getFileSize();
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
    StringRef getMainFileName() { return includeStack.front().cache->getName(); }
private:
    bool searchFileInDir(xstring &result, StringRef path, xstring *dir, size_t len, location_t loc) {
        for (size_t i = 0; i < len; ++i) {
            result += dir[i];
            if (dir[i].empty() || dir[i].front() == '/' || dir[i].front() == '\\')
                result += '/';
            result += path;
            result.make_eos();
            if (addFile(result.data(), false, loc))
                return true;
            result.clear();
        }
        return false;
    }
    bool searchFileInDir(xstring &result, StringRef path, const char **dir, size_t len, location_t loc) {
        for (size_t i = 0; i < len; ++i) {
            result += dir[i];
            if (!*(dir[i]) || dir[i][0] == '/' || dir[i][0] == '\\')
                result += '/';
            result += path;
            result.make_eos();
            if (addFile(result.data(), false, loc))
                return true;
            result.clear();
        }
        return false;
    }
public:
    bool addIncludeFile(StringRef path, bool isAngled, location_t loc) {
        // https://stackoverflow.com/q/21593/15886008
        // path must be null terminated
        xstring result = xstring::get_with_capacity(256);
        if (!isAngled && (!lastDir.empty())) {
            result += lastDir;
            result += '/';
            result += path;
            if (addFile(result.data(), false, loc))
                return true;
            result.clear();
        }
        if (searchFileInDir(result, path, userPaths.data(), userPaths.size(), loc))
            return true;
        if (searchFileInDir(result, path, sysPaths.data(), sysPaths.size(), loc))
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
        note("%r", Msg);
    }
public:
    // Returns true if no source file inputs
    bool empty() const {
        return includeStack.empty();
    }
    // Retruns true if no files current reading
    bool empty_active() const {
        return grow_include_stack.empty();
    }
    // this function does not save the string, so the called must pass as constant global string or alloced somewhere.
    void addStdin(location_t includePos = 0) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> MemberBufferOrErr = llvm::MemoryBuffer::getSTDIN();
        if (!MemberBufferOrErr) {
            std::string msg = MemberBufferOrErr.getError().message();
            error("error reading from stdin: %R", msg);
            return;
        }
        ContentCache *cache = new ContentCache(MemberBufferOrErr->release(), "<stdin>");
        cached_strings.push_back(cache);
        includeStack.emplace_back(cache, getInsertPos(), includePos);
        addBuffer();
    }
    // this function does not save the string, so the called must pass as constant global string or alloced somewhere.
    void addString(StringRef s, StringRef Name = "<string>", location_t includePos = 0) {
        ContentCache *cache = new ContentCache(llvm::MemoryBuffer::getMemBuffer(s, Name, false).release());
        cache->setNameAsBufferName();
        cached_strings.push_back(cache);
        includeStack.emplace_back(cache, getInsertPos(), includePos);
        addBuffer();
    }
    void addMemoryBuffer(llvm::MemoryBuffer *Buffer, location_t includePos = 0) {
        ContentCache *cache = new ContentCache(Buffer);
        cache->setNameAsBufferName();
        cached_strings.push_back(cache);
        includeStack.emplace_back(cache, getInsertPos(), includePos);
        addBuffer();
    }
private:
    inline void setCurPtr() {
        IncludeFile file = includeStack[grow_include_stack.back()];
        cur_loc = file.getStartLoc();
        cur_buffer_ptr = file.getBufferStart();
    }
    inline void addBuffer() {
        grow_include_stack.push_back(includeStack.size() - 1);
        setCurPtr();
    }
    uint16_t lastc = 256, lastc2 = 256;
    inline char advance_next() { return cur_buffer_ptr[cur_loc++]; }
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
        if (grow_include_stack.empty()) {
            return '\0';
        } else {
            char c = advance_next();
            switch (c) {
            default: return c;
            case '\n': current_line++; return '\n';
            case '\r': {
                char c2;
                c2 = advance_next();
                current_line++;
                if (c2 != '\n')
                    lastc = (unsigned char)c;
                return '\n';
            }
            }
        }
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
public:
    bool nextFile() {
        grow_include_stack.pop_back();
        if (grow_include_stack.empty())
            return true;
        setCurPtr();
        lastc = lastc2 = 0;
        return false;
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
};
