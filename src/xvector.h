template <typename T> struct xvector {
    struct _xvector_impl {
        size_t _length, _capacity;
        T _data[0];
    } *p;
    using p_xvector_impl = _xvector_impl *;
    void free() const { std::free(reinterpret_cast<void *>(this->p)); };
    static xvector<T> get() {
        xvector<T> res;
        size_t init_size = 15;
        res.p = reinterpret_cast<p_xvector_impl>(llvm::safe_malloc(sizeof(_xvector_impl) + init_size * sizeof(T)));
        res.p->_length = 0;
        res.p->_capacity = init_size;
        return res;
    }
    static xvector<T> get_empty() { return xvector<T>{.p = nullptr}; }
    static xvector<T> get_with_capacity(size_t init_size) {
        xvector<T> res;
        res.p = reinterpret_cast<p_xvector_impl>(llvm::safe_malloc(sizeof(_xvector_impl) + init_size * sizeof(T)));
        res.p->_length = 0;
        res.p->_capacity = init_size;
        return res;
    }
    static xvector<T> from_opache_pointer(const void *Ptr) {
        xvector<T> res;
        res.p = reinterpret_cast<p_xvector_impl>(const_cast<void *>(Ptr));
        return res;
    }
    static xvector<T> get_with_length(size_t length) {
        xvector<T> res;
        res.p = reinterpret_cast<p_xvector_impl>(llvm::safe_malloc(sizeof(_xvector_impl) + length * sizeof(T)));
        res.p->_length = length;
        res.p->_capacity = length;
        return res;
    }
    void clear() { p->_length = 0; }
    size_t size() const { return p->_length; }
    size_t &msize() { return p->_length; }
    size_t length() const { return p->_length; }
    size_t size_in_bytes() const { return p->_length * sizeof(_xvector_impl) * sizeof(T); }
    size_t capacity() const { return p->_capacity; }
    const T *data() const { return p->_data; }
    T *data() { return p->_data; }
    void erase(const T *it) {
        assert(it >= begin() && it < end() && "Iterator to erase is out of bounds.");
        std::move(const_cast<T *>(it) + 1, this->end(), const_cast<T *>(it));
        pop_back();
    }
    const T *find(const T &elem) const {
        for (const T &it = begin(); it != end(); ++it) {
            if (*it == elem)
                return &it;
        }
        return nullptr;
    }
    T *find(const T &elem) { return const_cast<const xvector<T>>(*this)->find(elem); }
    T operator[](size_t Index) const {
        assert(Index < p->_length && "Index too large!");
        return p->_data[Index];
    }
    T &operator[](size_t Index) {
        assert(Index < p->_length && "Index too large!");
        return p->_data[Index];
    }
    void grow() {
        if (p->_length >= p->_capacity) {
            ++p->_capacity;
            p->_capacity *= 3;
            p->_capacity /= 2;
            p = reinterpret_cast<p_xvector_impl>(
                llvm::safe_realloc(p, sizeof(_xvector_impl) + p->_capacity * sizeof(T)));
        }
    }
    void append(const void *src, size_t num) {
        auto Pos = end();
        p->_length += num;
        grow();
        memcpy(Pos, src, num);
    }
    void append(const T &c, size_t num) {
        auto Ptr = p->_data + p->_length;
        p->_length += num;
        grow();
        for (size_t i = 0; i < num; ++i)
            Ptr[i] = c;
    }
    void push_back(const T &c) {
        grow();
        p->_data[p->_length++] = c;
    }
    void push_back(T &&c) {
        grow();
        p->_data[p->_length++] = std::move(c);
    }
    void pop_back() {
        assert(p->_length && "string empty!");
        --p->_length;
    }
    bool empty() const { return p->_length == 0; }
    const T &front() const { return *p->_data; }
    T &front() { return *p->_data; }
    const T &back() const { return p->_data[p->_length - 1]; }
    T &back() { return p->_data[p->_length - 1]; }
    const T *begin() const { return p->_data; };
    T *begin() { return p->_data; }
    const T *end() const { return p->_data + p->_length; }
    T *end() { return p->_data + p->_length; }
};

static_assert(sizeof(xvector<int>) == sizeof(void *), "sizeof xvector<T> should equals to sizeof pointer");
