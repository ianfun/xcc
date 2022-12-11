static_assert(!(CC_ARENA_BLOCK_SIZE % 4096),
              "CC_ARENA_BLOCK_SIZE should larger than page size, and larger than max size of Stmt, Expr, CType");

struct arena_block {
    size_t offset;
    struct arena_block *next;
    char heap[CC_ARENA_BLOCK_SIZE];
};
static struct arena_block *new_arena_block() {
    auto it = reinterpret_cast<struct arena_block *>(llvm::safe_malloc(sizeof(struct arena_block)));
    it->next = nullptr;
    it->offset = 0;
    return it;
}
struct ArenaAllocator : public llvm::AllocatorBase<ArenaAllocator> {
    void Reset() const { }
    void PrintStats() const { }
    struct arena_block *head, *cur;
#if CC_DEBUG
    size_t allocated_bytes = 0, num_blocks = 1;
#endif

    LLVM_ATTRIBUTE_RETURNS_NONNULL void *Allocate(size_t Size, size_t) {

#if CC_DEBUG
        allocated_bytes += Size;
#endif
        if (cur->offset + Size > CC_ARENA_BLOCK_SIZE) {
#if CC_DEBUG
            // dbgprint("ArenaAllocator: new_arena_block\n");
            ++num_blocks;
#endif
            struct arena_block *n = new_arena_block();
            cur->next = n;
            cur = n;
        }
        void *mem = &cur->heap[cur->offset];
        cur->offset += Size;
        return mem;
    }

    using AllocatorBase<ArenaAllocator>::Allocate;

    void Deallocate(const void *, size_t, size_t) { }

    using AllocatorBase<ArenaAllocator>::Deallocate;

    ~ArenaAllocator() {
#if CC_DEBUG
        statics("ArenaAllocator statics\n");
        statics("  - Number of blocks: %zu\n", num_blocks);
        statics("  - Total bytes allocated: %zu\n", allocated_bytes);
        endStatics();
#endif
        struct arena_block *p = head;
        do {
            struct arena_block *tmp = p;
            p = p->next;
            // dbgprint("  block %p(size = %zu)\n", tmp, tmp->offset);
            free(tmp);
        } while (p);
    }
    ArenaAllocator() { head = cur = new_arena_block(); }
};

} // end namespace xcc

void *operator new(size_t Size, xcc::ArenaAllocator &Allocator) {
    return Allocator.Allocate(Size, alignof(std::max_align_t));
}
void *operator new[](size_t Size, xcc::ArenaAllocator &Allocator) {
    return Allocator.Allocate(Size, alignof(std::max_align_t));
}

namespace xcc {
