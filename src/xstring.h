/*
xstring - a flexible pointer structure that containing a string buffer.

xstring is designed to be as big as pointers to make shadow copy (MOV) fast.
*/

struct xstring {
    struct _xstring_impl {
        size_t _length, _capacity;
        char _data[0];
    } *p;
    void free() const { std::free(reinterpret_cast<void *>(this->p)); };
    static xstring get() {
        xstring res;
        size_t init_size = 15;
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + init_size));
        res.p->_length = 0;
        res.p->_capacity = init_size;
        return res;
    }
    static xstring get_empty() { return xstring{.p = nullptr}; }
    static xstring get_from_char(char c) {
        xstring res;
        size_t init_size = 15;
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + init_size));
        res.p->_length = 1;
        res.p->_capacity = init_size;
        res.p->_data[0] = c;
        return res;
    }
    static xstring get_with_capacity(size_t init_size) {
        xstring res;
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + init_size));
        res.p->_length = 0;
        res.p->_capacity = init_size;
        return res;
    }
    static xstring get_with_length(size_t length) {
        xstring res;
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + length));
        res.p->_length = length;
        res.p->_capacity = length;
        return res;
    }
    static xstring get(const char *s) {
        xstring res;
        size_t len = std::strlen(s);
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + len));
        res.p->_length = 0;
        res.p->_capacity = len;
        memcpy(reinterpret_cast<void *>(&res.p->_data), reinterpret_cast<const void *>(s), len);
        return res;
    }
    static xstring get(llvm::StringRef s) {
        xstring res;
        size_t len = s.size();
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + len));
        res.p->_length = 0;
        res.p->_capacity = len;
        memcpy(reinterpret_cast<void *>(&res.p->_data), s.data(), len);
        return res;
    }
    static xstring get(const std::string &s) {
        xstring res;
        size_t len = s.size();
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + len));
        res.p->_length = 0;
        res.p->_capacity = len;
        memcpy(reinterpret_cast<void *>(&res.p->_data), s.data(), len);
        return res;
    }
#if __cplusplus > 201402L
    static xstring get(std::string_view s) {
        xstring res;
        size_t len = s.size();
        res.p = reinterpret_cast<_xstring_impl *>(llvm::safe_malloc(sizeof(_xstring_impl) + len));
        res.p->_length = 0;
        res.p->_capacity = len;
        memcpy(reinterpret_cast<void *>(&res.p->_data), s.data(), len);
        return res;
    }
#endif
    static xstring get(const llvm::Twine &s) {
        llvm::SmallString<32> buf;
        return xstring::get(s.toStringRef(buf));
    }
    void clear() { p->_length = 0; }
    size_t size() const { return p->_length; }
    size_t &msize() { return p->_length; }
    size_t length() const { return p->_length; }
    size_t size_in_bytes() const { return p->_length * sizeof(_xstring_impl); }
    size_t capacity() const { return p->_capacity; }
    // return maybe not null-termiated string
    const char *data() const { return p->_data; }
    char *data() { return p->_data; }
    // add '\0', and return null-termiated string
    char *c_str() {
        add('\0');
        return p->_data;
    }
    StringRef str() const { return StringRef(p->_data, p->_length); }
    operator StringRef() const { return str(); }
    operator std::string() const { return std::string(p->_data, p->_length); }
    operator const char *() const { return data(); }
    size_t count(char C) const { return str().count(C); }
    bool equals(StringRef RHS) const { return str().equals(RHS); }
    char operator[](size_t Index) const {
        assert(Index < p->_length && "Index too large!");
        return p->_data[Index];
    }
    char &operator[](size_t Index) {
        assert(Index < p->_length && "Index too large!");
        return p->_data[Index];
    }
    void add(char c) {
        if (p->_length >= p->_capacity) {
            ++p->_capacity;
            p->_capacity *= 3;
            p->_capacity /= 2;
            p = reinterpret_cast<_xstring_impl *>(llvm::safe_realloc(p, sizeof(_xstring_impl) + p->_capacity));
        }
        p->_data[p->_length++] = c;
    }
    void add(StringRef s) {
        size_t len = s.size();
        size_t after = len + p->_length;
        if (after > p->_capacity) {
            p->_capacity += len;
            p = reinterpret_cast<_xstring_impl *>(llvm::safe_realloc(p, sizeof(_xstring_impl) + p->_capacity));
        }
        memcpy(p->_data + p->_length, s.data(), len);
        p->_length = after;
    }
    void add(const char *s) {
        size_t len = strlen(s);
        return add(StringRef(s, len));
    }
    inline void push_back(char c) { return add(c); }
    inline void push_back(const char *s) { return add(s); }
    inline void push_back(xstring other) { return add(other.str()); }
    inline void push_back(StringRef s) { return add(s); }
    inline void operator+=(char c) { return add(c); }
    inline void operator+=(const char *s) { return add(s); }
    inline void operator+=(xstring other) { return add(other.str()); }
    inline void operator+=(StringRef s) { return add(s); }
    void pop_back() {
        assert(p->_length && "string empty!");
        --p->_length;
    }
    bool empty() const { return p->_length == 0; }
    char front() const { return *p->_data; }
    char &front() { return *p->_data; }
    char back() const { return p->_data[p->_length - 1]; }
    char &back() { return p->_data[p->_length - 1]; }
    const char *begin() const { return p->_data; };
    char *begin() { return p->_data; }
    const char *end() const { return p->_data + p->_length; }
    char *end() { return p->_data + p->_length; }
    void lower_inplace() {
        for (size_t i = 0; i < p->_length; ++i)
            p->_data[i] = std::tolower(p->_data[i]);
    }
    void upper_inplace() {
        for (size_t i = 0; i < p->_length; ++i)
            p->_data[i] = std::toupper(p->_data[i]);
    }
    void make_eos() { add('\0'); }
};

static_assert(sizeof(xstring) == sizeof(void *), "sizeof xstring should equals to sizeof pointer");
