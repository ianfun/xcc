template <typename T, unsigned InitialSize>
struct ScopeBase {
    struct Storage {
        IdentRef sym;
        T info;
    };
    SmallVector<Storage, InitialSize> data;
    using iterator = Storage*;
    using const_iterator = const Storage*;
    iterator begin() {
        return data.data();
    }
    const_iterator begin() const {
        return data.data();
    }
    iterator end() {
        return data.data() + data.size();
    }
    const_iterator end() const {
        return data.data() + data.size();
    }
    T *getSym(IdentRef sym, size_t &idx) {
        for (size_t i = data.size();i--;) {
            Storage &elem = data[i];
            if (elem.sym == sym)
                return idx = i, &elem.info;
        }
        return nullptr;
    }
    T *getSym(IdentRef sym) {
        for (size_t i = data.size();i--;) {
            Storage &elem = data[i];
            if (elem.sym == sym)
                return &elem.info;
        }
        return nullptr;
    }
    T &getSym(size_t index) {
        return data[index].info;
    }
    Storage &getSymFull(size_t index) {
        return data[index];
    }
    StringRef &getSymName(size_t index) {
        return data[index];
    }
    size_t putSym(IdentRef sym, const T &Elt) {
        size_t size = data.size();
        Storage s;
        s.sym = sym;
        s.info = Elt;
        data.push_back(s);
        return size;
    }
    bool contains(IdentRef sym) {
        for (size_t i = data.size() - 1;i--;) 
            if (data[i].sym == sym)
                return true;
        return false;
    }
    size_t numSyms() const {
        return data.size();
    }
    void resize_for_overwrite(size_t N) {
        data.resize_for_overwrite(N);
    }
};
template <typename T, unsigned InitialSize = 64>
struct BlockScope: public ScopeBase<T, InitialSize>
{
    // https://stackoverflow.com/q/1120833
    // Derived template-class access to base-class member-data
    SmallVector<unsigned, 8> blocks;
    unsigned maxSyms;
    size_t numSymsThisBlock() const {
        return blocks.back();
    }
    void push() {
        blocks.push_back(this->data.size());
    }
    void pop() {
        maxSyms = std::max(maxSyms, blocks.back());
        this->data.truncate(blocks.back());
        blocks.pop_back();
    }
    auto current_block() {
        return this->data.data() + numSymsThisBlock();
    }
    auto current_block() const {
        return this->data.data() + numSymsThisBlock();
    }
    T *getSymInCurrentScope(IdentRef Name) {
        for (size_t i = numSymsThisBlock();i < this->data.size();++i) {
            auto &elem = this->data[i];
            if (elem.sym == Name)
                return &elem.info;
        }
        return nullptr;
    }
    T *getSymInCurrentScope(IdentRef Name, size_t &idx) {
        for (size_t i = numSymsThisBlock();i < this->data.size();++i) {
            auto &elem = this->data[i];
            if (elem.sym == Name)
                return idx = i, &elem.info;
        }
        return nullptr;
    }
};
