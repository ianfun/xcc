// CType - 64 bit unsigned integer
struct OpaqueCType {
    static constexpr type_tag_t important_mask = TYCOMPLEX | TYIMAGINARY | TYVOID | TYNULLPTR;

  private:
    OpaqueCType() : tags{0} { }
    OpaqueCType(type_tag_t tags) : tags{tags} {};
    type_tag_t tags;
    [[maybe_unused]] static void TEST1() {
        OpaqueCType a = {};
        for (unsigned i = 0; i < 64; ++i) {
            if (i < 16)
                a.setKind(i);
            a.setAlignLog2Value(i);
            assert(a.getAlignLog2Value() == i);
            if (i < 16)
                assert(a.getKind() == i);
            a.tags = 0;
        }
        a.setIntegerReprsentation();
        assert(a.isInteger());
        a.setFloatReprsentation();
        assert(a.isFloating());
        a.toogleReprsentation();
        assert(a.isInteger());
        assert(a.isUnsigned());
        a.setSigned();
        assert(a.isSigned());
        a.toogleSign();
        assert(a.isUnsigned());
        puts("TEST 1: OK");
    }
    [[maybe_unused]] static void TEST2() {
        type_tag_t s;
        OpaqueCType a;
        for (unsigned i = 0; i < 8; ++i) {
            a.tags = build_integer(i);
            if (rand() < 0xFFFF)
                a.toogleSign();
            s = a.isSigned();
            a.setAlignLog2Value(i);
            assert(a.isSigned() == s);
            assert(a.isInteger());
            assert((type_tag_t)a.getIntegerKind().asLog2() == i);
            assert(a.getAlignLog2Value() == i);
            a.clearIntegerAllBits();
            assert(a.getAlignLog2Value() == i);
            assert(!a.getIntegerKind());
            assert(a.isSigned() == s);
        }
        puts("TEST 2: OK");
    }
    [[maybe_unused]] static void TEST3() {
        OpaqueCType a;
        for (unsigned i = 0; i < 16; ++i) {
            a.tags = build_float(i);
            assert(a.isFloating());
            assert(static_cast<type_tag_t>(a.getFloatKind()) == i);
            a.setAlignLog2Value(i);
            assert(a.getAlignLog2Value() == i);
            a.clearFloatAllBits();
            assert(!a.getFloatKind());
            assert(a.getAlignLog2Value() == i);
            a.clearAlignAllBits();
            assert(!a.getAlignLog2Value());
        }
        puts("TEST 3: OK");
    }

  public:
    [[maybe_unused]] static void TEST_MAIN() {
        puts("Running tests...\n");
        TEST1();
        TEST2();
        TEST3();
        puts("All tests passed.");
    }
    type_tag_t getTags() const { return tags; }
    type_tag_t getTagsQualifiersOnly() const { return tags & type_qualifiers; }
    type_tag_t getTagsStoragesOnly() const { return tags & storage_class_specifiers; }
    type_tag_t getTagsQualifiersAndStoragesOnly() const { return tags & type_qualifiers_and_storage_class_specifiers; }
    type_tag_t getTagsNoQualifiersAndStorages() const { return tags & ~type_qualifiers_and_storage_class_specifiers; }
    type_tag_t getTagsNoQualifiers() const { return tags & ~type_qualifiers; }
    type_tag_t getTagsNoStorages() const { return tags & ~storage_class_specifiers; }
    void clearQualifiers() { tags &= ~type_qualifiers; }
    void clearStorages() { tags &= ~storage_class_specifiers; }
    void lvalue_cast() { tags = getTagsNoQualifiersAndStorages(); }
    bool isVoid() const { return tags & TYVOID; }
    bool isComplex() const { return tags & TYCOMPLEX; }
    bool isImaginary() const { return tags & TYIMAGINARY; }
    bool isBool() const { return getKind() == TYPRIM && isInteger() && getIntegerKind().isBool(); }
    bool isVLA() const { return getKind() == TYVLA; }
    bool basic_equals(const_CType other) const {
        // two types are equal
        // the align are equal
        // if one is floating, the other must be floating
        // if one if integer, the other must be integer
        // two IntegerKinds or FloatKinds are equal
        assert(getKind() == TYPRIM);
        assert(other->getKind() == TYPRIM);
        if ((getTags() & important_mask) != (other->getTags() & important_mask))
            return false;
        bool A = isInteger();

        if (A != other->isInteger())
            return false;

        if (A) {
            if (getIntegerKind().asLog2() == other->getIntegerKind().asLog2()) {
                return isSigned() == other->isSigned();
            }
            return false;
        }
        return getFloatKind().asEnum() == other->getFloatKind().asEnum();
    }
    bool isGlobalStorage() const { return tags & (TYSTATIC | TYEXTERN); }
    void setTag(type_tag_t new_tag) { tags = new_tag; }
    void setTags(const type_tag_t new_tags) { tags = new_tags; }
    bool hasTag(const type_tag_t theTag) const { return tags & theTag; }
    bool hasTags(const type_tag_t theTag) const { return hasTag(theTag); }
    void addTag(const type_tag_t tag) { tags |= tag; }
    void addTags(const type_tag_t tags) { return addTag(tags); }
    bool isScalar() const {
        auto k = getKind();
        return k == TYPOINTER || (k == TYPRIM && !(tags & TYVOID));
    }
    bool isNullPtr_t() const { return hasTag(TYNULLPTR); }
    enum CTypeKind getKind() const { return static_cast<enum CTypeKind>(tags >> 60); }
    void setKind(enum CTypeKind kind) {
        const type_tag_t k = static_cast<type_tag_t>(kind);
        assert(k <= 15 && "invalid CTypeKind");
        tags |= (k << 60);
    }
    void setKind(type_tag_t kind) { return setKind(static_cast<enum CTypeKind>(kind)); }
    type_tag_t getAlignLog2Value() const { return (tags >> 53) & 63; }
    llvm::MaybeAlign getAlignAsMaybeAlign() const {
        type_tag_t A = getAlignLog2Value();
        if (A)
            return llvm::Align(type_tag_t(1) << A);
        return llvm::MaybeAlign();
    }
    void setAlignLog2Value(type_tag_t Align) {
        assert(Align < 64 && "Alignment too large");
        tags |= (Align << 53);
    }
    void setAlignInBytes(type_tag_t Bytes) {
        assert(llvm::isPowerOf2_64(Bytes) && "Alignment bytes is not a power of 2");
        return setAlignLog2Value(llvm::Log2_64(Bytes));
    }
    void setAlignInBits(type_tag_t Bits) {
        assert(llvm::isPowerOf2_64(Bits) && "Alignment bits is not power of 2");
        return setAlignInBytes(llvm::Log2_64(Bits));
    }
    void clearAlignAllBits() { tags &= ~(63ULL << 53); }
    static constexpr type_tag_t integer_bit = 1ULL << 52;
    void setFloatReprsentation() { tags |= integer_bit; }
    void setIntegerReprsentation() { tags &= ~integer_bit; }
    bool isInteger() const { return !(tags & integer_bit); }
    bool isFloating() const { return (tags & integer_bit); }
    void toogleReprsentation() { tags ^= integer_bit; }
    static constexpr type_tag_t sign_bit = 1ULL << 51;
    void setSigned() {
        assert(isInteger());
        tags |= sign_bit;
    }
    bool isSigned() const {
        assert(isInteger());
        return tags & sign_bit;
    }
    bool isSigned2() const { return tags & sign_bit; }
    void setUnsigned() {
        assert(isInteger());
        tags &= ~sign_bit;
    }
    bool isUnsigned() const {
        assert(isInteger());
        return !(tags & sign_bit);
    }
    void toogleSign() {
        assert(isInteger());
        tags ^= sign_bit;
    }
    type_tag_t getRawData() const { return tags >> 47; }
    IntegerKind getIntegerKind() const {
        assert(isInteger());
        return getRawData() & 0b111;
    }
    FloatKind getFloatKind() const {
        assert(isFloating());
        return getRawData() & 0b1111;
    }
    type_tag_t getBitWidth() const {
        return isInteger() ? getIntegerKind().getBitWidth() : getFloatKind().getBitWidth();
    }
    void clearFloatAllBits() { tags &= ~(0b1111ULL << 47); }
    void clearIntegerAllBits() { tags &= ~(0b111ULL << 47); }
    [[nodiscard]] type_tag_t del(type_tag_t tags_to_delete) const { return tags & ~tags_to_delete; }
    [[maybe_unused]] void dumpBits() {
        bin(tags);
        bin(tags >> 8);
        bin(tags >> 16);
        bin(tags >> 24);
        bin(tags >> 32);
        bin(tags >> 40);
        bin(tags >> 48);
        bin(tags >> 56);
        putchar(10);
    }
    void clearTags(const type_tag_t tags_to_clear) { tags &= ~tags_to_clear; }
    void clearTag(const type_tag_t tag_to_clear) { tags &= ~tag_to_clear; }
    type_tag_t andTags(const type_tag_t tag_to_clear) const { return tags & tag_to_clear; }
    void noralize() {
        switch (getKind()) {
        case TYFUNCTION: {
            type_tag_t h = ret->getTagsStoragesOnly();
            ret->clearTags(storage_class_specifiers);
            addTags(h);
            break;
        }
        case TYARRAY: {
            type_tag_t h = arrtype->getTagsStoragesOnly();
            arrtype->clearTags(storage_class_specifiers);
            addTags(h);
            break;
        }
        case TYPOINTER: {
            type_tag_t h = p->getTagsStoragesOnly();
            p->clearTags(storage_class_specifiers);
            addTags(h);
            break;
        }
        default: break;
        }
    }
    StringRef get_pointer_qual_str() const {
        if (hasTag(TYCONST))
            return "const";
        if (hasTag(TYVOLATILE))
            return "volatile";
        if (hasTag(TYRESTRICT))
            return "restrict";
        return StringRef();
    }
    StringRef get_storage_str() const {
        if (hasTag(TYSTATIC))
            return "static";
        if (hasTag(TYEXTERN))
            return "extern";
        if (hasTag(TYTHREAD_LOCAL))
            return "_Thread_local";
        if (hasTag(TYTYPEDEF))
            return "typedef";
        if (hasTag(TYREGISTER))
            return "register";
        if (hasTag(TYATOMIC))
            return "_Atomic";
        return StringRef();
    }
    // clang::BuiltinType::getName
    raw_ostream &print_basic(raw_ostream &OS) const {
        if (hasTag(TYVOID))
            return OS << "void";
        if (isComplex())
            OS << "_Complex ";
        if (isImaginary())
            OS << "_Imaginary ";
        if (isInteger())
            return OS << getIntegerKind().show(isSigned());
        return OS << getFloatKind().show();
    }
    bool isIncomplete() const {
        const auto kind = getKind();
        if (kind == TYINCOMPLETE)
            return true;
        if (isVoid())
            return true;
        if (kind == TYARRAY && hassize == false)
            return true;
        return false;
    }
