// tempAllocator - temporary object Allocator
// It is replacement of VLA and allocas, much safer to allocate large arrays

struct OnceAllocator {
    size_t buf_size;
    void *buf;
    OnceAllocator(size_t  init_size = sizeof(void*) * 10) : buf_size{init_size}, buf{::malloc(buf_size)} {
        assert(init_size && "zero size is invalid");
        if (!buf)
            llvm::report_bad_alloc_error("malloc() failed");
    }
    ~OnceAllocator() { ::free(buf); }
    template <typename T> inline 
    LLVM_ATTRIBUTE_RETURNS_NONNULL T *Allocate(size_t Size) {
        AllocateInternal(Size * sizeof(T));
        return reinterpret_cast<T *>(buf);
    }
    void AllocateInternal(size_t Size) {
        if (buf_size < Size) {
            buf_size = Size * 3 / 2;
            buf = ::realloc(buf, buf_size);
            if (!buf)
                llvm::report_bad_alloc_error("realloc() failed");
        }
    }
};
