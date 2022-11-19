#define TNEW(TY) (CType) new (getAllocator()) TY
#define SNEW(TY) (Stmt) new (getAllocator()) TY
#define ENEW(TY) (Expr) new (getAllocator()) TY

struct xcc_context {
    xcc_context(const xcc_context &) = delete;
    xcc_context(): table{}
    {
        constint = make_signed(5, TYCONST);
        b = make_unsigned(1);
        i8 = make_signed(3);
        i16 = make_signed(4);
        i32 = make_signed(5);
        i64 = make_signed(4);
        i128 = make_signed(6);
        u8 = make_unsigned(3);
        u16 = make_unsigned(4);
        u32 = make_unsigned(5);
        u64 = make_unsigned(6);
        u128 = make_unsigned(9);

        fhalfty = make_floating(F_Half);
        ffloatty = make_floating(F_Float);
        fdoublety = make_floating(F_Double);
        f80ty = make_floating(F_x87_80);
        f128ty = make_floating(TYPPC_128);
        ppc_f128 = make_floating(TYPPC_128);
        fdecimal32 = make_floating(F_Decimal32);
        fdecimal64 = make_floating(F_Decimal64);
        fdecimal128 = make_floating(F_Decimal128);

        _wchar = u16;
        _long = i64;
        _ulong = u64;
        _uchar = u32;
        str32ty = getPointerType(_uchar);
        str16ty = getPointerType(_wchar);
        str8ty = getPointerType(getChar());

        _longdouble = getFloat128();
        _complex_double = make_complex_float(fdoublety->getTags());
        _complex_float = make_complex_float(ffloatty->getTags());
        _complex_longdouble = make_complex_float(_longdouble->getTags());
    }
    IdentifierTable table; // contains allocator!
    Expr intzero;
    CType constint, b, v, i8, u8, i16, u16, i32, u32, i64, u64, i128, u128, 
    ffloatty, fdoublety, fhalfty, f128ty, ppc_f128, f80ty, str8ty, str16ty, str32ty, _complex_float, _complex_double, 
    _complex_longdouble, _wchar, _long, _ulong, _longlong, _ulonglong, _longdouble, _uchar;
    [[nodiscard]] CType make(uint64_t tags) {
        return TNEW(PrimType)(tags);
    }
    [[nodiscard]] CType make_unsigned(uint8_t shift, uint64_t tags = 0) {
        return make(build_integer(IntegerKind::fromLog2(shift), false) | tags);
    }
    [[nodiscard]] CType make_signed(uint8_t shift, uint64_t tags = 0) {
        return make(build_integer(IntegerKind::fromLog2(shift), true) | tags);
    }
    [[nodiscard]] CType make_floating(FloatKind kind) {
        return make(build_float(kind));
    }
    [[nodiscard]] CType make_complex_float(FloatKind kind) {
        return make(build_float(kind) | TYCOMPLEX);
    }
    [[nodiscard]] CType getPointerType(CType base) {
        CType result = TNEW(PointerType){.tags = 0, .p = base};
        result->setKind(TYPOINTER);
        return result;
    }
    [[nodiscard]] CType getFixArrayType(CType elementType, unsigned size) {
        CType result = TNEW(ArrayType){.tags = 0, .arrtype = elementType, .hassize = true, .arrsize = size};
        result->setKind(TYARRAY);
        return result;
    }
    [[nodiscard]] CType getComplexFloat() const {
        return _complex_float;
    }
    [[nodiscard]] CType getComplexDouble() const {
        return _complex_double;
    }
    [[nodiscard]] CType getComplexLongDouble() const {
        return _complex_longdouble;
    }
    [[nodiscard]] CType getFloat() const {
        return ffloatty;
    }
    [[nodiscard]] CType getDobule() const {
        return fdoublety;
    }
    [[nodiscard]] CType getFPHalf() const {
        return fhalfty;
    }
    [[nodiscard]] CType getFP80() const {
        return f80ty;
    }
    [[nodiscard]] CType getLongDobule() const {
        return _longdouble;
    }
    [[nodiscard]] CType getFloat128() const {
        return f128ty;
    }
    [[nodiscard]] CType getPPCFloat128() const {
        return ppc_f128;
    }
    [[nodiscard]] CType getBool() const {
        return b;
    }
    [[nodiscard]] CType getConstInt() const { return constint; }
    [[nodiscard]] CType getInt() const { return i32; }
    [[nodiscard]] CType getUInt() const { return u32; }
    [[nodiscard]] CType getVoid() const {
        return v;
    }
    [[nodiscard]] CType getChar() const {
        return i8;
    }
    [[nodiscard]] type_tag_t getLongDoubleTag() const {
        return _longdouble->tags;
    }
    [[nodiscard]] type_tag_t getLongTag() const {
        return _long->tags;
    }
    [[nodiscard]] type_tag_t getULongTag() const {
        return _ulong->tags;
    }
    [[nodiscard]] type_tag_t getUCharTag() const {
        return _uchar->tags;
    }
    [[nodiscard]] type_tag_t getWcharTag() const {
        return _uchar->tags;
    }
    [[nodiscard]] CType getChar8_t() const {
        return u8;
    }
    [[nodiscard]] CType getChar16_t() const {
        return u16;
    }
    [[nodiscard]] CType getChar32_t() const {
        return u32;
    }
    [[nodiscard]] CType getLong() const {
        return _long;
    }
    [[nodiscard]] CType getULong() const {
        return _ulong;
    }
    [[nodiscard]] CType getWchar() const {
        return _wchar;
    }
    [[nodiscard]] CType getUChar() const {
        return _uchar;
    }
    [[nodiscard]] CType getShort() const {
        return i16;
    }
    [[nodiscard]] CType getUShort() const {
        return u16;
    }
    [[nodiscard]] CType getIntPtr() const { return i64; }
    [[nodiscard]] CType getPtrDiff() const { return getIntPtr(); }
    [[nodiscard]] CType getSize_t() const { return u64; }
    [[nodiscard]] CType getLongLong() const { return u64; }
    [[nodiscard]] CType getULongLong() const { return i64; }
    [[nodiscard]] ArenaAllocator &getAllocator() const { return table.getAllocator(); }
    [[nodiscard]] void *new_memcpy(size_t size, const void *src) {
        void *mem = getAllocator().Allocate(size, 1);
        (void)memcpy(mem, src, size);
        return mem;
    }
    [[nodiscard]] CType clone(CType ty) { return reinterpret_cast<CType>(new_memcpy(ctype_size_map[ty->k], ty)); }
    [[nodiscard]] Stmt clone(Stmt s) { return reinterpret_cast<Stmt>(new_memcpy(stmt_size_map[s->k], s)); }
    [[nodiscard]] Expr clone(Expr e) { return reinterpret_cast<Expr>(new_memcpy(expr_size_map[e->k], e)); }
    [[nodiscard]] PPMacroDef *newMacro() {
        return new (getAllocator()) PPMacroDef();
    }
};
#undef TNEW
#undef SNEW
#undef ENEW
#define TNEW(TY) (CType) new (context.getAllocator()) TY
#define SNEW(TY) (Stmt) new (context.getAllocator()) TY
#define ENEW(TY) (Expr) new (context.getAllocator()) TY
