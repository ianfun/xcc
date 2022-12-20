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
    type_tag_t getTagsStoragesAndFunctionsOnly() const {
        return tags & (storage_class_specifiers | TYINLINE | TYNORETURN);
    }
    type_tag_t getTagsQualifiersAndStoragesOnly() const { return tags & type_qualifiers_and_storage_class_specifiers; }
    type_tag_t getTagsNoQualifiersAndStorages() const { return tags & ~type_qualifiers_and_storage_class_specifiers; }
    type_tag_t getTagsNoQualifiersAndStoragesAndFunctions() const {
        return tags & (~type_qualifiers_and_storage_class_specifiers | TYINLINE | TYNORETURN);
    }
    type_tag_t getTagsNoQualifiers() const { return tags & ~type_qualifiers; }
    type_tag_t getTagsNoStorages() const { return tags & ~storage_class_specifiers; }
    void clearQualifiers() { tags &= ~type_qualifiers; }
    void clearStorages() { tags &= ~storage_class_specifiers; }
    void lvalue_cast() { tags = getTagsNoQualifiersAndStoragesAndFunctions(); }
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
        return k == TYPOINTER || (k == TYPRIM && !(tags & TYVOID)) || (k == TYTAG && isEnum());
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
    llvm::Align getAlign() const {
        return llvm::Align(type_tag_t(1) << getAlignLog2Value());
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
    const_CType getFunctionAttrTy() const { return this; }
    CType getFunctionAttrTy() { return this; }
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
        switch (getKind()) {
        case TYPRIM: return isVoid();
        case TYTAG: return !hasDefinition();
        case TYARRAY: return !hassize;
        default: return false;
        }
    }
    bool isBitField() const {
        return getKind() == TYBITFIELD;
    }
    unsigned getBitFieldSize() const {
        assert(isBitField());
        return bitsize;
    }
    CType getBitFieldRealType() const {
        assert(isBitField());
        return bittype;
    }
    bool isBitInt() const { return getKind() == TYBITINT; }
    unsigned getBitIntBits() const {
        assert(isBitInt());
        return bits;
    }
    void setBitIntBits(unsigned bits) {
        assert(isBitInt());
        this->bits = bits;
    }
    CType getBitIntBaseType() const {
        assert(isBitInt());
        return bitint_base;
    }
    bool isBitIntSigned() const {
        assert(isBitInt());
        return bitint_base->isSigned();
    }
    void setDefinition(struct RecordDecl *def) {
        assert(isAgg());
        tag_decl = def;
    }
    bool hasDefinition() const {
        assert(isAgg());
        return static_cast<bool>(tag_decl);
    }
    void completeDefinition(union TagDecl tag_decl) {
        assert(isAgg());
        this->tag_decl = tag_decl;
    }
    enum TagKind getTagDeclTagType() const { return tag; }
    bool isTag(enum TagKind tag) const { return tag == this->tag; }
    bool isAgg() const { return getKind() == TYTAG; }
    bool isEnum() const {
        assert(isAgg());
        return isTag(TagType_Enum);
    }
    bool isUnion() const {
        assert(isAgg());
        return isTag(TagType_Union);
    }
    bool isStruct() const {
        assert(isAgg());
        return isTag(TagType_Struct);
    }
    bool isUnionOrStruct() const {
        assert(isAgg());
        return !isTag(TagType_Enum);
    }
    const TagDecl getTagDecl() const {
        assert(isAgg());
        return tag_decl;
    }
    const RecordDecl *getRecord() const {
        assert(isUnionOrStruct());
        return tag_decl.getRecord();
    }
    RecordDecl *getRecord() {
        assert(isUnionOrStruct());
        return tag_decl.getRecord();
    }
    const EnumDecl *getEnum() const {
        assert(isEnum());
        return tag_decl.getEnum();
    }
    EnumDecl *getEnum() {
        assert(isEnum());
        return tag_decl.getEnum();
    }
    bool hasTagName() const {
        assert(isAgg());
        return tag_name != nullptr;
    }
    IdentRef getTagName() const {
        assert(isAgg());
        return tag_name;
    }
    void setTagName(IdentRef tag_name) {
        assert(isAgg());
        this->tag_name = tag_name;
    }
    CType getTypeForIndex(unsigned i) {
        if (getKind() == TYARRAY)
            return arrtype;
        return getRecord()->fields[i].ty;
    }
    unsigned getNumElements() const {
        if (getKind() == TYARRAY)
            return hassize ? arrsize : unsigned(-1);
        return getRecord()->fields.size();
    }
    CType getFieldIndex(IdentRef Name, xvector<unsigned> &idxs) const {
        const SmallVectorImpl<FieldDecl> &fields = getRecord()->fields;
        for (unsigned i = 0;i < fields.size();++i) {
            if (!fields[i].name) {
                assert(fields[i].ty->isAgg());
                if (!fields[i].ty->isEnum()) {
                    idxs.push_back(i);
                    size_t oldSize = idxs.size();
                    CType ty = fields[i].ty->getFieldIndex(Name, idxs);
                    if (idxs.size() != oldSize) {
                        return ty;
                    }
                    idxs.pop_back();
                }
            }
            else if (fields[i].name == Name) {
                idxs.push_back(i);
                return fields[i].ty;
            }
        }
        return nullptr;
    }
    CType getFieldIndex(IdentRef Name, SmallVectorImpl<Designator> &idxs) const {
        const SmallVectorImpl<FieldDecl> &fields = getRecord()->fields;
        for (unsigned i = 0;i < fields.size();++i) {
            if (!fields[i].name) {
                assert(fields[i].ty->isAgg());
                if (!fields[i].ty->isEnum()) {
                    idxs.push_back(Designator(i));
                    size_t oldSize = idxs.size();
                    CType ty = fields[i].ty->getFieldIndex(Name, idxs);
                    if (idxs.size() != oldSize) {
                        return ty;
                    }
                    idxs.pop_back();
                }
            }
            else if (fields[i].name == Name) {
                idxs.push_back(Designator(i));
                return fields[i].ty;
            }
        }
        return nullptr;
    }
    CType getFieldIndexType(ArrayRef<unsigned> idxs) {
        if (idxs.empty()) return this;
        if (getKind() == TYARRAY) {
            return arrtype->getFieldIndexType(idxs.drop_front(1));
        }
        assert(isUnionOrStruct());
        return getRecord()->fields[idxs.front()].ty->getFieldIndexType(idxs.drop_front(1));
    }

