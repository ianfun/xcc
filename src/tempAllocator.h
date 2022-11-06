// tempAllocator - alloc from object lives young.
// this is the replacement for `alloca` and variable-length-arrays(VLA), to prevent from stack overflows

struct OnceAllocator {
    static constexpr size_t init_size = 0xFF;
    size_t buf_size;
    void *buf;

#if WINDOWS
    HANDLE heap;
    OnceAllocator() : buf_size{init_size}, heap{HeapCreate(HEAP_NO_SERIALIZE | HEAP_GROWABLE, 0, 0)} {
        buf = ::HeapAlloc(heap, 0, buf_size);
    };
    ~OnceAllocator() {
        ::HeapFree(heap, 0, buf);
        ::HeapDestroy(heap);
    };
#else
    OnceAllocator() : buf_size{init_size} { buf = llvm::safe_malloc(buf_size); }
    ~OnceAllocator() { ::free(buf); };
#endif
    template <typename T> inline T *Allocate(size_t Size) {
        return reinterpret_cast<T *>(AllocateInternal(Size * sizeof(T)));
    }
    LLVM_ATTRIBUTE_RETURNS_NONNULL void *AllocateInternal(size_t Size) {
        if (buf_size < Size) {
            buf_size = Size * 3 / 2;
#if WINDOWS
            buf = HeapReAlloc(heap, 0, buf, buf_size);
#else
            buf = llvm::safe_realloc(buf, buf_size);
#endif
        }
        return buf;
    }
};
