#include "config.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/STLArrayExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Compiler.h>
#include <llvm/Support/ConvertUTF.h>
#include <llvm/Support/Errno.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/VersionTuple.h>
#include <llvm/Support/TargetParser.h>
#include <llvm/Support/AArch64TargetParser.h>
#include <llvm/Support/X86TargetParser.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/Support/RISCVISAInfo.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Frontend/OpenMP/OMPGridValues.h>
#include <llvm/Support/CSKYTargetParser.h>
#include <llvm/BinaryFormat/Magic.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/Compression.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/ExitCodes.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/ScopedPrinter.h>
#include <llvm/Support/Threading.h>
#include <llvm/Support/ARMTargetParser.h>
#include <llvm/Support/MD5.h>
#include <llvm/Support/StringSaver.h>
#include <llvm/Support/ScopedPrinter.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/YAMLParser.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Option/Option.h>
#include <llvm/Option/OptSpecifier.h>
#include <llvm/Option/ArgList.h>
#include <llvm/Option/Arg.h>
#include <llvm/ProfileData/InstrProf.h>

// va_list, va_arg
#include <cstdarg>
#include <cstdint>

// open/read/close
#include <algorithm> // std::max
#include <cmath>     // pow
#include <cstdint>   // intmax_t, uintmax_t
#include <cstdio>
#include <ctime>
#include <deque>

#if !WINDOWS
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <string>
#include <vector>
#include <map>

#if !CC_NO_RAEADLINE
#include <readline/history.h>  // add_history
#include <readline/readline.h> // readline
#endif

#include <cctype>

#if CC_DEBUG
  #define dbgprint(msg, ...) fprintf(stderr, "\33[01;33m[DEBUG]\33[0m " msg, ##__VA_ARGS__) 
#else
  #define dbgprint(...) (void)0
#endif

#if CC_STATICS
  #define endStatics() fputc(10, stderr)
  #define statics(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__) 
#else
  #define endStatics() (void)0
  #define statics(...) (void)0
#endif

namespace xcc {
#if WINDOWS
HANDLE
hStdin = GetStdHandle(STD_INPUT_HANDLE), hStdout = GetStdHandle(STD_OUTPUT_HANDLE),
                                         hStderr = GetStdHandle(STD_ERROR_HANDLE);
#endif

using llvm::APFloat;
using llvm::APInt;
using llvm::ArrayRef;
using llvm::cast;
using llvm::cast_if_present;
using llvm::cast_or_null;
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::DenseMap;
using llvm::dyn_cast;
using llvm::dyn_cast_if_present;
using llvm::dyn_cast_or_null;
using llvm::GlobalValue;
using llvm::isa;
using llvm::isa_and_nonnull;
using llvm::isa_and_present;
using llvm::raw_fd_ostream;
using llvm::raw_ostream;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::Twine;
using llvm::sys::ProcessInfo;
using llvm::MutableArrayRef;
using llvm::None;
using llvm::Optional;
using llvm::LLVMContext;
using llvm::OwningArrayRef;
using llvm::SaveAndRestore;
using llvm::VersionTuple;
using llvm::Expected;
using llvm::IntrusiveRefCntPtr;
using llvm::IntrusiveRefCntPtrInfo;
using llvm::RefCountedBase;
using type_tag_t = uint64_t;
constexpr APFloat::cmpResult cmpGreaterThan = APFloat::cmpGreaterThan, cmpEqual = APFloat::cmpEqual,
                                             cmpLessThan = APFloat::cmpLessThan;
enum TagType: uint8_t {
    TagType_Enum,
    TagType_Struct,
    TagType_Union
};
enum StringPrefix: uint8_t {
    Prefix_none,// An integer character constant has type int.
    Prefix_u8, // A UTF-8 character constant has type char8_t. (unsigned)
    Prefix_L,  // A wchar_t character constant prefixed by the letter L has type wchar_t
    Prefix_u,  // A UTF-16 character constant has type char16_t (unsigned)
    Prefix_U,  // A UTF-32 character constant has type char32_t (unsigned)
};
const char *show(enum TagType tag) {
    switch (tag) {
    case TagType_Enum: return "enum";
    case TagType_Struct: return "struct";
    case TagType_Union: return "union";
    }
    llvm_unreachable("invalid enum TagType");
}
enum PostFixOp {
    PostfixIncrement = 1,
    PostfixDecrement
};
// https://github.com/llvm-mirror/clang/blob/master/include/clang/AST/OperationKinds.def
enum UnaryOp {
    UNeg = 1,
    SNeg,
    FNeg,
    CNeg, // complex negate
    Not,
    CConj, // complex conjugate(conj(), `~`)
    AddressOf,
    Dereference,
    LogicalNot,
    C__real__,
    C__imag__,
    ToBool
};
enum BinOp {
    // Arithmetic operators

    // unsigned addition
    UAdd = 1,
    // signed addition
    SAdd,
    // float addition
    FAdd,
    // complex addition(signed or unsigned)
    CAdd,
    // float complex addition
    CFAdd,
    // unsigned subtraction
    USub,
    // signed subtraction
    SSub,
    // float subtraction
    FSub,
    // complex subtraction(signed or unsigned)
    CSub,
    // float complex subtraction
    CFSub,
    // unsigned multiplication
    UMul,
    // signed multiplication
    SMul,
    // float multiplication
    FMul,
    // complex multiplication(signed or unsigned)
    CMul,
    // unsigned division
    CFMul,
    // unsigmed complex multiplication
    UDiv,
    // signed division
    SDiv,
    // float division
    FDiv,
    // signed complex division
    CSDiv,
    // unsigned complex division
    CUDiv,
    // float complex division
    CFDiv,
    // unsigned remainder
    URem,
    // signed remainder
    SRem,
    // float remainder(extension)
    FRem,
    // logical right shift: (a >> b), a is unsigned
    Shr,
    // arithmetic right shift(x86: SAR): (a >> b), a is signed
    AShr,
    // shift left: (a << b)
    Shl,
    // bitwise And: (a & b)
    And,
    // bitwise Xor: (a ^ b)
    Xor,
    // bitwise Or: (a | b)
    Or,

    // logical And: (a && b) => (a ? b : false)
    LogicalAnd,
    // logical Or: (a || b) => (a ? true : b)
    LogicalOr,
    // assignment operator: (a += b) => (a = a + b)
    //                      (++a) => (a = a + 1)
    //                      result type is a'type, a must be a lvalue.
    Assign,
    // add integer to pointer: ((int*)a + b) => ((uintptr)a + sizeof(b)), result type is a's type
    SAddP,
    // pointer differece: ((int*)a - (int*)b) => (((uintptr)a - (uintptr)b) / 4), result type is ptrdiff_t1
    PtrDiff,
    // the comma operator: (a, b)
    Comma,
    // construct a complex number: (a + bi)
    Complex_CMPLX,

    // Atomic ops
    AtomicrmwAdd,
    AtomicrmwSub,
    AtomicrmwAnd,
    AtomicrmwOr,
    AtomicrmwXor,

    // compare operators
    EQ,
    NE,
    UGT,
    UGE,
    ULT,
    ULE,
    SGT,
    SGE,
    SLT,
    SLE,
    // ordered float compare operators
    FEQ,
    FNE,
    FGT,
    FGE,
    FLT,
    FLE,
    // complex compare operators
    CEQ,
    CNE
};
enum CastOp {
    Trunc = 1,
    ZExt,
    SExt,
    FPToUI,
    FPToSI,
    UIToFP,
    SIToFP,
    FPTrunc,
    FPExt,
    PtrToInt,
    IntToPtr,
    BitCast
};
static const char *show(enum BinOp op) {
#define BINOP3(two, r)                                                                                                 \
    case U##two:                                                                                                       \
    case S##two:                                                                                                       \
    case F##two: return r
    switch (op) {
        BINOP3(Add, "+");
        BINOP3(Sub, "-");
        BINOP3(Mul, "*");
        BINOP3(Div, "/");
        BINOP3(Rem, "%");
    case CAdd: return "+";
    case CFAdd: return "+";
    case CSub: return "-";
    case CFSub: return "-";
    case CMul: return "*";
    case CFMul: return "*";
    case CFDiv:
    case CUDiv:
    case CSDiv: return "/";
    case Shr:
    case AShr: return ">>";
    case Shl: return "<<";
    case And: return "&";
    case Xor: return "^";
    case Or: return "|";
    case LogicalAnd: return "&&";
    case LogicalOr: return "||";
    case Assign: return "=";
    case SAddP: return "+";
    case PtrDiff: return "-";
    case Comma: return ",";
    case Complex_CMPLX: return " + j";
    case AtomicrmwAdd: return "+";
    case AtomicrmwSub: return "-";
    case AtomicrmwAnd: return "&";
    case AtomicrmwOr: return "|";
    case AtomicrmwXor: return "^";
    case EQ:
    case FEQ:
    case CEQ: return "==";
    case NE:
    case FNE:
    case CNE: return "!=";
#define CMPOP3(c, r)                                                                                                   \
    case U##c:                                                                                                         \
    case S##c:                                                                                                         \
    case F##c: return r
        CMPOP3(GT, ">");
        CMPOP3(GE, ">=");
        CMPOP3(LT, "<");
        CMPOP3(LE, "<=");
    }
    llvm_unreachable("bad binary operator");
}
static const char *show(enum UnaryOp o) {
    switch (o) {
    case UNeg:
    case SNeg:
    case CNeg:
    case FNeg: return "-";
    case CConj:
    case Not: return "~";
    case AddressOf: return "&";
    case Dereference: return "*";
    case LogicalNot: return "!";
    case C__real__: return "__real__ ";
    case C__imag__: return "__imag__ ";
    case ToBool: return "(_Bool)";
    default: return "(unknown unary operator)";
    }
}
static const char *show(enum PostFixOp o) {
    switch (o) {
    case PostfixIncrement: return "++";
    case PostfixDecrement: return "--";
    default: return "(unknown postfix operator)";
    }
}
static const char *show(enum CastOp op) {
    switch (op) {
        case Trunc: return "trunc";
        case ZExt: return "zext";
        case SExt: return "sext";
        case FPToUI: return "fptoui";
        case FPToSI: return "fptosi";
        case UIToFP: return "uitosp";
        case SIToFP: return "sitofp";
        case FPTrunc: return "fptrunc";
        case FPExt: return "fpext";
        case PtrToInt: return "ptrToInt";
        case IntToPtr: return "intToPtr";
        case BitCast: return "bitcast";
    }
    llvm_unreachable("invalid CastOp");
}
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
// Octuple-precision floating-point are not supported
// TODO: Decimal Float
// https://discourse.llvm.org/t/rfc-decimal-floating-point-support-iso-iec-ts-18661-2-and-c23/62152
enum FloatKindEnum: uint8_t {
    F_Invalid,
    F_Half, // https://en.wikipedia.org/wiki/Half-precision_floating-point_format
    F_BFloat, // https://en.wikipedia.org/wiki/Bfloat16_floating-point_format
    F_Float, // https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    F_Double, // https://en.wikipedia.org/wiki/Double-precision_floating-point_format
    F_x87_80, // 80 bit float (X87)
    F_Quadruple, // https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
    F_PPC128, // https://gcc.gnu.org/wiki/Ieee128PowerPC
    F_Decimal32,
    F_Decimal64,
    F_Decimal128
};
struct FloatKind {
    static constexpr size_t MAX_KIND = static_cast<size_t>(F_Decimal128) - static_cast<size_t>(F_Half);
    enum FloatKindEnum e;
    constexpr FloatKind(enum FloatKindEnum k): e{k} { }
    constexpr FloatKind(uint64_t k): e{static_cast<enum FloatKindEnum>(k)} { }
    constexpr enum FloatKindEnum getKind() { return e; }
    explicit constexpr operator uint64_t() const { return static_cast<uint64_t>(e); }
    constexpr enum FloatKindEnum asEnum() const { return e; }
    constexpr bool isValid() const { return e != F_Invalid && e <= F_PPC128; }
    explicit constexpr operator bool() const { return e != F_Invalid; }
    constexpr bool equals(const enum FloatKindEnum other) const {
        return e == other;
    }
    uint64_t getBitWidth() const {
        switch (e) {
        case F_Half:
        case F_BFloat: return 16;
        case F_Float: return 32;
        case F_Double: return 64;
        case F_x87_80: return 80;
        case F_Quadruple:
        case F_PPC128: return 128;
        case F_Decimal32: return 32;
        case F_Decimal64: return 64;
        case F_Decimal128: return 128;
        case F_Invalid: break;
        }
        llvm_unreachable("broken type: invalid FloatKindEnum");
    };
    llvm::Type *toLLVMType(LLVMContext &ctx) const {
        switch (e) {
        case F_Half: return llvm::Type::getHalfTy(ctx);
        case F_BFloat: return llvm::Type::getBFloatTy(ctx);
        case F_Float: return llvm::Type::getFloatTy(ctx);
        case F_Double: return llvm::Type::getDoubleTy(ctx);
        case F_x87_80: return llvm::Type::getX86_FP80Ty(ctx);
        case F_Quadruple: return llvm::Type::getFP128Ty(ctx);
        case F_PPC128: return llvm::Type::getPPC_FP128Ty(ctx);
        case F_Invalid: break;
        default: break;
        }
        llvm_unreachable("Decimal floating are not supported now!");
    }
    const llvm::fltSemantics &getFltSemantics() const {
        switch (e) {
        case F_Half: return APFloat::IEEEhalf();
        case F_BFloat: return APFloat::BFloat();
        case F_Float: return APFloat::IEEEsingle();
        case F_Double: return APFloat::IEEEdouble();
        case F_x87_80: return APFloat::x87DoubleExtended();
        case F_Quadruple: return APFloat::IEEEquad();
        case F_PPC128: return APFloat::PPCDoubleDouble();
        case F_Invalid: break;
        default: break;
        }
        llvm_unreachable("Decimal floating are not supported now!");
    }
    StringRef show() const {
        switch (e) {
            case F_Half: return "half";
            case F_BFloat: return "__bf16";
            case F_Float: return "float";
            case F_Double: return "double";
            case F_x87_80: return "__float80";
            case F_Quadruple: return "__float128";
            case F_PPC128: return "__ibm128";
            case F_Decimal32: return "_Decimal32";
            case F_Decimal64: return "_Decimal64";
            case F_Decimal128: return "_Decimal128";
            case F_Invalid: break;
        }
        return "(invalid FloatKind)";
    }
    bool isIEEE() const {
        return APFloat::getZero(getFltSemantics()).isIEEE();
    }
    APFloat getZero() const {
        return APFloat::getZero(getFltSemantics());
    }
    ConstantFP *getZero(LLVMContext &ctx) const {
        return ConstantFP::get(ctx, getZero());
    }
};
struct IntegerKind {
    uint8_t shift;
    constexpr IntegerKind(uint8_t shift): shift{shift} {}
    constexpr bool isValid() const {
        return shift != 1 && shift != 2 && shift <= 7;
    }
    explicit constexpr operator bool() const {
        return shift;
    }
    explicit constexpr operator uint64_t() const {
        return shift;
    }
    constexpr uint8_t asLog2() const {
        return shift;
    }
    constexpr uint64_t asBits() const {
        return uint64_t(1) << shift;
    }
    constexpr StringRef show(bool Signed) const {
        switch (shift) {
            case 0:
            case 1: return "_Bool"; // XXX: C23 is bool
            case 3: return Signed ? StringRef("char") : StringRef("unsigned char");
            case 4: return Signed ? StringRef("short") : StringRef("unsigned short");
            case 5: return Signed ? StringRef("int") : StringRef("unsigned int"); // XXX: 'unsigned' is ok
            case 6: return Signed ? StringRef("long long") : StringRef("unsigned long long");
            case 7: return Signed ? StringRef("__int128") : StringRef("unsigned __int128");
            default: break;
        }
        return "(invalid IntegerKind)";
    }
    constexpr uint64_t getBitWidth() const { return asBits(); }
    uint64_t asBytes() const {
        uint64_t bits = asBits();
        assert((bits % 8) == 0 && "invalid call to asBytes(): loss information!");
        return bits / 8;
    }
    constexpr bool operator==(const IntegerKind &other) const {
        return asLog2() == other.asLog2();
    }
    constexpr static IntegerKind fromLog2(uint8_t Value) {
        return IntegerKind(Value);
    }
    static IntegerKind fromBytes(uint64_t Bytes) {
        assert(llvm::isPowerOf2_64(Bytes));
        return fromLog2(llvm::Log2_64(Bytes));
    }
    static IntegerKind fromBits(uint64_t Bits) {
        assert(llvm::isPowerOf2_64(Bits));
        return fromBytes(llvm::Log2_64(Bits));
    }
    APInt getZero() const {
        return APInt::getZero(asBits());
    }
    bool isBool() const { 
        return shift == 0;
    }
    llvm::Type *toLLVMType(LLVMContext &ctx) const {
        return llvm::IntegerType::get(ctx, asBits());
    }
    ConstantInt *getZero(LLVMContext &ctx) const {
        return ConstantInt::get(ctx, getZero());
    }
};
static constexpr uint64_t 
   build_integer(IntegerKind kind, bool Signed = false),
   build_float(FloatKind kind);

#include "option.cpp"
#include "Arena.cpp"
#include "tokens.inc"
#include "IdentifierTable.h"
#include "tempAllocator.h"
#include "types.inc"
#include "xint128.cpp"
#include "xstring.h"
#include "xvector.h"

constexpr type_tag_t
  storage_class_specifiers = TYTYPEDEF | TYEXTERN | TYSTATIC | TYTHREAD_LOCAL | TYREGISTER | TYCONSTEXPR| TYAUTO,
  type_qualifiers = TYCONST | TYRESTRICT | TYVOLATILE | TYATOMIC,
  type_qualifiers_and_storage_class_specifiers = type_qualifiers | storage_class_specifiers;

#define kw_start Kextern
#define kw_end K_Generic
typedef unsigned label_t;
#define INVALID_LABEL ((label_t)-1)
typedef uint32_t Codepoint;
typedef struct OpaqueStmt *Stmt;
typedef const struct OpaqueStmt *const_Stmt;
typedef struct OpaqueExpr *Expr;
typedef const struct OpaqueExpr *const_Expr;
typedef struct OpaqueCType *CType;
typedef const struct OpaqueCType *const_CType;

typedef unsigned line_t;
typedef unsigned column_t;
typedef unsigned fileid_t;
typedef uint32_t location_t;

static bool location_is_stdin(location_t loc) {
    return loc & 0xf0000000;
}
static location_t location_as_index(location_t loc) {
    return loc & 0x7fffffff;
}

static const char hexs[] = "0123456789ABCDEF";
static char hexed(unsigned a) { return hexs[a & 0b1111]; }
struct SourceRange {
    location_t start, end;
    SourceRange(): start{0}, end{0} {};
    SourceRange(location_t loc) : start{loc}, end{loc} {}
    SourceRange(location_t L, location_t R): start{L}, end{R} {}
    location_t getStart() const {
        return start;
    }
    location_t getEnd() const {
        return end;
    }
    bool contains(location_t loc) const {
        return loc >= start && loc <= end;
    }
    bool isValid() const {
        return start != 0 && end != 0;
    }
    bool isInValid() const {
        return start == 0 || end == 0;
    }
    bool isSignle() const {
        return start == end;
    }
    bool operator==(const SourceRange &other) {
        return start == other.start && end == other.end;
    }
    void dump() const {
        llvm::errs() << "SourceRange(" << start << ", " << end << ")";
    }
};
struct FixItHint {
    StringRef code;
    location_t insertPos;
    static FixItHint CreateInsertion(const StringRef &code, location_t insertPos) {
        assert(code.size() && "empty FixItHint Insertion");
        FixItHint Hint;
        Hint.code = code;
        Hint.insertPos = insertPos;
        return Hint;
    }
};
struct SourceLine {
    SmallString<0> sourceLine;
    SmallString<0> CaretLine;
    location_t startLoc, insertLoc;
    void clear() {
        sourceLine.clear();
        CaretLine.clear();
    }
    void setInsertLoc(location_t loc) {
        startLoc = (insertLoc = loc);
    }
    std::string buildFixItInsertionLine(const ArrayRef<FixItHint> FixItHints) const {
        assert(!FixItHints.empty());
        std::string line(CaretLine.size(), ' ');
        location_t loc = startLoc;
        for (size_t Repeat = line.size();--Repeat;loc++) {
            for (const auto &it : FixItHints) {
                if (it.insertPos == loc) {
                    size_t FixItinsertLoc = static_cast<size_t>(loc) - static_cast<size_t>(startLoc);
                    size_t end = FixItinsertLoc + it.code.size();
                    line.resize(end, ' ');
                    size_t c = 0;
                    while (FixItinsertLoc < end) {
                        line[FixItinsertLoc++] = it.code[c++];
                    }
                }
            }
        }
        return line;
    }
    bool push_back(char c, location_t caretLoc, const ArrayRef<SourceRange> ranges) {
        switch (c) {
        case '\n':
        case '\r': return true;
        case '\t':
            CaretLine.append(CC_SHOW_TAB_SIZE, ' ');
            sourceLine.append(CC_SHOW_TAB_SIZE, ' ');
            break;
        default:
            if (!llvm::isPrint(c)) { /*std::iscntrl(c)*/ 
                    sourceLine.push_back('<');
                    sourceLine.push_back('0');
                    sourceLine.push_back('x');
                    sourceLine.push_back(hexed(c) >> 4);
                    sourceLine.push_back(hexed(c));
                    sourceLine.push_back('>');
                    CaretLine.append(' ', 6);
                } else {
                    CaretLine.push_back(' ');
                    for (const auto it: ranges) {
                        if (it.contains(insertLoc)) {
                            CaretLine.back() = '~';
                        }
                    }
                    if (caretLoc == insertLoc)
                        CaretLine.back() = '^';
                    sourceLine.push_back(c);
                }
        }
        insertLoc++;
        return false;
    }
};

struct source_location {
    line_t line = 0;
    column_t col = 0;
    fileid_t fd = 0;
    struct LocTree *tree = nullptr;
    SourceLine source_line;
    bool isValid() const { return line != 0; }
    source_location(line_t line = 0, column_t col = 0, fileid_t fd = 0): line{line}, col{col}, fd{fd} {}
    bool operator >(const source_location &rhs) {
        return line > rhs.line && col > rhs.col;
    }
    bool operator <(const source_location &rhs) {
        return line < rhs.line && col < rhs.col;
    }
    bool operator >=(const source_location &rhs) {
        return line >= rhs.line && col >= rhs.col;
    }
    bool operator <=(const source_location &rhs) {
        return line <= rhs.line && col <= rhs.col;
    }
};
struct Include_Info {
    line_t line; // the line where #include occurs
    fileid_t fd; // the file ID in SourceMgr's streams
};
struct LocTree {
    struct LocTree *parent;
    union {
        Include_Info *include;
        struct PPMacroDef *macro;
    };
    bool isAInclude;
    LocTree(struct LocTree *parent, Include_Info *include = nullptr): parent{parent}, isAInclude{true} {
        this->include = include;
    }
    LocTree(struct LocTree *parent, struct PPMacroDef *def = nullptr): parent{parent}, isAInclude{false} {
        this->macro = def;
    }
    void setParent(LocTree *theParent) { this->parent = theParent; }
    LocTree *getParent() const { return parent; }
};
struct Declator {
    IdentRef name;
    CType ty;
    Declator(IdentRef Name = nullptr, CType ty = nullptr) : name{Name}, ty{ty} { }
};
struct Param : public Declator {
    Param(IdentRef Name = nullptr, CType ty = nullptr) : Declator{Name, ty} { }
    Param(const Declator &decl) : Declator(decl) { }
};
struct EnumPair {
    IdentRef name;
    uint64_t val;
};
struct VarDecl {
    IdentRef name;
    CType ty;
    Expr init; // maybe null
    size_t idx;
};
struct SwitchCase {
    location_t loc;
    const APInt *CaseStart;
    label_t label;
    SwitchCase(location_t loc, label_t label, const APInt *CastStart) : loc{loc}, CaseStart{CastStart}, label{label} { }
    static bool equals(const SwitchCase &lhs, const SwitchCase &rhs) {
        return *lhs.CaseStart == *rhs.CaseStart;
    }
};
struct GNUSwitchCase : public SwitchCase {
    APInt range;
    GNUSwitchCase(location_t loc, label_t label, const APInt *CastStart, const APInt *CaseEnd)
        : SwitchCase(loc, label, CastStart), range{*CaseEnd - *CastStart} { }
    bool contains(const APInt &C) const {
        return (C - *CaseStart).ule(range);
    }
    bool contains(const SwitchCase &G) const {
        return contains(*G.CaseStart);
    }
    bool overlaps(const GNUSwitchCase &G) const {
        // https://stackoverflow.com/questions/3269434/whats-the-most-efficient-way-to-test-if-two-ranges-overlap
        // G   :  [xxxxxxx]
        // this:     [yyyyyyy]
        if (G.CaseStart->ult(*CaseStart)) {
            return (*G.CaseStart + G.range).uge(*CaseStart);
        }
        // G   :      [xxxxxxx]
        // this: [yyyyyyy]
        return (*CaseStart + range).ule(*G.CaseStart);
    }
};
#include "ctypes.inc"
#include "expressions.inc"
#include "printer.cpp"
#include "statements.inc"
#include "utf8.cpp"
struct ReplacedExprParen {
  enum ExprKind k=EConstant;
  CType ty;
  struct {
    llvm::Constant* C;
    size_t id;
    IdentRef varName;
    location_t ReplacedLoc;
    location_t paren_loc[2];
  };
};
struct ReplacedExpr {
  enum ExprKind k=EConstant;
  CType ty;
  struct {
    llvm::Constant* C;
    size_t id;
    IdentRef varName;
    location_t ReplacedLoc;
  };
};
location_t *OpaqueExpr::getParenLLoc() {
    if (ty->hasTag(TYREPLACED_CONSTANT)) {
        return reinterpret_cast<ReplacedExprParen*>(this)->paren_loc;
    }
    const size_t Size = expr_size_map[k];
    return reinterpret_cast<location_t*>(reinterpret_cast<uintptr_t>(this) + Size);
}
const location_t *OpaqueExpr::getParenLLoc() const {
    return const_cast<Expr>(this)->getParenLLoc();
}
const location_t *OpaqueExpr::getParenRLoc() const {
    return getParenLLoc() + 1;
}
location_t *OpaqueExpr::getParenRLoc() {
    return getParenLLoc() + 1;
}
location_t OpaqueExpr::getBeginLoc() const {
    if (ty->hasTag(TYPAREN)) 
        return *getParenLLoc();
    switch (k) {
    case EVar: return varLoc;
    case EBin: return lhs->getBeginLoc();
    case EUnary: return opLoc;
    case ECast: return castval->getBeginLoc();
    case ESubscript: return left->getBeginLoc();
    case EConstant: return constantLoc;
    case ECondition: return cond->getBeginLoc();
    case ECall: return callfunc->getBeginLoc();
    case EConstantArray: return constantArrayLoc;
    case EMemberAccess: return obj->getBeginLoc();
    case EArrToAddress: return arr3->getBeginLoc();
    case EPostFix: return poperand->getBeginLoc();
    case EArray: return ArrayStartLoc;
    case EStruct: return StructStartLoc;
    case EVoid: return voidStartLoc;
    case EConstantArraySubstript: return constantArraySubscriptLoc;
    }
    llvm_unreachable("invalid Expr");
}
location_t OpaqueExpr::getEndLoc() const {
    if (ty->hasTag(TYPAREN)) 
        return *getParenRLoc();
    switch (k) {
    case EConstant:
        return constantEndLoc ? constantLoc : constantEndLoc;
    case EBin:
        return rhs->getEndLoc();
    case EUnary:
        return uoperand->getEndLoc();
    case ESubscript:
        return right->getEndLoc();
    case EConstantArraySubstript:
        return constantArraySubscriptLocEnd;
    case EConstantArray:
        return constantArrayLocEnd;
    case EVoid:
        return voidexpr->getEndLoc();
    case ECast:
        return castval->getEndLoc();
    case EVar:
        return varLoc + varName->getKey().size() - 1;
    case EMemberAccess:
        return memberEndLoc;
    case EArrToAddress:
        return arr3->getEndLoc();
    case ECondition: 
        return cright->getEndLoc();
    case ECall:
        return callEnd;
    case EPostFix:
        return postFixEndLoc;
    case EArray:
        return ArrayEndLoc;
    case EStruct:
        return StructEndLoc;
    }
    llvm_unreachable("invalid Expr");
}
static constexpr uint64_t build_integer(IntegerKind kind, bool Signed) {
    const uint64_t log2size = kind.asLog2();
    return (log2size << 47) | (Signed ? OpaqueCType::sign_bit : 0ULL);
}
static constexpr uint64_t build_float(FloatKind kind) {
    const uint64_t k = static_cast<uint64_t>(kind);
    return (k << 47) | OpaqueCType::integer_bit;
}
static enum CTypeKind transform(enum TagType tag) {
    switch (tag) {
    case TagType_Union: return TYUNION;
    case TagType_Struct: return TYSTRUCT;
    case TagType_Enum: return TYENUM;
    }
    llvm_unreachable("bad TagType");
}
static enum TagType transform(enum CTypeKind k) {
    switch (k) {
    case TYSTRUCT: return TagType_Struct;
    case TYUNION: return TagType_Union;
    case TYENUM: return TagType_Enum;
    default: llvm_unreachable("invalid argument to transform()");
    }
}
static const char *show2(enum CTypeKind k) {
    switch (k) {
    case TYENUM: return "enum";
    case TYUNION: return "union";
    case TYSTRUCT: return "struct";
    default: llvm_unreachable("invalid argument to show_transform()");
    }
}
static const char *show2(enum TagType k) {
    switch (k) {
    case TagType_Enum: return "enum";
    case TagType_Union: return "union";
    case TagType_Struct: return "struct";
    default: llvm_unreachable("invalid argument to show_transform()");
    }
}


static bool is_declaration_specifier(Token a) { return a >= Kextern && a <= Kvolatile; }

static bool type_equal(CType a, CType b) {
    assert(a && "type_equal: a is nullptr");
    assert(b && "type_equal: b is nullptr");
    if (a->getKind() != b->getKind())
        return false;
    switch (a->getKind()) {
        case TYPRIM:
            return a->basic_equals(b);
        case TYPOINTER: return type_equal(a->p, b->p);
        case TYENUM:
        case TYSTRUCT:
        case TYUNION: return a == b;
        case TYINCOMPLETE: return a->name == b->name;
        default: return false;
    }
}
static bool compatible(CType p, CType expected) {
    // https://en.cppreference.com/w/c/language/type
    if (p->getKind() != expected->getKind())
        return false;
    switch (p->getKind()) {
        case TYPRIM: return p->basic_equals(expected);
        case TYFUNCTION:
            if (!compatible(p->ret, expected->ret) || p->params.size() != expected->params.size())
                return false;
            for (unsigned i = 0; i < expected->params.size(); ++i)
                if (!compatible(p->params[i].ty, expected->params[i].ty))
                    return false;
            return true;
        case TYSTRUCT:
        case TYENUM:
        case TYUNION: return p == expected;
        case TYPOINTER: {
            // ignore TYLVALUE attribute
            return p->isNullPtr_t() || expected->isNullPtr_t() || p->p->isVoid() || expected->p->isVoid() ||
                   (
                    p->andTags(type_qualifiers) == expected->andTags(type_qualifiers) &&
                    compatible(p->p, expected->p)
                  );
        }
        case TYINCOMPLETE: return p->tag == expected->tag && p->name == expected->name;
        case TYBITFIELD: llvm_unreachable("");
        case TYARRAY:
            if (p->hassize != expected->hassize)
                return false;
            if (p->hassize && (p->arrsize != expected->arrsize))
                return false;
            return compatible(p->arrtype, expected->arrtype);
    }
    llvm_unreachable("bad CTypeKind");
}

// TokenV: A Token with a value, and a macro is a sequence of TokenVs
enum TokenVKind : uint8_t {
    ATokenVBase,
    ATokenIdent,
    ATokenVNumLit,
    ATokenVStrLit,
    ATokenVChar,
    ATokenVLoc
};

struct TokenV {
    enum TokenVKind k = ATokenVBase; // 8 bits
    Token tok;                       // 8 bits
    union {
        // containing the encoding, .e.g: 8, 16, 32
        xstring str;
        IdentRef s; // 64 bits
        struct {
            uint8_t i;
            uint8_t itag;
        };
        struct LocTree *tree;
    };
    TokenV(): k{ATokenVBase}, tok{TNul} {};
    TokenV(enum TokenVKind k, Token tok) : k{k}, tok{tok} { }
    TokenV(Token tok) : k{ATokenVBase}, tok{tok} { }
    TokenV(struct LocTree *tree) : k{ATokenVLoc}, tok{PPMacroTraceLoc} { this->tree = tree; } 
    enum StringPrefix getStringPrefix() {
        auto r = static_cast<enum StringPrefix>(static_cast<unsigned char>(str.back()));
        str.pop_back();
        return r;
    }
    void setStringPrefix(enum StringPrefix enc = Prefix_none) {
        str.push_back(static_cast<char>(enc));
    }
    enum StringPrefix getCharPrefix() const {
        return static_cast<enum StringPrefix>(itag);
    }
    void dump(raw_ostream &OS) const {
        switch (k) {
        case ATokenVBase: OS << show(tok); break;
        case ATokenIdent:
            OS.write_escaped(s->getKey());
            // OS << '(' << show(tok) << '-' << show(s->second.getToken()) << ')';
            break;
        case ATokenVNumLit: OS << str.str(); break;
        case ATokenVStrLit: OS.write_escaped(str.str()); break;
        case ATokenVChar:
            if (isprint(i)) {
                OS << '\'' << i << '\'';
            } else {
                (OS << "<0x").write_hex(i) << '>';
            }
            break;
        default: llvm_unreachable("bad TokenV kind");
        }
    }
};
enum PPMacroKind : uint8_t {
    MOBJ,
    MFUNC
};
struct PPMacro {
    static bool tokensEq(ArrayRef<TokenV> a, ArrayRef<TokenV> b) {
        if (a.size() != b.size())
            return false;
        for (unsigned i = 0; i < a.size(); ++i) {
            TokenV x = a[i];
            TokenV y = b[i];
            if (x.tok > TIdentifier)
                x.tok = TIdentifier;
            if (y.tok > TIdentifier)
                y.tok = TIdentifier;
            if (x.tok != y.tok)
                return false;
            switch (x.tok) {
            case TStringLit:
            case PPNumber:
                if (x.str.str() != y.str.str())
                    return false;
                break;
            case TIdentifier:
                if (x.s != y.s)
                    return false;
                break;
            default: break;
            }
        }
        return true;
    }
    IdentRef Name;
    enum PPMacroKind k = MOBJ; // 1 byte
    bool ivarargs = false;     // 1 byte
    SmallVector<TokenV, 20> tokens{};
    SmallVector<IdentRef, 0> params{};
    bool equals(PPMacro &other) {
        return ((k == other.k) && (k == MFUNC ? (ivarargs == other.ivarargs) : true) && tokensEq(tokens, other.tokens));
    }
};
struct PPMacroDef {
    PPMacro m;
    location_t loc;
};
static unsigned intRank(const_CType ty) {
    return ty->getIntegerKind().asLog2();
}
static unsigned floatRank(const_CType ty) {
    return ty->getFloatKind().getBitWidth();
}
static unsigned scalarRankNoComplex(const_CType ty) {
    if (ty->isFloating())
        return floatRank(ty) + 100; // a dummy number
    return intRank(ty);
}
static unsigned scalarRank(const_CType ty) {
    // _Imaginary' Rank types as floating type's
    if (ty->isComplex())
        return scalarRankNoComplex(ty) + 1000; // a dummy number
    return scalarRank(ty);
}
#include "console.cpp"
#include "Diagnostic.cpp"
#include "JIT.cpp"
#include "LLVMDiagnosticHandler.cpp"
#include "Scope.cpp"
#include "State.cpp"
#include "SourceMgr.cpp"
#include "TextDiagnosticPrinter.cpp"
#include "codegen.cpp"
#include "lexer.h"
#include "output.cpp"
#include "StringPool.cpp"
#include "parser.cpp"
#include "lexer.cpp"
#ifdef XCC_MAIN
#include "Target/TargetInfo.h"
#include "toolchains/ToolChain.cpp"
#endif
} // namespace xcc
