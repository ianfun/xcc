#define TNEW(TY) (CType)new(getAllocator())TY
#define SNEW(TY) (Stmt)new(getAllocator())TY
#define ENEW(TY) (Expr)new(getAllocator())TY

struct xcc_context {
    xcc_context(xcc_context&) = delete;
    xcc_context(struct TextDiagnosticPrinter *printer = nullptr) : table{}, printer{printer},  
    typecache {
      .b=make(TYBOOL),
      .v=make(TYVOID),
      .i8=make(TYINT8),
      .u8=make(TYUINT8),
      .i16=make(TYINT16),
      .u16=make(TYINT16),
      .i32=make(TYINT32),
      .u32=make(TYUINT32),
      .i64=make(TYINT64),
      .u64=make(TYUINT64),
      .i128=make(TYINT128),
      .u128=make(TYUINT128),
      .ffloatty=make(TYFLOAT),
      .fdoublety=make(TYDOUBLE),
      .f128ty=make(TYF128),
      .strty=getPointerType(make(TYINT8)) 
    }
    {
        intzero = ENEW(IntLitExpr) {.loc = Location(), .ty = typecache.i32, .ival = APInt::getZero(32)};
    }
    IdentifierTable table; // contains allocator!
    struct TextDiagnosticPrinter *printer;
    Expr intzero;
    void setPrinter(struct TextDiagnosticPrinter *thePrinter) {
        printer = thePrinter;
    }
    Expr getIntZero() {
        return intzero;
    }
    struct TypeCache {
        CType 
            b, v, i8, u8, i16, u16, i32, u32, i64, u64, i128, u128,
            ffloatty, fdoublety, f128ty,
            strty;
    } typecache;
    CType make(uint32_t tag) {
        return (CType)(TNEW(PrimType) {.align = 0, .tags = tag});
    }
    CType getPointerType(CType base) {
        return (CType)(TNEW(PointerType) {.align = 0, .p = base});
    }
    CType getInt() {
        return typecache.i32;
    }
    CType getUInt(){
        return typecache.u32;
    }
    CType getLong(){
        if (TYLONG == TYINT64)
            return typecache.i64;
        return typecache.i32;
    }
    CType getULong(){
        if (TYLONG == TYINT64)
            return typecache.u64;
        return typecache.u32;
    }
    CType getWchar(){
#if CC_WCHAR32
          return typecache.i32;
#else
          return typecache.i16;
#endif
    }
    size_t getsizeof(CType ty) {
        return 0;
    }
    size_t getsizeof(Expr e) {
        return getsizeof(e->ty);
    }
    size_t getAlignof(CType ty) {
        return 0;
    }
    size_t getAlignof(Expr e) {
        return getAlignof(e->ty);
    }
    CType getIntPtr(){
        return typecache.i64;
    }
    CType getPtrDiff(){
        return getIntPtr();
    }
    CType getSize_t(){
        return typecache.u64;
    }
    CType getLongLong() {
        return typecache.u64;
    }
    CType getULongLong() {
        return typecache.i64;
    }
    ArenaAllocator &getAllocator() {
        return table.getAllocator();
    }
    void *new_memcpy(size_t size, const void *src) {
        void *mem = getAllocator().Allocate(size, 1);
        (void)memcpy(mem, src, size);
        return mem;
    }
    CType clone(CType ty) {
        return reinterpret_cast<CType>(new_memcpy(ctype_size_map[ty->k], ty));
    }
    Stmt clone(Stmt s) {
        return reinterpret_cast<Stmt>(new_memcpy(stmt_size_map[s->k], s));
    }
    Expr clone(Expr e) {
        return reinterpret_cast<Expr>(new_memcpy(expr_size_map[e->k], e));
    }
};
#undef TNEW
#undef SNEW
#undef ENEW
#define TNEW(TY) (CType)new(context.getAllocator())TY
#define SNEW(TY) (Stmt)new(context.getAllocator())TY
#define ENEW(TY) (Expr)new(context.getAllocator())TY
