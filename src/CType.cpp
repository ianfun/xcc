// CType - 64 bit unsigned integer
struct OpacheCType
{
    uint64_t tags;
    static void bin(uint64_t a) {
        putchar(a & 1 ? '1' : '0');
        putchar(a & 2 ? '1' : '0');
        putchar(a & 4 ? '1' : '0');
        putchar(a & 8 ? '1' : '0');
        putchar(a & 16 ? '1' : '0');
        putchar(a & 32 ? '1' : '0');
        putchar(a & 64 ? '1' : '0');
        putchar(a & 128 ? '1' : '0');
        putchar(' ');
    }
    uint64_t getTags() const { return tags; }
    void setTags(uint64_t new_tag) { tags = new_tag; }
    void setTags(const uint64_t new_tags) { tags = new_tags; }
    bool hasTag(const uint64_t theTag) const {
        return tags & theTag;
    }
    bool hasTags(const uint64_t theTag) const {
        return hasTag(theTag);
    }
    void addTag(const uint64_t tag) {
        tags |= tag;
    }
    void addTags(const uint64_t tag) {
        return addTags(tag);
    }
    enum CTypeKind getKind() const {
        return static_cast<enum CTypeKind>(tags >> 60);
    }
    void setKind(enum CTypeKind kind) {
        const uint64_t k = static_cast<uint64_t>(kind);
        assert(k <= 15 && "invalid CTypeKind");
        tags |= (k << 60);
    }
    uint64_t getAlignLog2Value() const {
        return (tags >> 53) & 63;
    }
    llvm::MaybeAlign getAlignAsMaybeAlign() const {
        uint64_t A = getAlignLog2Value();
        if (A) return llvm::Align(llvm::Align::LogValue(A));
        return llvm::MaybeAlign();
    }
    void setAlignLog2Value(uint64_t Align) {
        assert(Align < 64 && "Alignment too large");
        tags |= (Align << 53);
    }
    void setAlignInBytes(uint64_t Bytes) {
        assert(llvm::isPowerOf2_64(Bytes) && "Alignment bytes is not a power of 2");
        return setAlignLog2Value(llvm::Log2_64(Bytes));
    }
    void setAlignInBits(uint64_t Bits) {
        assert(llvm::isPowerOf2_64(Bits) && "Alignment bits is not power of 2");
        return setAlignInBytes(llvm::Log2_64(Bits));
    }
    void clearAlignAllBits() {
        tags &= ~(63ULL << 53);
    }
    static constexpr uint64_t integer_bit = 1ULL << 52;
    void setFloatReprsentation() {
        tags |= integer_bit;
    }
    void setIntegerReprsentation() {
        tags &= ~integer_bit;
    }
    bool isInteger() const {
        return !(tags & integer_bit);
    }
    bool isFloating() const {
        return (tags & integer_bit);
    }
    void toogleReprsentation() {
        tags ^= integer_bit;
    }
    static constexpr uint64_t sign_bit = 1ULL << 51;
    void setSigned() {
        assert(isInteger());
        tags |= sign_bit;
    }
    bool isSigned() const {
        assert(isInteger());
        return tags & sign_bit;
    }
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
    IntegerKind getInteger() const {
        assert(isInteger());
        return IntegerKind::fromLog2((tags >> 47) & 0b111);
    }
    FloatKind getFloatKind() const {
        assert(isFloating());
        return (tags >> 47) & 0b1111;
    }
    static uint64_t make_integer(IntegerKind kind, bool Signed = false) {
        const uint64_t log2size = kind.asLog2();
        assert(log2size <= 7 && "integer suze too large, max is log2(128)=7!");
        return (log2size << 47) | (Signed ? sign_bit : 0ULL);
    }
    static uint64_t make_float(FloatKind kind) {
        const uint64_t k = kind;
        assert(k < 16 && "invalid kind(max is 15)");
        return (k << 47) | integer_bit;
    }
    void clearFloatAllBits() {
        tags &= ~(0b1111ULL << 47);
    }
    void clearIntegerAllBits() {
        tags &= ~(0b111ULL << 47);
    }
    [[nodiscard]] void del(uint64_t tags_to_delete) const {
        return tags & tags_to_delete;
    }
    void dumpBits(){
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
    void clearTags(const uint64_t tags_to_clear) {
        tags &= ~tags_to_clear;
    }
    uint64_t andTags(const uint64_t tag_to_clear) const {
        return tags & tag_to_clear;
    }
    enum CTypeKind getKind() const {
        return static_cast<enum CTypeKind>(this->tags >> 52);
    }
    void noralize() {
        const type_tag_t mask = ty_storages;
        switch (k) {
        case TYFUNCTION:
        {
            type_tag_t h = ret->del(mask);
            ret->clearTags(mask);
            addTags(h);
            break;
        }
        case TYARRAY:
        {
            type_tag_t h = arrtype->del(mask);
            arrtype->clearTags(mask);
            addTags(h);
            break;
        }
        case TYPOINTER:
        {
            type_tag_t h = p->del(mask);
            p->clearTags(mask);
            addTags(h);
            break;
        }
        default: break;
        }
    }
    [[maybe_unused]] static void TEST1() {
        CType a = {};
        for (unsigned i = 0;i < 64;++i) {
            if (i < 16) a.setKind(i);
            a.setAlign(i);
            assert(a.getAlign()==i);
            if (i < 16) assert(a.getKind()==i);
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
        uint64_t s;
        CType a;
        for (unsigned i = 0;i < 8;++i) {
            a.setAlign(i);
            a.tags = CType::make_integer(i);
            if (rand() < 0xFFFF)
                a.toogleSign();
            s = a.isSigned();
            a.setAlign(i);
            assert(a.isSigned()==s);
            assert(a.isInteger());
            assert(a.getInteger()==i);
            assert(a.getAlign()==i);
            a.clearIntegerAllBits();
            assert(a.getAlign()==i);
            assert(a.getInteger()==0);
            assert(a.isSigned()==s);
        }
        puts("TEST 2: OK");
    }
    [[maybe_unused]] static void TEST3() {
        CType a;
        for(unsigned i = 0;i < 16;++i)
        {
            a.tags = CType::make_float(i);
            assert(a.isFloating());
            assert(a.getFloatKind() == i);
            a.setAlign(i);
            assert(a.getAlign()==i);
            a.clearFloatAllBits();
            assert(a.getFloatKind()==0);
            assert(a.getAlign()==i);
            a.clearAlignAllBits();
            assert(a.getAlign()==0);
        }
        puts("TEST 3: OK");
    }
    [[maybe_unused]] static void TEST_MAIN() {
        puts("Running tests...\n");
        TEST1();
        TEST2();
        TEST3();
        puts("All tests passed.");
    }
