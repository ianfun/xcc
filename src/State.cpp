#define TNEW(TY) (CType) new (getAllocator()) TY
#define SNEW(TY) (Stmt) new (getAllocator()) TY
#define ENEW(TY) (Expr) new (getAllocator()) TY

struct xcc_context {
    xcc_context(const xcc_context &) = delete;
    xcc_context(): table{}
    {
        constint = makeSigned(32, TYCONST);
        b = makeUnsigned(1);

        i8 = makeSigned(8);
        i16 = makeSigned(16);
        i32 = makeSigned(32);
        i64 = makeSigned(64);
        i128 = makeSigned(128);

        u8 = makeUnsigned(8);
        u16 = makeUnsigned(16);
        u32 = makeUnsigned(32);
        u64 = makeUnsigned(64);
        u128 = makeUnsigned(128);

        fhalfty = makeFloating(TYHALF);
        ffloatty = makeFloating(TYFLOAT);
        fdoublety = makeFloating(TYDOUBLE);
        f80ty = makeFloating(TYF80);
        f128ty = makeFloating(TYPPC_128);
        ppc_f128 = makeFloating(TYPPC_128);

        _wchar = u16;
        _long = i64;
        _ulong = u64;
        _uchar = u32;
        str32ty = getPointerType(_uchar);
        str16ty = getPointerType(_wchar);
        str8ty = getPointerType(getChar());
        _longdouble = getFloat128();
        _complex_f128 = makeComplexFloatingType(TYF128 | TYCOMPLEX);
        _complex_longdouble = makeComplexFloatingType(_longdouble->tags);
        _complex_double = makeComplexFloatingType(TYDOUBLE);
        _complex_float = makeComplexFloatingType(TYFLOAT);
    }
    IdentifierTable table; // contains allocator!
    Expr intzero;
    CType constint, b, v, i8, u8, i16, u16, i32, u32, i64, u64, i128, u128, 
    ffloatty, fdoublety, fhalfty, f128ty, ppc_f128, f80ty, str8ty, str16ty, str32ty, _complex_float, _complex_double, 
    _complex_longdouble, _complex_f128, _wchar, _long, _ulong, _longlong, _ulonglong, _longdouble, _uchar;
    CType makeUnsigned(uint64_t Size, uint64_t tag = 0) {
        CType result = TNEW(PrimType)(tag);
        result->setKind(TYPRIM);
        result->setIntergerBithWidth(Size);
        return result;
    }
    CType makeSigned(uint64_t Size, uint64_t tag = 0) {
        CType result = TNEW(PrimType)(tag | OpacheCType::getSignedBit());
        result->setKind(TYPRIM);
        result->setIntergerBithWidth(Size);
        return result;
    }
    CType makeFloating(FloatKind k, uint64_t tag = 0) {
        CType result = TNEW(PrimType)(tag | OpacheCType::getFloatingBit());
        result->setKind(TYPRIM);
        result->setFloatKind(k);
        return result;
    }
    CType getPointerType(CType base) {
        CType result = TNEW(PointerType){.tags = 0, .p = base};
        result->setKind(TYPOINTER);
        return result;
    }
    CType getFixArrayType(CType elementType, unsigned size) {
        CType result = TNEW(ArrayType){.tags = 0, .arrtype = elementType, .hassize = true, .arrsize = size};
        result->setKind(TYARRAY);
        return result;
    }
    CType makeComplexFloatingType(const_CType ty) {
        return makeFloating(ty->tags | TYCOMPLEX, ty->getFloatKind());
    }
    CType makeComplexSignedType(cosnt_CType ty) {
        return makeSigned(ty->tags | TYCOMPLEX, ty->getBitWidth());
    }
    CType makeComplexUnsignedType(const_CType ty) {
        return makeUnsigned(ty->tags | TYCOMPLEX, ty->getBitWidth());
    }
    CType makeComplexType(const_CType ty) {
        assert(ty->getKind() == TYPRIM);
        if (is->isFloating())
            return makeComplexFloatingType(ty);
        if (ty->isSigned())
            return makeComplexSignedType(ty);
        return makeComplexUnsignedType(ty);
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
    CType getFPHalf() {
        return fhalfty;
    }
    CType getFP80() {
        return f80ty;
    }
    CType getLongDobule() {
        return _longdouble;
    }
    CType getFloat128() {
        return f128ty;
    }
    CType getPPCFloat128() {
        return ppc_f128;
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
    type_tag_t getLongDoubleTag() {
        return _longdouble->tags;
    }
    type_tag_t getLongTag() {
        return _long->tags;
    }
    type_tag_t getULongTag() {
        return _ulong->tags;
    }
    type_tag_t getUCharTag() {
        return _uchar->tags;
    }
    type_tag_t getWcharTag() {
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
