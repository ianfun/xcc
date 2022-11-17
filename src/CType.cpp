// CType - 64 bit unsigned integer
struct OpacheCType
{
private:
    uint64_t tags;
public:
    uint64_t getTags() const { return this->tags; }
    void setTags(uint64_t new_tag) { this->tags = new_tag; }
    void setTags(const uint64_t new_tags) { this->tags = new_tags; }
    bool hasTag(const uint64_t tag) const {
        return this->tags & tag;
    }
    bool hasTags(const uint64_t tags) const {
        return this->tags & tags;
    }
    void addTag(const uint64_t tag) {
        this->tags |= tag;
    }
    void addTags(const ArrayRef<uint64_t> tags) {
        for (const auto T: tags)
            this->tags |= T;
    }
    void operator+=(uint64_t v) {
        addTags(v);
    }
    void operator-=(uint64_t v) {
        clearTags(v);
    }
    uint64_t +(uint64_t tag) const {
        return getTags() | tag;
    }
    uint64_t -(uint64_t tag) const {
        return getTags() ^ tag;
    }
    uint64_t add(uint64_t tag) const {
        return getTags() | tag;
    }
    uint64_t del(uint64_t tag) const {
        return getTags() ^ tag;
    }
    void clearTags(const uint64_t tags_to_clear) {
        this->tag &= ~tags_to_clear;
    }
    uint64_t andTags(const uint64_t tag_to_clear) const {
        return this->tag & tag_to_clear;
    }
    enum CTypeKind getKind() const {
        return static_cast<enum CTypeKind>(this->tags >> 52);
    }
    uint64_t getAlign() const {
        return this->tags >> 56;
    }
    void setAlign(uint64_t align) {
        assert(llvm::isPowerOf2_64(align) && "Alignment is not a power of 2");
        auto ShiftValue = Log2_64(align);
        assert(ShiftValue < 64 && "Broken invariant");
        this->tags |= align >> 50;
    }
    void clearKind() const {
        this->tags &= 0xfffffff0ULL;
    }
    void setKind(enum CTypeKind kind) {
        assert(static_cast<uint64_t>(kind) <= 0b1111 && "invalid CTypeKind");
        clearKind();
        this->tags |= static_cast<uint64_t>(kind) << 60;
    }
    static uint64_t getFloatingBit() { return 0x800000000000000ULL; }
    bool isInteger() const {
        return !(this->tags & getFloatingBit());
    }
    bool isFloating() const {
        return this->tags & getFloatingBit();
    }
    void toggleIntegerBit() {
        this->tags ^= getFloatingBit();
    }
    static uint64_t getSignedBit() {
        return 0x400000000000000ULL;
    }
    bool isSigned() const {
        return this->tags & getSignedBit();
    }
    bool isUnsigned() const {
        return !(this->tags & getSignedBit());
    }
    void toggleSign() {
        this->tags ^= getSignedBit();
    }
    uint64_t getBitWidthForInteger() {
        assert(isInteger());
        return this->tags >> 58;
    }
    FloatKind getFloatKind() {
        assert(isFloating());
        return FloatKind(static_cast<enum FloatKindEnum>(this->tags >> 62));
    }
    uint64_t getBitWidth() const {
        if (isInteger())
            return getBitWidthForInteger();
        return getFloatKind().getBitSize();
    }
    void setIntergerBithWidth(uint64_t Size) {
        assert(isInteger());
        assert(!getBitWidthForInteger() && "already set size!");
        this->tags |= Size << 62;
    }
    void setFloatKind(FloatKind k) {
        assert(getFloatKind() != F_Invalid);
        assert(k.isValid());
        this->tags |= Size << 62;
    }
    bool isScalar() const {
        auto k = getKind();
        return k == TYPOINTER || (k == TYPRIM && !(this->tags & TYVOID));
    }
    void noralize() {
        constexpr type_tag_t mask = ty_storages;
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
