template <typename T> struct ScopeBase {
    struct Storage {
        IdentRef sym;
        T info;
    };
    std::vector<Storage> data{};
    using iterator = Storage *;
    using const_iterator = const Storage *;
    iterator begin() { return data.data(); }
    const_iterator begin() const { return data.data(); }
    iterator end() { return data.data() + data.size(); }
    const_iterator end() const { return data.data() + data.size(); }
    T *getSym(IdentRef sym, unsigned &idx) {
        for (size_t i = data.size(); i--;) {
            Storage &elem = data[i];
            if (elem.sym == sym)
                return idx = i, &elem.info;
        }
        return nullptr;
    }
    T *getSym(IdentRef sym) {
        for (size_t i = data.size(); i--;) {
            Storage &elem = data[i];
            if (elem.sym == sym)
                return &elem.info;
        }
        return nullptr;
    }
    T &getSym(unsigned index) { return data[index].info; }
    Storage &getSymFull(unsigned index) { return data[index]; }
    StringRef getSymName(unsigned index) { return data[index].sym->getKey(); }
    unsigned putSym(IdentRef sym, const T &Elt) {
        unsigned size = data.size();
        Storage s;
        s.sym = sym;
        s.info = Elt;
        data.push_back(s);
        return size;
    }
    bool contains(IdentRef sym) {
        for (size_t i = data.size() - 1; i--;)
            if (data[i].sym == sym)
                return true;
        return false;
    }
    size_t numSyms() const { return data.size(); }
    void resize_for_overwrite(size_t N) { data.resize_for_overwrite(N); }
};
template <typename T> struct BlockScope : public ScopeBase<T> {
    // https://stackoverflow.com/q/1120833
    // Derived template-class access to base-class member-data
    SmallVector<unsigned, 8> blocks{};
    unsigned maxSyms = 0;
    unsigned numSymsThisBlock() const { return blocks.back(); }
    void push() { blocks.push_back(this->data.size()); }
    void pop() {
        assert(blocks.size() && "mismatched push/pop: no stack to pop()!");
        maxSyms = std::max(static_cast<size_t>(maxSyms), this->data.size());
        this->data.resize(blocks.back());
        blocks.pop_back();
    }
    void finalizeGlobalScope() { maxSyms = std::max(static_cast<size_t>(maxSyms), this->data.size()); }
    bool isInGlobalScope(IdentRef Name) {
        assert(blocks.size() && "no global scope");
        for (unsigned i = 0; i < blocks.size(); i++) {
            if (this->data[i].sym == Name)
                return true;
        }
        return false;
    }
    bool isInGlobalScope(unsigned idx) {
        assert(blocks.size() && "no global scope");
        return idx < blocks.front();
    }
    auto current_block() { return this->data.data() + numSymsThisBlock(); }
    auto current_block() const { return this->data.data() + numSymsThisBlock(); }
    T *getSymInCurrentScope(IdentRef Name) {
        for (size_t i = numSymsThisBlock(); i < this->data.size(); ++i) {
            auto &elem = this->data[i];
            if (elem.sym == Name)
                return &elem.info;
        }
        return nullptr;
    }
    T *getSymInCurrentScope(IdentRef Name, unsigned &idx) {
        for (size_t i = numSymsThisBlock(); i < this->data.size(); ++i) {
            auto &elem = this->data[i];
            if (elem.sym == Name)
                return idx = i, &elem.info;
        }
        return nullptr;
    }
    bool containsInCurrentScope(IdentRef Name) {
        for (size_t i = numSymsThisBlock(); i < this->data.size(); ++i)
            if (this->data[i].sym == Name)
                return true;
        return false;
    }
};
template <typename T> struct FunctionAndBlockScope : public BlockScope<T> {
    unsigned _current_function_offset;
    auto current_function() { return this->data.data() + _current_function_offset; }
    auto current_function() const { return this->data.data() + _current_function_offset; }
    void push_function() { _current_function_offset = this->data.size(); }
    void pop_function() const { }
};
