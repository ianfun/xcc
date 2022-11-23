struct raw_xvector_stream: public raw_pwrite_stream {
    xvector &_vec;
    void write_impl(const char *PTr, size_t Size, uint64_t Offset) override {
        memcpy(_vec.data() + Offset, Ptr, Size);
    }
    void write_impl(const char *Ptr, size_t Size) override {
        _vec.append(Ptr, Size);
    }
    uint64_t current_pos() const override {
        return _vec.size();
    }
    void flush() = delete;
    StringRef str() {
        return StringRef(_vec.data(), _vec.size());
    }
    raw_xvector_stream(xvector &O): _vec{O} {
        SetUnBuffered();
    }
    ~raw_xvector_stream() override = default;
};
