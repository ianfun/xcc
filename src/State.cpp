#define TNEW(TY) (CType) new (getAllocator()) TY
#define SNEW(TY) (Stmt) new (getAllocator()) TY
#define ENEW(TY) (Expr) new (getAllocator()) TY

struct xcc_context {
    xcc_context(const xcc_context &) = delete;
    xcc_context()
        : table{},
          constint{make(TYCONST | TYINT)},
        b{make(TYBOOL)}, v{make(TYVOID)}, i8{make(TYINT8)}, u8{make(TYUINT8)}, i16{make(TYINT16)},
        u16{make(TYINT16)}, i32{make(TYINT32)}, u32{make(TYUINT32)}, i64{make(TYINT64)}, u64{make(TYUINT64)},
        i128{make(TYINT128)}, u128{make(TYUINT128)}, ffloatty{make(TYFLOAT)}, fdoublety{make(TYDOUBLE)},
        f128ty{make(TYF128)}
    {
        _wchar = u16;
        _long = i64;
        _ulong = u64;
        _uchar = u32;
        str32ty = getPointerType(_uchar);
        str16ty = getPointerType(_wchar);
        str8ty = getPointerType(getChar());
        _longdouble = getFloat128();
        _complex_f128 = make(TYF128 | TYCOMPLEX);
        _complex_longdouble = getComplexType(_longdouble->tags);
        _complex_double = getComplexType(TYDOUBLE);
        _complex_float = getComplexType(TYFLOAT);
    }
    IdentifierTable table; // contains allocator!
    Expr intzero;
    CType constint, b, v, i8, u8, i16, u16, i32, u32, i64, u64, i128, u128, 
    ffloatty, fdoublety, f128ty, str8ty, str16ty, str32ty, _complex_float, _complex_double, 
    _complex_longdouble, _complex_f128, _wchar, _long, _ulong, _longlong, _ulonglong, _longdouble, _uchar;
    CType make(uint32_t tag) { return TNEW(PrimType){.align = 0, .tags = tag}; }
    CType getPointerType(CType base) { return TNEW(PointerType){.align = 0, .tags = 0, .p = base}; }
    CType getFixArrayType(CType elementType, unsigned size) {
        return TNEW(ArrayType){.align = 0, .tags = 0, .arrtype = elementType, .hassize = true, .arrsize = size};
    }
    CType getComplexType(const_CType ty) {
        return make(ty->tags | TYCOMPLEX);
    }
    CType getComplexType(uint32_t tags) {
        return make(tags | TYCOMPLEX);
    }
    CType getComplexFloat() {
        return _complex_float;
    }
    CType getComplexDouble() {
        return _complex_double;
    }
    CType getComplexLongDouble() {
        return _complex_longdouble;
    }
    CType getComplexFloat128() {
        return _complex_f128;
    }
    CType getFloat() {
        return ffloatty;
    }
    CType getDobule() {
        return fdoublety;
    }
    CType getLongDobule() {
        return _longdouble;
    }
    CType getFloat128() {
        return f128ty;
    }
    CType getBool() {
        return b;
    }
    CType getConstInt() { return constint; }
    CType getInt() { return i32; }
    CType getUInt() { return u32; }
    CType getVoid() {
        return v;
    }
    CType getChar() {
        return i8;
    }
    uint32_t getLongDoubleTag() {
        return _longdouble->tags;
    }
    uint32_t getLongTag() {
        return _long->tags;
    }
    uint32_t getULongTag() {
        return _ulong->tags;
    }
    uint32_t getUCharTag() {
        return _uchar->tags;
    }
    uint32_t getWcharTag() {
        return _uchar->tags;
    }
    CType getChar8_t() {
        return u8;
    }
    CType getChar16_t() {
        return u16;
    }
    CType getChar32_t() {
        return u32;
    }
    CType getLong() {
        return _long;
    }
    CType getULong() {
        return _ulong;
    }
    CType getWchar() {
        return _wchar;
    }
    CType getUChar() {
        return _uchar;
    }
    CType getShort() {
        return i16;
    }
    CType getUShort() {
        return u16;
    }
    size_t getsizeof(CType ty) { return 0; }
    size_t getsizeof(Expr e) { return getsizeof(e->ty); }
    size_t getAlignof(CType ty) { return 0; }
    size_t getAlignof(Expr e) { return getAlignof(e->ty); }
    CType getIntPtr() { return i64; }
    CType getPtrDiff() { return getIntPtr(); }
    CType getSize_t() { return u64; }
    CType getLongLong() { return u64; }
    CType getULongLong() { return i64; }
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
};
#undef TNEW
#undef SNEW
#undef ENEW
#define TNEW(TY) (CType) new (context.getAllocator()) TY
#define SNEW(TY) (Stmt) new (context.getAllocator()) TY
#define ENEW(TY) (Expr) new (context.getAllocator()) TY
