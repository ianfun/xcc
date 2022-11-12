#define TNEW(TY) (CType) new (getAllocator()) TY
#define SNEW(TY) (Stmt) new (getAllocator()) TY
#define ENEW(TY) (Expr) new (getAllocator()) TY

struct xcc_context {
    xcc_context(const xcc_context &) = delete;
    xcc_context()
        : table{}, 
          constint{reinterpret_cast<CType>((TNEW(PrimType){.align = 0, .tags = TYINT | TYCONST}))}, typecache {
        .b = make(TYBOOL), .v = make(TYVOID), .i8 = make(TYINT8), .u8 = make(TYUINT8), .i16 = make(TYINT16),
        .u16 = make(TYINT16), .i32 = make(TYINT32), .u32 = make(TYUINT32), .i64 = make(TYINT64), .u64 = make(TYUINT64),
        .i128 = make(TYINT128), .u128 = make(TYUINT128), .ffloatty = make(TYFLOAT), .fdoublety = make(TYDOUBLE),
        .f128ty = make(TYF128), .str8ty = getPointerType(make(TYINT8)),
#if CC_WCHAR32
        .str16ty = getPointerType(make(TYINT32)),
#else
        .str16ty = getPointerType(make(TYINT16)),
#endif
        .str32ty = getPointerType(make(TYUINT32))
    }
    { }
    IdentifierTable table; // contains allocator!
    Expr intzero;
    CType constint;
    struct TypeCache {
        CType b, v, i8, u8, i16, u16, i32, u32, i64, u64, i128, u128, ffloatty, fdoublety, f128ty, str8ty, str16ty,
            str32ty;
    } typecache;
    CType make(uint32_t tag) { return TNEW(PrimType){.align = 0, .tags = tag}; }
    CType getPointerType(CType base) { return TNEW(PointerType){.align = 0, .tags = 0, .p = base}; }
    CType getFixArrayType(CType elementType, unsigned size) {
        return TNEW(ArrayType){.align = 0, .tags = 0, .arrtype = elementType, .hassize = true, .arrsize = size};
    }
    CType getConstInt() { return constint; }
    CType getInt() { return typecache.i32; }
    CType getUInt() { return typecache.u32; }
    CType getLong() {
        if (TYLONG == TYINT64)
            return typecache.i64;
        return typecache.i32;
    }
    CType getULong() {
        if (TYLONG == TYINT64)
            return typecache.u64;
        return typecache.u32;
    }
    CType getChar() {
        return typecache.i8;
    }
    CType getWchar() {
#if CC_WCHAR32
        return typecache.i32;
#else
        return typecache.i16;
#endif
    }
    size_t getsizeof(CType ty) { return 0; }
    size_t getsizeof(Expr e) { return getsizeof(e->ty); }
    size_t getAlignof(CType ty) { return 0; }
    size_t getAlignof(Expr e) { return getAlignof(e->ty); }
    CType getIntPtr() { return typecache.i64; }
    CType getPtrDiff() { return getIntPtr(); }
    CType getSize_t() { return typecache.u64; }
    CType getLongLong() { return typecache.u64; }
    CType getULongLong() { return typecache.i64; }
    ArenaAllocator &getAllocator() { return table.getAllocator(); }
    void *new_memcpy(size_t size, const void *src) {
        void *mem = getAllocator().Allocate(size, 1);
        (void)memcpy(mem, src, size);
        return mem;
    }
    CType clone(CType ty) { return reinterpret_cast<CType>(new_memcpy(ctype_size_map[ty->k], ty)); }
    Stmt clone(Stmt s) { return reinterpret_cast<Stmt>(new_memcpy(stmt_size_map[s->k], s)); }
    Expr clone(Expr e) { return reinterpret_cast<Expr>(new_memcpy(expr_size_map[e->k], e)); }
    PPMacroDef *newMacro() {
        return new (getAllocator()) PPMacroDef();
    }
    FullSourceLoc *getFullLoc(Location loc) {
        FullSourceLoc *res = new (getAllocator().Allocate(sizeof(FullSourceLoc), 1)) FullSourceLoc;
        res->loc = loc;
        res->num_macros = 0;
        res->num_stack = 0;
        return res;
    }
    FullSourceLoc *getFullLoc(Location loc, SourceMgr &SM, std::vector<PPMacroDef*> &macros) {
        unsigned inc_size = SM.includeStack.size();
        unsigned macro_size = macros.size();
        printf("macro_size = %u\n", macro_size);
        if (inc_size == 0)
            inc_size = 1;
        --inc_size;
        FullSourceLoc *res = new (getAllocator().Allocate(sizeof(FullSourceLoc) + inc_size * sizeof(Include_Info) + macro_size * sizeof(struct PPMacroDef *), 1)) FullSourceLoc;
        res->loc = loc;
        res->num_stack = inc_size;
        res->num_macros = macro_size;
        for (unsigned i = 0; i < inc_size;++i) {
            fileid_t it = SM.includeStack[i];
            Stream &f = SM.streams[it];
            res->include_stack[i] = Include_Info{.line = f.line, .fd = it };
        }
        for (unsigned i = 0; i < macro_size;++i)
            res->macros[i] = macros[i];
        return res;
    }
};
#undef TNEW
#undef SNEW
#undef ENEW
#define TNEW(TY) (CType) new (context.getAllocator()) TY
#define SNEW(TY) (Stmt) new (context.getAllocator()) TY
#define ENEW(TY) (Expr) new (context.getAllocator()) TY
