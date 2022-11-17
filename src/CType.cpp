// CType - 64 bit unsigned integer
enum CTypeKind {};
enum FloatKindEnum {
    F_Half, // https://en.wikipedia.org/wiki/Half-precision_floating-point_format
    F_BFloat, // https://en.wikipedia.org/wiki/Bfloat16_floating-point_format
    F_Float, // https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    F_Double, // https://en.wikipedia.org/wiki/Double-precision_floating-point_format
    F_x87_80, // 80 bit float (X87)
    F_Quadruple, // https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
    F_PPC128 // https://gcc.gnu.org/wiki/Ieee128PowerPC
};
struct FloatKind {
    enum FloatKindEnum e;
    inline FloatKind(enum FloatKindEnum k) e{k} { }
    enum FloatKindEnum getKind() { return e; }
    inline operator enum FloatKindEnum() const { return e; }
    uint64_t getBitSize() const {
        switch (e) {
        case F_Half:
        case F_BFloat: return 16;
        case F_Float: return 32;
        case F_Double: return 64;
        case F_x87_80: return 80;
        case F_Quadruple:
        case F_PPC128: return 128;
        }
        llvm_unreachable("broken type: invalid FloatKindEnum");
    };
    llvm::Type *toLLVMType(LLVMContext &ctx) const {
        switch (e) {
        case F_Half: return llvm::Type::getHalfTy(ctx);
        case F_BFloat: return llvm::Type::getBFloatTy(ctx);
        case F_Float: return llvm::Type::getFloatTy(ctx);
        case F_Double: return llvm::Type::getDouble(ctx);
        case F_x87_80: return llvm::Type::getX86_FP80Ty(ctx);
        case F_Quadruple: return llvm::Type::getFP128Ty(ctx);
        case F_PPC128: return llvm::Type::getPPC_FP128Ty(ctx);
        }
        llvm_unreachable("broken type: invalid FloatKindEnum");
    }
    llvm::fltSemantics &llvm::getFltSemantics() const {
        switch (e) {
        case F_Half: return APFloat::IEEEhalf();
        case F_BFloat: return APFloat::BFloat();
        case F_Float: APFloat::IEEEsingle();
        case F_Double: return APFloat::IEEEdouble();
        case F_x87_80: return APFloat::x87DoubleExtended();
        case F_Quadruple: return APFloat::IEEEquad();
        case F_PPC128: return APFloat::PPCDoubleDouble();
        }
        llvm_unreachable("broken type: invalid FloatKindEnum");
    }
    bool isIEEE() const {
        return APFloat::getZero(getFltSemantics()).isIEEE();
    }
};

struct OpacheCType
{
    uint64_t tags;
    uint64_t getTags() const { return this->tags; }
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
    void addTags(const uint64_t tags) {
        this->tags |= tags;
    }
    enum CTypeKind getKind() const {
        return static_cast<enum CTypeKind>(this->tags >> 60);
    }
    void clearKind() const {
        this->tags &= 0xfffffff0ULL;
    }
    void setKind(enum CTypeKind kind) {
        assert(static_cast<uint64_t>(kind) <= 0b1111 && "invalid CTypeKind");
        clearKind();
        this->tags |= static_cast<uint64_t>(kind) << 60;
    }
    bool isInteger() const {
        return this->tags & 0x800000000000000ULL;
    }
    bool isFloating() const {
        return !(this->tags & 0x800000000000000ULL);
    }
    void toggleInteger() {
        this->tags ^= 0x800000000000000ULL;
    }
    bool isSigned() const {
        return this->tags & 0x400000000000000ULL;
    }
    bool isUnsigned() const {
        return !(this->tags & 0x400000000000000ULL);
    }
    void toggleSign() {
        this->tags ^= 0x400000000000000ULL;
    }
    uint64_t getBitWidthForInteger() {
        assert(isInteger());
        return this->tags >> 62;
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
};
static_assert(sizeof(enum FloatKindEnum) == sizeof(FloatKind), "The C++ compiler and XCC disagree on the size of FloatKind!\n");
static_assert(sizeof(CType) == sizeof(uint64_t), "The C++ compiler and XCC disagree on the size of CType!\n");
