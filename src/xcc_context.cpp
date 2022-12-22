#define TNEW(TY) (CType) new (getAllocator()) TY
#define SNEW(TY) (Stmt) new (getAllocator()) TY
#define ENEW(TY) (Expr) new (getAllocator()) TY

struct xcc_context {
    xcc_context(const xcc_context &) = delete;
    xcc_context() : table{}, prim_ctype_map{} {
        constint = make_signed(getIntLog2(), TYCONST);
        v = make_unsigned(0, TYVOID);
        b = make_unsigned(getBoolLog2());
        i8 = make_signed(3);
        i16 = make_signed(4);
        i32 = make_signed(5);
        i64 = make_signed(6);
        i128 = make_signed(7);
        u8 = make_unsigned(3);
        u16 = make_unsigned(4);
        u32 = make_unsigned(5);
        u64 = make_unsigned(6);
        u128 = make_unsigned(7);

        fhalfty = make_floating(F_Half);
        bfloatty = make_floating(F_BFloat);
        ffloatty = make_floating(F_Float);
        fdoublety = make_floating(F_Double);

        f80ty = make_floating(F_x87_80);
        f128ty = make_floating(F_Quadruple);
        ppc_f128 = make_floating(F_PPC128);
        fdecimal32 = make_floating(F_Decimal32);
        fdecimal64 = make_floating(F_Decimal64);
        fdecimal128 = make_floating(F_Decimal128);

        _wchar = isWChar_tSigned() ? (getWCharLog2() == 5 ? i32 : i16) : (getWCharLog2() == 5 ? u32 : u16);
        _long = getLongLog2() == 5 ? i32 : i64;
        _ulong = getLongLog2() == 5 ? u32 : u64;
        assert(getUCharLog2() == 5);
        _uchar = isUChar_tSigned() ? i32 : u32;
        _nullptr_t = getPointerType(v, TYNULLPTR);
        void_ptr_ty = getPointerType(v);

        // For wide string literals prefixed by the letter u or U, the array elements have type char16_t or char32_t
        str32ty = getPointerType(make(getChar()->getTags() | TYCONST));
        str16ty = getPointerType(make(getChar8_t()->getTags() | TYCONST));
        // For wide string literals prefixed by the letter L, the array elements have type wchar_t
        wstringty = getPointerType(make(getWChar()->getTags() | TYCONST));
        // For character string literals, the array elements have type char,
        stringty = getPointerType(make(getChar()->getTags() | TYCONST));
        // For UTF-8 string literals, the array elements have type char8_t, and are initialized with the characters of
        // the multibyte character sequence
        str8ty = getPointerType(make(getChar8_t()->getTags() | TYCONST));

        _longdouble = make_floating(getLongDoubleFloatKind());

        // There are three complex types, designated as float _Complex, double _Complex, and long double _Complex.

        _complex_double = make(fdoublety->getTags() | TYCOMPLEX);
        _complex_float = make(ffloatty->getTags() | TYCOMPLEX);
        _complex_longdouble = make(_longdouble->getTags() | TYCOMPLEX);

        _imaginary_double = make(fdoublety->getTags() | TYIMAGINARY);
        _imaginary_float = make(ffloatty->getTags() | TYIMAGINARY);

    }
    IdentifierTable table; // contains allocator!
    llvm::DenseMap<type_tag_t, CType> prim_ctype_map;
    // C23: _Bitint
    llvm::DenseMap<std::pair<unsigned, uint8_t>, CType> bitIntMap;
    CType constint, b, v, i8, u8, i16, u16, i32, u32, i64, u64, i128, u128, ffloatty, fdoublety, bfloatty, fhalfty,
        f128ty, ppc_f128, f80ty, str8ty, str16ty, str32ty, stringty, wstringty, _complex_float, _complex_double,
        _complex_longdouble, _short, _ushort, _wchar, _long, _ulong, _longlong, _ulonglong, _longdouble, _uchar,
        fdecimal32, fdecimal64, fdecimal128, _imaginary_double, _imaginary_float,
        _nullptr_t, void_ptr_ty;
    [[nodiscard]] CType make(type_tag_t tags) { return TNEW(PrimType){.tags = tags}; }
    [[nodiscard]] CType make_cached(type_tag_t tags) {
        auto it = prim_ctype_map.insert({tags, nullptr});
        if (it.second) { // not found
            it.first->second = make(tags);
        }
        return it.first->second;
    }
    [[nodiscard]] CType make_unsigned(uint8_t shift, type_tag_t tags = 0) {
        return make(build_integer(IntegerKind::fromLog2(shift), false) | tags);
    }
    [[nodiscard]] CType make_signed(uint8_t shift, type_tag_t tags = 0) {
        return make(build_integer(IntegerKind::fromLog2(shift), true) | tags);
    }
    [[nodiscard]] CType make_floating(FloatKind kind, type_tag_t tags = 0) { return make(build_float(kind) | tags); }
    [[nodiscard]] CType make_complex_float(FloatKind kind, type_tag_t tags = 0) {
        return make(build_float(kind) | TYCOMPLEX | tags);
    }
    [[nodiscard]] CType make_imaginary_float(FloatKind kind, type_tag_t tags = 0) {
        return make(build_float(kind) | TYIMAGINARY | tags);
    }
    [[nodiscard]] CType getVoidPtrType() const {
        return void_ptr_ty;
    }
    [[nodiscard]] CType getBlockAddressType() const {
        return void_ptr_ty;
    }
    [[nodiscard]] CType getBitIntType(unsigned width, CType base) {
        auto it = bitIntMap.insert({std::make_pair(width, base->getIntegerKind().asLog2()), nullptr});
        if (it.second) {
            CType ty = TNEW(BitintType) {.bitint_base = base, .bits = width};
            ty->setKind(TYBITINT);
            it.first->second = ty;
        }
        return it.first->second;
    } 
    [[nodiscard]] CType lookupType(const_CType ty, type_tag_t tags) {
        if (ty->isFloating()) {
            switch (ty->getFloatKind().asEnum()) {
            case F_Float: return getFloat();
            case F_Double: return getDobule();
            case F_Half: return getFPHalf();
            case F_BFloat: return getBFloat();
            case F_Quadruple: return getFloat128();
            case F_PPC128: return getPPCFloat128();
            case F_x87_80: return getFP80();
            default: break;
            }
        } else {
            bool isSigned = ty->isSigned();
            switch (ty->getIntegerKind().asLog2()) {
            case 0:
            case 1: return b;
            case 3: return isSigned ? i8 : u8;
            case 4: return isSigned ? i16 : u16;
            case 5: return isSigned ? i32 : u32;
            case 6: return isSigned ? i64 : u64;
            case 7: return isSigned ? i128 : u128;
            default: break;
            }
        }
        return make_cached(ty->del(tags));
    }
    [[nodiscard]] CType getComplexElementType(const_CType ty) {
        // For each floating type there is a corresponding real type, which is always a real floating type. For real
        // floating types, it is the same type. For complex types, it is the type given by deleting the keyword _Complex
        // from the type name.
        return lookupType(ty, TYCOMPLEX);
    }
    [[nodiscard]] CType getImaginaryElementType(const_CType ty) { return lookupType(ty, TYIMAGINARY); }
    [[nodiscard]] CType tryGetComplexTypeFromNonComplex(const_CType ty) {
        if (ty->isFloating()) {
            switch (ty->getFloatKind().asEnum()) {
            case F_Float: return getComplexFloat();
            case F_Double: return getComplexDouble();
            default: break;
            }
        }
        return make_cached(ty->getTags() | TYCOMPLEX);
    }
    [[nodiscard]] CType tryGetImaginaryTypeFromNonImaginary(const_CType ty) {
        if (ty->isFloating()) {
            switch (ty->getFloatKind().asEnum()) {
            case F_Float: return getImaginaryFloat();
            case F_Double: return getImaginaryDouble();
            default: break;
            }
        }
        return make_cached(ty->getTags() | TYIMAGINARY);
    }
    [[nodiscard]] CType getImaginaryDouble() { return _imaginary_double; }
    [[nodiscard]] CType getImaginaryFloat() { return _imaginary_float; }
    [[nodiscard]] CType getPointerType(CType base, type_tag_t tags = 0) {
        CType result = TNEW(PointerType){.tags = tags, .p = base};
        result->setKind(TYPOINTER);
        return result;
    }
    [[nodiscard]] CType getFixArrayType(CType elementType, unsigned size) {
        CType result = TNEW(ArrayType){.tags = 0, .arrtype = elementType, .hassize = true, .arrsize = size};
        result->setKind(TYARRAY);
        return result;
    }
    [[nodiscard]] CType getNullPtr_t() const { return _nullptr_t; }
    [[nodiscard]] CType createDummyType() {
        CType res = reinterpret_cast<CType>(getAllocator().Allocate(ctype_max_size, 1));
        res->setTags(0);
        res->setKind(TYPRIM);
        return res;
    }
    [[nodiscard]] CType getComplexFloat() const { return _complex_float; }
    [[nodiscard]] CType getComplexDouble() const { return _complex_double; }
    [[nodiscard]] CType getComplexLongDouble() const { return _complex_longdouble; }
    [[nodiscard]] CType getFloat() const { return ffloatty; }
    [[nodiscard]] CType getBFloat() const { return bfloatty; }
    [[nodiscard]] CType getDobule() const { return fdoublety; }
    [[nodiscard]] CType getFPHalf() const { return fhalfty; }
    [[nodiscard]] CType getFP80() const { return f80ty; }
    [[nodiscard]] CType getLongDobule() const { return _longdouble; }
    [[nodiscard]] CType getFloat128() const { return f128ty; }
    [[nodiscard]] CType getPPCFloat128() const { return ppc_f128; }
    [[nodiscard]] CType getDecimal32() const { return fdecimal32; }
    [[nodiscard]] CType getDecimal64() const { return fdecimal64; }
    [[nodiscard]] CType getDecimal128() const { return fdecimal128; }
    [[nodiscard]] CType getBool() const { return b; }
    [[nodiscard]] CType getConstInt() const { return constint; }
    [[nodiscard]] CType getInt() const { return i32; }
    [[nodiscard]] CType getUInt() const { return u32; }
    [[nodiscard]] CType getVoid() const { return v; }
    // https://stackoverflow.com/questions/15533115/why-dont-the-c-or-c-standards-explicitly-define-char-as-signed-or-unsigned
    [[nodiscard]] CType getChar() const { return i8; }
    // C23
    // The types declared are mbstate_t (described in 7.31.1) and size_t (described in 7.21);
    //     char8_t
    // which is an unsigned integer type used for 8-bit characters and is the same type as unsigned char;
    //     char16_t
    // which is an unsigned integer type used for 16-bit characters and is the same type as uint_least16_t
    // (described in 7.22.1.2); and
    //     char32_t
    // which is an unsigned integer type used for 32-bit characters and is the same type as uint_least32_t
    // (also described in 7.22.1.2).
    [[nodiscard]] CType getChar8_t() const { return u8; }
    [[nodiscard]] CType getChar16_t() const { return u16; }
    [[nodiscard]] CType getChar32_t() const { return u32; }
    [[nodiscard]] uint8_t getChar8_tLog2() const { return 3; }
    [[nodiscard]] uint8_t getChar16_tLog2() const { return 4; }
    [[nodiscard]] uint8_t getChar32_tLog2() const { return 5; }
    [[nodiscard]] CType getLong() const { return _long; }
    [[nodiscard]] CType getULong() const { return _ulong; }
    [[nodiscard]] CType getWChar() const { return _wchar; }
    [[nodiscard]] CType getUChar() const { return _uchar; }
    [[nodiscard]] CType getShort() const { return _short; }
    [[nodiscard]] CType getUShort() const { return _ushort; }
    [[nodiscard]] CType getInt128() const { return i128; }
    [[nodiscard]] CType getUInt128() const { return u128; }
    [[nodiscard]] CType getLongLong() const { return u64; }
    [[nodiscard]] CType getULongLong() const { return i64; }
    [[nodiscard]] ArenaAllocator &getAllocator() { return table.getAllocator(); }
    [[nodiscard]] void *new_memcpy(size_t size, const void *src) {
        void *mem = getAllocator().Allocate(size, 1);
        (void)memcpy(mem, src, size);
        return mem;
    }
    [[nodiscard]] CType fromLLVMType(const llvm::Type *Ty) {
        switch (Ty->getTypeID()) {
            case llvm::Type::HalfTyID:
                return getFPHalf();
            case llvm::Type::BFloatTyID:
                return getBFloat();
            case llvm::Type::FloatTyID:
                return getFloat();
            case llvm::Type::DoubleTyID:
                return getDobule();
            case llvm::Type::X86_FP80TyID:
                return getFP80();
            case llvm::Type::FP128TyID:
                return getFloat128();
            case llvm::Type::PPC_FP128TyID:
                return getPPCFloat128();
            case llvm::Type::VoidTyID:
                return getVoid();
             case llvm::Type::ArrayTyID:
            {
                const llvm::ArrayType *ty = cast<llvm::ArrayType>(Ty);
                return getFixArrayType(fromLLVMType(ty->getElementType()), ty->getNumElements());
            }
            case llvm::Type::LabelTyID:
            case llvm::Type::MetadataTyID:
            case llvm::Type::X86_MMXTyID:
            case llvm::Type::X86_AMXTyID:
            case llvm::Type::TokenTyID:
            case llvm::Type::StructTyID:
            case llvm::Type::FixedVectorTyID: // TODO: ...
            case llvm::Type::ScalableVectorTyID:
            default:
                break;
            case llvm::Type::IntegerTyID:
                return getIntTypeFromBitSize(cast<llvm::IntegerType>(Ty)->getBitWidth());
            case llvm::Type::FunctionTyID:
            {
                const llvm::FunctionType *FT = cast<llvm::FunctionType>(Ty);
                CType ty = TNEW(FunctionType){.ret = fromLLVMType(FT->getReturnType()), .params = xvector<Param>::get_with_length(FT->getNumParams()), .isVarArg = FT->isVarArg()};
                ty->setKind(TYFUNCTION);
                size_t i = 0;
                for (const llvm::Type *it: FT->params()) 
                    ty->params[i] = Param(nullptr, fromLLVMType(it));
                return ty;
            }
            case llvm::Type::PointerTyID:
                return getVoidPtrType();
        }
        llvm_unreachable("unsupported type");
    }
    [[nodiscard]] CType getIntTypeFromBitSize(unsigned bitSize) {
        switch (bitSize) {
        case 128: return u128;
        case 64: return u64;
        case 32: return u32;
        case 16: return u16;
        case 8: return u8;
        case 1: return b;
        }
        return getBitIntType(bitSize, getUInt());
    }
    [[nodiscard]] Expr createParenExpr(Expr e, location_t L, location_t R) {
        const size_t Size = expr_size_map[e->k];
        void *const mem = getAllocator().Allocate(Size + sizeof(location_t) * 2, 1);
        (void)memcpy(mem, e, Size);
        location_t *const Ptr = reinterpret_cast<location_t *>(reinterpret_cast<uintptr_t>(mem) + Size);
        Ptr[0] = L;
        Ptr[1] = R;
        Expr res = reinterpret_cast<Expr>(mem);
        res->ty = clone(e->ty);
        res->ty->addTag(TYPAREN);
        return res;
    }
    [[nodiscard]] CType clone(CType ty) {
        return reinterpret_cast<CType>(new_memcpy(ctype_size_map[ty->getKind()], ty));
    }
    [[nodiscard]] Stmt clone(Stmt s) { return reinterpret_cast<Stmt>(new_memcpy(stmt_size_map[s->k], s)); }
    [[nodiscard]] Expr clone(Expr e) { return reinterpret_cast<Expr>(new_memcpy(expr_size_map[e->k], e)); }
    [[nodiscard]] uint8_t getBoolLog2() const { return 0; }
    [[nodiscard]] uint8_t getCharLog2() const { return 3; }
    [[nodiscard]] uint8_t getShortLog2() const { return 4; }
    [[nodiscard]] uint8_t getIntLog2() const { return 5; }
    [[nodiscard]] uint8_t getLongLog2() const { return 6; }
    [[nodiscard]] uint8_t getLongLongLog2() const { return 6; }
    [[nodiscard]] uint8_t getInt128Log2() const { return 7; }
    // https://learn.microsoft.com/en-us/cpp/cpp/string-and-character-literals-cpp?view=msvc-170
    // Linux's wchar(L prefix) is int, Windows is unsigned short
    // they are system APIs, so we must follow.
    /*
    C23
    Prefix | Corresponding Type
    none | unsigned char
    u8 | char8_t
    L | the unsigned type corresponding to wchar_t
    u | char16_t
    U | char32_t
    */
    [[nodiscard]] uint8_t getWCharLog2() const { return 5; }
    [[nodiscard]] bool isWChar_tSigned() const {
        // unsigned in Win32(unsigned short)
        // signed int Linux(int)
        return true;
    }
    [[nodiscard]] uint8_t getUCharLog2() const { return 5; }
    [[nodiscard]] bool isUChar_tSigned() const {
        // always unsigned since char32_t is unsigned
        return false;
    }
    [[nodiscard]] FloatKind getLongDoubleFloatKind() const { return F_Quadruple; }
    [[nodiscard]] unsigned getEnumSizeInBits() const {
        return 32;
    }
    [[nodiscard]] unsigned getEnumLog2() const {
        return 5;
    }
    [[nodiscard]] CType getEnumRealType() const {
        return getInt();
    }
    [[nodiscard]] CType getStringType(enum StringPrefix enc) const {
        switch (enc) {
            case Prefix_none:
                return stringty;
            case Prefix_u8:
                return str8ty;
            case Prefix_L:
                return wstringty;
            case Prefix_u:
                return str16ty;
            case Prefix_U:
                return str32ty;
        }
        llvm_unreachable("invalid enum StringPrefix");
    }
    [[nodiscard]] unsigned getStringCharSizeInBits(enum StringPrefix enc) const {
        switch (enc) {
            case Prefix_none:
                return 8;
            case Prefix_u8:
                return 8;
            case Prefix_L:
                return getWCharLog2() == 5 ? 32 : 16;
            case Prefix_u:
                return 16;
            case Prefix_U:
                return 32;
        }
        llvm_unreachable("invalid enum StringPrefix");
    }
};
#undef TNEW
#undef SNEW
#undef ENEW
#define TNEW(TY) (CType) new (context.getAllocator()) TY
#define SNEW(TY) (Stmt) new (context.getAllocator()) TY
#define ENEW(TY) (Expr) new (context.getAllocator()) TY
