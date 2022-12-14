#include "xcc_config.h"

#if !WINDOWS
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <windows.h>
#endif

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#if !CC_NO_RAEADLINE
#include <readline/history.h>  // add_history
#include <readline/readline.h> // readline
#endif

#include <llvm/Config/llvm-config.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/GlobalsModRef.h>
#include <llvm/Analysis/StackSafetyAnalysis.h>
#include <llvm/Analysis/ConstantFolding.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>
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
#include <llvm/ADT/GraphTraits.h>
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
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/IR/IntrinsicsAMDGPU.h>
#include <llvm/IR/IntrinsicsARM.h>
#include <llvm/IR/IntrinsicsBPF.h>
#include <llvm/IR/IntrinsicsHexagon.h>
#include <llvm/IR/IntrinsicsNVPTX.h>
#include <llvm/IR/IntrinsicsPowerPC.h>
#include <llvm/IR/IntrinsicsR600.h>
#include <llvm/IR/IntrinsicsRISCV.h>
#include <llvm/IR/IntrinsicsS390.h>
#include <llvm/IR/IntrinsicsWebAssembly.h>
#include <llvm/IR/IntrinsicsX86.h>
#include <llvm/IR/MatrixBuilder.h>
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
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Compiler.h>
#include <llvm/Support/ConvertUTF.h>
#include <llvm/Support/Errno.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/DOTGraphTraits.h>
#include <llvm/Support/GraphWriter.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/VersionTuple.h>
#include <llvm/Support/TargetParser.h>
#include <llvm/Support/AArch64TargetParser.h>
#include <llvm/Support/X86TargetParser.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/Support/RISCVISAInfo.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/CSKYTargetParser.h>
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
#include <llvm/BinaryFormat/Magic.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Frontend/OpenMP/OMPGridValues.h>
#include <llvm/Transforms/Coroutines/CoroCleanup.h>
#include <llvm/Transforms/Coroutines/CoroEarly.h>
#include <llvm/Transforms/Coroutines/CoroElide.h>
#include <llvm/Transforms/Coroutines/CoroSplit.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/AlwaysInliner.h>
#include <llvm/Transforms/IPO/LowerTypeTests.h>
#include <llvm/Transforms/IPO/ThinLTOBitcodeWriter.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Instrumentation.h>
#include <llvm/Transforms/Instrumentation/AddressSanitizer.h>
#include <llvm/Transforms/Instrumentation/AddressSanitizerOptions.h>
#include <llvm/Transforms/Instrumentation/BoundsChecking.h>
#include <llvm/Transforms/Instrumentation/DataFlowSanitizer.h>
#include <llvm/Transforms/Instrumentation/GCOVProfiler.h>
#include <llvm/Transforms/Instrumentation/HWAddressSanitizer.h>
#include <llvm/Transforms/Instrumentation/InstrProfiling.h>
#include <llvm/Transforms/Instrumentation/MemProfiler.h>
#include <llvm/Transforms/Instrumentation/MemorySanitizer.h>
#include <llvm/Transforms/Instrumentation/SanitizerCoverage.h>
#include <llvm/Transforms/Instrumentation/ThreadSanitizer.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/EarlyCSE.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/JumpThreading.h>
#include <llvm/Transforms/Scalar/LowerMatrixIntrinsics.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/CanonicalizeAliases.h>
#include <llvm/Transforms/Utils/Debugify.h>
#include <llvm/Transforms/Utils/EntryExitInstrumenter.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
#include <llvm/Transforms/Utils/NameAnonGlobals.h>
#include <llvm/Transforms/Utils/SymbolRewriter.h>

#include <cstdarg>
#include <cstdint>
#include <optional>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <deque>
#include <optional>
#include <cassert>
#include <type_traits>
#include <iterator>
#include <string>
#include <vector>
#include <map>
#include <memory>
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

#define HANDLE_EXTENSION(Ext) llvm::PassPluginLibraryInfo get##Ext##PluginInfo();
#include "llvm/Support/Extension.def"

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
using llvm::VersionTuple;
using llvm::DenseMap;
using llvm::dyn_cast;
using llvm::dyn_cast_if_present;
using llvm::dyn_cast_or_null;
using llvm::Expected;
using llvm::GlobalValue;
using llvm::IntrusiveRefCntPtr;
using llvm::IntrusiveRefCntPtrInfo;
using llvm::isa;
using llvm::isa_and_nonnull;
using llvm::isa_and_present;
using llvm::LLVMContext;
using llvm::MutableArrayRef;
using llvm::None;
using llvm::Optional;
using llvm::OwningArrayRef;
using llvm::raw_fd_ostream;
using llvm::raw_ostream;
using llvm::raw_svector_ostream;
using llvm::RefCountedBase;
using llvm::SaveAndRestore;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::Twine;
using type_tag_t = uint64_t;

constexpr APFloat::cmpResult cmpGreaterThan = APFloat::cmpGreaterThan, cmpEqual = APFloat::cmpEqual,
                             cmpLessThan = APFloat::cmpLessThan;
enum TagKind : uint8_t {
    TagType_Enum,
    TagType_Struct,
    TagType_Union
};
static const char *show(enum TagKind k) {
    switch (k) {
    case TagType_Enum: return "enum";
    case TagType_Union: return "union";
    case TagType_Struct: return "struct";
    }
    llvm_unreachable("invalid enum TagType");
}
template <typename T> struct WithAlloc {
    T *data;
    size_t Size;
    WithAlloc(size_t Size) : Size{Size} { data = new T[Size]; }
    template <typename T2> WithAlloc(const WithAlloc<T2> &) = delete;
    ~WithAlloc() { delete[] data; }
    T *begin() { return data; }
    T *end() { return data + Size; }
    size_t size() const { return Size; }
    T &operator[](size_t i) { return data[i]; }
    T *operator->() { return data; }
};
enum StringPrefix : unsigned char {
    Prefix_none, // An integer character constant has type int.
    Prefix_u8,   // A UTF-8 character constant has type char8_t. (unsigned)
    Prefix_L,    // A wchar_t character constant prefixed by the letter L has type wchar_t
    Prefix_u,    // A UTF-16 character constant has type char16_t (unsigned)
    Prefix_U,    // A UTF-32 character constant has type char32_t (unsigned)
};
enum PostFixOp: unsigned char {
    PostfixIncrement = 1,
    PostfixDecrement
};
// https://github.com/llvm-mirror/clang/blob/master/include/clang/AST/OperationKinds.def
enum UnaryOp: unsigned char {
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
enum BinOp: unsigned char {
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
enum CastOp: unsigned char {
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
static llvm::Instruction::CastOps getCastOp(CastOp a) {
    switch (a) {
    case Trunc: return llvm::Instruction::Trunc;
    case ZExt: return llvm::Instruction::ZExt;
    case SExt: return llvm::Instruction::SExt;
    case FPToUI: return llvm::Instruction::FPToUI;
    case FPToSI: return llvm::Instruction::FPToSI;
    case UIToFP: return llvm::Instruction::UIToFP;
    case SIToFP: return llvm::Instruction::SIToFP;
    case FPTrunc: return llvm::Instruction::FPTrunc;
    case FPExt: return llvm::Instruction::FPExt;
    case PtrToInt: return llvm::Instruction::PtrToInt;
    case IntToPtr: return llvm::Instruction::IntToPtr;
    case BitCast: return llvm::Instruction::BitCast;
    default: llvm_unreachable("bad cast operator");
    }
}
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
    case SAddP: return "+[pointer]";
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
    case ToBool: return "(bool)";
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
enum FloatKindEnum : uint8_t {
    F_Invalid,
    F_Half,      // https://en.wikipedia.org/wiki/Half-precision_floating-point_format
    F_BFloat,    // https://en.wikipedia.org/wiki/Bfloat16_floating-point_format
    F_Float,     // https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    F_Double,    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format
    F_x87_80,    // 80 bit float (X87)
    F_Quadruple, // https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
    F_PPC128,    // https://gcc.gnu.org/wiki/Ieee128PowerPC
    F_Decimal32,
    F_Decimal64,
    F_Decimal128
};
struct FloatKind {
    static constexpr size_t MAX_KIND = static_cast<size_t>(F_Decimal128) - static_cast<size_t>(F_Half);
    enum FloatKindEnum e;
    constexpr FloatKind(enum FloatKindEnum k) : e{k} { }
    constexpr FloatKind(uint64_t k) : e{static_cast<enum FloatKindEnum>(k)} { }
    constexpr enum FloatKindEnum getKind() { return e; }
    explicit constexpr operator uint64_t() const { return static_cast<uint64_t>(e); }
    constexpr enum FloatKindEnum asEnum() const { return e; }
    constexpr bool isValid() const { return e != F_Invalid && e <= F_PPC128; }
    explicit constexpr operator bool() const { return e != F_Invalid; }
    constexpr bool equals(const enum FloatKindEnum other) const { return e == other; }
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
    bool isIEEE() const { return APFloat::getZero(getFltSemantics()).isIEEE(); }
    APFloat getZero() const { return APFloat::getZero(getFltSemantics()); }
    ConstantFP *getZero(LLVMContext &ctx) const { return ConstantFP::get(ctx, getZero()); }
};
struct IntegerKind {
    uint8_t shift;
    constexpr IntegerKind(uint8_t shift) : shift{shift} { }
    constexpr bool isValid() const { return shift != 1 && shift != 2 && shift <= 7; }
    explicit constexpr operator bool() const { return shift; }
    explicit constexpr operator uint64_t() const { return shift; }
    constexpr uint8_t asLog2() const { return shift; }
    constexpr uint64_t asBits() const { return uint64_t(1) << shift; }
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
    constexpr bool operator==(const IntegerKind &other) const { return asLog2() == other.asLog2(); }
    constexpr static IntegerKind fromLog2(uint8_t Value) { return IntegerKind(Value); }
    static IntegerKind fromBytes(uint64_t Bytes) {
        assert(llvm::isPowerOf2_64(Bytes));
        return fromLog2(llvm::Log2_64(Bytes));
    }
    static IntegerKind fromBits(uint64_t Bits) {
        assert(llvm::isPowerOf2_64(Bits));
        return fromBytes(llvm::Log2_64(Bits));
    }
    APInt getZero() const { return APInt::getZero(asBits()); }
    bool isBool() const { return shift == 0; }
    llvm::Type *toLLVMType(LLVMContext &ctx) const { return llvm::IntegerType::get(ctx, asBits()); }
    ConstantInt *getZero(LLVMContext &ctx) const { return ConstantInt::get(ctx, getZero()); }
};
static constexpr uint64_t build_integer(IntegerKind kind, bool Signed = false), build_float(FloatKind kind);

#include "compiler_builtins.inc"
#include "Arena.cpp"
#include "tokens.inc"
#include "IdentifierTable.h"
#include "tempAllocator.h"
#include "types.inc"
#include "xint128.cpp"
#include "xstring.h"
#include "xvector.h"

constexpr type_tag_t storage_class_specifiers =
                         TYTYPEDEF | TYEXTERN | TYSTATIC | TYTHREAD_LOCAL | TYREGISTER | TYCONSTEXPR | TYAUTO,
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
static const char hexs[] = "0123456789ABCDEF";
static char hexed(unsigned a) { return hexs[a & 0b1111]; }
struct SourceRange {
    location_t start, end;
    SourceRange() : start{0}, end{0} {};
    SourceRange(location_t loc) : start{loc}, end{loc} {
        assert(end <= start && "attempt to construct an invalid SourceRange");
    }
    SourceRange(location_t L, location_t R) : start{L}, end{R} { }
    SourceRange(location_t start, IdentRef Name)
        : start{start}, end{start + static_cast<location_t>(Name->getKeyLength()) - 1} { }
    location_t getStart() const { return start; }
    location_t getEnd() const { return end; }
    location_t getStartLoc() const { return start; }
    location_t getEndLoc() const { return end; }
    bool contains(location_t loc) const { return loc >= start && loc <= end; }
    bool isValid() const { return start != 0 && end != 0; }
    bool isInValid() const { return start == 0 || end == 0; }
    bool isSignle() const { return start == end; }
    bool Chars() const { return start - end + 1; }
    bool operator==(const SourceRange &other) { return start == other.start && end == other.end; }
    void dump() const { llvm::errs() << "SourceRange(" << start << ", " << end << ")"; }
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
    void setInsertLoc(location_t loc) { startLoc = (insertLoc = loc); }
    std::string buildFixItInsertionLine(const ArrayRef<FixItHint> FixItHints) const {
        assert(!FixItHints.empty());
        std::string line(CaretLine.size(), ' ');
        location_t loc = startLoc;
        for (size_t Repeat = line.size(); Repeat--; loc++) {
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
                for (const auto it : ranges) {
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
    unsigned line = 0;
    unsigned col = 0;
    struct LocTree *tree = nullptr;
    SourceLine source_line;
    bool isValid() const { return line != 0; }
    bool operator>(const source_location &rhs) { return line > rhs.line && col > rhs.col; }
    bool operator<(const source_location &rhs) { return line < rhs.line && col < rhs.col; }
    bool operator>=(const source_location &rhs) { return line >= rhs.line && col >= rhs.col; }
    bool operator<=(const source_location &rhs) { return line <= rhs.line && col <= rhs.col; }
};
struct LocTree {
    struct LocTree *parent;
    struct PPMacroDef *macro;
    LocTree(struct LocTree *parent, struct PPMacroDef *def = nullptr) : parent{parent} { this->macro = def; }
    void setParent(LocTree *theParent) { this->parent = theParent; }
    LocTree *getParent() const { return parent; }
};
// A declaration with a type and name
// https://clang.llvm.org/doxygen/classclang_1_1DeclaratorDecl.html
struct Declator {
    IdentRef name;
    CType ty;
    Declator(IdentRef Name = nullptr, CType ty = nullptr) : name{Name}, ty{ty} { }
};
// function parameter-list
struct Param : public Declator {
    Param(IdentRef Name = nullptr, CType ty = nullptr, location_t loc = 0) : Declator{Name, ty}, loc{loc} { }
    Param(const Declator &decl) : Declator(decl), loc{0} { }
    location_t loc;
};
// An enumerator
// https://clang.llvm.org/doxygen/classclang_1_1EnumConstantDecl.html
struct EnumPair {
    IdentRef name;
    uint64_t val;
    EnumPair(IdentRef Name, uint64_t val) : name{Name}, val{val} { }
};
// Any declaration or definition but except function definition
// https://clang.llvm.org/doxygen/classclang_1_1VarDecl.html
struct VarDecl {
    IdentRef name;
    CType ty;
    Expr init; // maybe null
    unsigned idx;
    location_t loc;
};
// enum declaration
// https://clang.llvm.org/doxygen/classclang_1_1EnumDecl.html
struct EnumDecl {
    SmallVector<EnumPair, 6> enums;
};
// A struct/union's field
// https://clang.llvm.org/doxygen/classclang_1_1FieldDecl.html
struct FieldDecl : public Declator {
    FieldDecl(IdentRef Name = nullptr, CType ty = nullptr) : Declator(Name, ty) { }
    FieldDecl(const Declator &decl) : Declator(decl) { }
};
// Record - A struct or a union
// https://clang.llvm.org/doxygen/classclang_1_1RecordDecl.html
struct RecordDecl {
    SmallVector<FieldDecl, 4> fields;
//    ArrayRef<unsigned> LLVMTypeMap;
};
// Tag - A record or enum
// https://clang.llvm.org/doxygen/classclang_1_1TagDecl.html
union TagDecl {
    RecordDecl *struct_decl;
    EnumDecl *enum_decl;
    TagDecl() : struct_decl{nullptr} { }
    TagDecl(RecordDecl *TD) : struct_decl{TD} { }
    TagDecl(EnumDecl *TD) : enum_decl{TD} { }
    const RecordDecl *getRecord() const { return struct_decl; }
    RecordDecl *getRecord() { return struct_decl; }
    const EnumDecl *getEnum() const { return enum_decl; }
    EnumDecl *getEnum() { return enum_decl; }

    bool hasValue() const { return struct_decl != nullptr; }
    operator bool() const { return hasValue(); }
};
struct Designator {
    uint32_t start;
    uint32_t end;
    Designator(): start{uint32_t(-1)}, end{uint32_t(-1)} {}
    Designator(uint32_t index): start{index}, end{uint32_t(-1)} {}
    Designator(uint32_t start, uint32_t end): start{start}, end{end} {}
    bool isInValid() const { return start == uint32_t(-1) && start == uint32_t(-1); }
    bool isValid() const { return start != uint32_t(-1) && end != uint32_t(-1); }
    bool isSingle() const { return (end + 1) == 0; }
    bool isDouble() const { return end != uint32_t(-1); }
    uint32_t getStart() const { return start; }
    uint32_t getEnd() const { return end; }
    uint32_t getRange() const { return start - end; }
};
struct Initializer {
    Expr value; // value
    xvector<Designator> idxs; // The index for the array/struct/union
    Designator idx; // -1 if use idxs
    Initializer(Expr value, Designator idx): value{value}, idxs{xvector<Designator>::get_null()}, idx{idx} {}
    Initializer(Expr value, ArrayRef<Designator> idxs): value{value}, idxs{xvector<Designator>::get(idxs)}, idx{Designator()} {}
    bool isSingle() const { return idxs.p == nullptr; }
    Designator getDesignator() const {
        assert(isSingle());
        return idx;
    }
    ArrayRef<Designator> getDesignators() const {
        return idxs;
    }
    ArrayRef<Designator> getDesignatorsOneOrMore() const {
        return isSingle() ? ArrayRef<Designator>(&idx, 1) : ArrayRef<Designator>(idxs);
    }
};
// Simple 'case' statement with one value
struct SwitchCase {
    location_t loc;
    const APInt *CaseStart;
    label_t label;
    SwitchCase(location_t loc, label_t label, const APInt *CastStart) : loc{loc}, CaseStart{CastStart}, label{label} { }
    static bool equals(const SwitchCase &lhs, const SwitchCase &rhs) { return *lhs.CaseStart == *rhs.CaseStart; }
};
// GNU extension: from (start ... end) or (start ... start + range)
struct GNUSwitchCase : public SwitchCase {
    APInt range;
    GNUSwitchCase(location_t loc, label_t label, const APInt *CastStart, const APInt *CaseEnd)
        : SwitchCase(loc, label, CastStart), range{*CaseEnd - *CastStart} { }
    bool contains(const APInt &C) const { return (C - *CaseStart).ule(range); }
    bool contains(const SwitchCase &G) const { return contains(*G.CaseStart); }
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
struct TranslationUnit {
    Stmt ast; // A HeadStmt
    unsigned max_tags_scope;
    unsigned max_typedef_scope;
};
enum VectorKind: unsigned char {
    /// not a target-specific vector type
    GenericVector,

    /// is AltiVec vector
    AltiVecVector,

    /// is AltiVec 'vector Pixel'
    AltiVecPixel,

    /// is AltiVec 'vector bool ...'
    AltiVecBool,

    /// is ARM Neon vector
    NeonVector,

    /// is ARM Neon polynomial vector
    NeonPolyVector,

    /// is AArch64 SVE fixed-length data vector
    SveFixedLengthDataVector,

    /// is AArch64 SVE fixed-length predicate vector
     SveFixedLengthPredicateVector
};
#include "ctypes.inc"
#include "expressions.inc"
#include "statements.inc"
#include "utf8.cpp"
#include "printer.cpp"
#include "Diagnostic.cpp"
#include "option.cpp"
#include "Graph.cpp"

struct ReplacedExprParen {
    enum ExprKind k = EConstant;
    CType ty;
    struct {
        llvm::Constant *C;
        unsigned id;
        IdentRef varName;
        location_t ReplacedLoc;
        location_t paren_loc[2];
    };
};
struct ReplacedExpr {
    enum ExprKind k = EConstant;
    CType ty;
    struct {
        llvm::Constant *C;
        unsigned id;
        IdentRef varName;
        location_t ReplacedLoc;
    };
};
location_t *OpaqueExpr::getParenLLoc() {
    if (ty->hasTag(TYREPLACED_CONSTANT)) {
        return reinterpret_cast<ReplacedExprParen *>(this)->paren_loc;
    }
    const size_t Size = expr_size_map[k];
    return reinterpret_cast<location_t *>(reinterpret_cast<uintptr_t>(this) + Size);
}
const location_t *OpaqueExpr::getParenLLoc() const { return const_cast<Expr>(this)->getParenLLoc(); }
const location_t *OpaqueExpr::getParenRLoc() const { return getParenLLoc() + 1; }
location_t *OpaqueExpr::getParenRLoc() { return getParenLLoc() + 1; }
location_t OpaqueExpr::getBeginLoc() const {
    if (ty->hasTag(TYPAREN))
        return *getParenLLoc();
    switch (k) {
    case EBlockAddress: return block_loc_begin;
    case ESizeof: return sizeof_loc_begin;
    case EVar: return varLoc;
    case EBin: return lhs->getBeginLoc();
    case EUnary: return opLoc;
    case ECast: return castval->getBeginLoc();
    case ESubscript: return left->getBeginLoc();
    case EInitList: return initStartLoc;
    case EConstant: return constantLoc;
    case ECondition: return cond->getBeginLoc();
    case ECall: return callfunc->getBeginLoc();
    case EString: return stringLoc;
    case EMemberAccess: return obj->getBeginLoc();
    case EArrToAddress: return arr3->getBeginLoc();
    case EPostFix: return poperand->getBeginLoc();
    case EBuiltinCall: return builtin_call_start_loc;
    case EVoid: return voidStartLoc;
    case EConstantArraySubstript: return casLoc;
    case ECallCompilerBuiltinCall: return cbc_start_loc;
    case ECallImplictFunction: return imt_start_loc;
    }
    llvm_unreachable("invalid Expr");
}
location_t OpaqueExpr::getEndLoc() const {
    if (ty->hasTag(TYPAREN))
        return *getParenRLoc();
    switch (k) {
    case EBlockAddress: return block_loc_begin + labelName->getKeyLength() - 1;
    case ESizeof: return sizeof_loc_end;
    case EInitList: return initEndLoc;
    case EConstant: return constantEndLoc;
    case EBin: return rhs->getEndLoc();
    case EUnary: return uoperand->getEndLoc();
    case ESubscript: return right->getEndLoc();
    case EConstantArraySubstript: return casEndLoc;
    case EString: return stringEndLoc;
    case EVoid: return voidexpr->getEndLoc();
    case ECast: return castval->getEndLoc();
    case EVar: return varLoc + varName->getKeyLength() - 1;
    case EMemberAccess: return memberEndLoc;
    case EArrToAddress: return arr3->getEndLoc();
    case ECondition: return cright->getEndLoc();
    case ECall: return callEnd;
    case EPostFix: return postFixEndLoc;
    case EBuiltinCall: return builtin_call_start_loc + builtin_func_name->getKeyLength() - 1;
    case ECallCompilerBuiltinCall: return cbc_end_loc;
    case ECallImplictFunction: return imt_end_loc;
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
static void scope_index_set_unnamed_alloca(unsigned &i) { i |= 1U << 31; }
static bool scope_index_is_unnamed_alloca(unsigned i) { return i & 1U << 31; }
static void scope_index_restore_unnamed_alloca(unsigned &i) { i &= (unsigned(-1) >> 1); }
static bool is_declaration_specifier(Token a) { return a >= Kextern && a <= Kvolatile; }

static bool type_equal(CType a, CType b) {
    assert(a && "type_equal: a is nullptr");
    assert(b && "type_equal: b is nullptr");
    if (a->getKind() != b->getKind())
        return false;
    switch (a->getKind()) {
    case TYBITFIELD: return (a->bitsize == b->bitsize) && type_equal(a->bittype, b->bittype);
    case TYVECTOR: return a->vec_num_elems == b->vec_num_elems && a->vec_kind == b->vec_kind && type_equal(a->vec_ty, b->vec_ty);
    case TYARRAY:
        if (a->hassize != b->hassize)
            return false;
        if (a->hassize && (a->arrsize != b->arrsize))
            return false;
        return type_equal(a->arrtype, b->arrtype);
    case TYPRIM: return a->basic_equals(b);
    case TYPOINTER: return type_equal(a->p, b->p);
    case TYTAG: return a->getTagDecl() == b->getTagDecl();
    case TYBITINT:
        return (a->getBitIntBits() == b->getBitIntBits()) && type_equal(a->getBitIntBaseType(), b->getBitIntBaseType());
    case TYFUNCTION:
        if (!type_equal(a->ret, b->ret) || a->params.size() != b->params.size())
            return false;
        for (unsigned i = 0; i < b->params.size(); ++i)
            if (!type_equal(a->params[i].ty, b->params[i].ty))
                return false;
        return true;
    case TYVLA: return (a->vla_expr == b->vla_expr) && type_equal(a->vla_arraytype, b->vla_arraytype);
    }
    llvm_unreachable("invalid CTypeKind");
}
static bool compatible(CType p, CType expected) {
    // https://en.cppreference.com/w/c/language/type
    if (p->getKind() != expected->getKind())
        return false;
    switch (p->getKind()) {
    case TYVLA: return (p->vla_expr == expected->vla_expr) && compatible(p->vla_arraytype, expected->vla_arraytype);
    case TYPRIM: return p->basic_equals(expected);
    case TYVECTOR:
        return p->vec_num_elems == expected->vec_num_elems && p->vec_kind == expected->vec_kind && type_equal(p->vec_ty, expected->vec_ty);
    case TYFUNCTION:
        if (!compatible(p->ret, expected->ret) || p->params.size() != expected->params.size())
            return false;
        for (unsigned i = 0; i < expected->params.size(); ++i)
            if (!compatible(p->params[i].ty, expected->params[i].ty))
                return false;
        return true;
    case TYPOINTER:
        return p->isNullPtr_t() || expected->isNullPtr_t() || p->p->isVoid() || expected->p->isVoid() ||
               (p->andTags(type_qualifiers) == expected->andTags(type_qualifiers) && compatible(p->p, expected->p));
    case TYBITFIELD: return (p->bitsize == expected->bitsize) && type_equal(p->bittype, expected->bittype);
    case TYARRAY:
        if (p->hassize != expected->hassize)
            return false;
        if (p->hassize && (p->arrsize != expected->arrsize))
            return false;
        return compatible(p->arrtype, expected->arrtype);
    case TYTAG: return p->getTagDecl() == expected->getTagDecl();
    case TYBITINT:
        return (p->getBitIntBits() == expected->getBitIntBits()) &&
               compatible(p->getBitIntBaseType(), expected->getBitIntBaseType());
    }
    llvm_unreachable("invalid CTypeKind");
}
static bool isConstant(const_Expr e) {
    switch (e->k) {
        case EConstant:
        case EString:
        case EConstantArraySubstript:
        case EBlockAddress:
            return true;
        case EInitList:
            for (const Initializer &it: e->inits) {
                if (!isConstant(it.value))
                    return false;
            }
            return true;
        case EBin:
            switch(e->bop) {
                case Complex_CMPLX: return true;
                default: return false;
            }
        default: return false;
    }
}
struct TokenV {
    union {
        const char *str;
        IdentRef s;
        struct {
            Codepoint i;
            enum StringPrefix itag;
        };
        struct LocTree *tree;
    };
    location_t length;
    location_t loc;
    Token tok;
    TokenV(Token tok = TNul, location_t loc = 0) : loc{loc}, tok{tok} {}
    TokenV(struct LocTree *tree): tree{tree}, loc{0}, tok{PPMacroTraceLoc} {}
    TokenV(Token tok, const char *str): str{str}, tok{tok} {}
    bool operator!=(const TokenV &other) const { return !(*this == other); }
    bool operator==(const TokenV &other) const {
        Token x = this->tok;
        Token y = other.tok;
        if (x > TIdentifier)
            x = TIdentifier;
        if (y > TIdentifier)
            y = TIdentifier;
        if (x != y)
            return false;
        switch (x) {
        case TStringLit:
        case PPNumber:
            return StringRef(this->str) == StringRef(other.str);
        case TIdentifier:
            return this->s == other.s;
        default: return true;
        }
    }
    enum StringPrefix getStringPrefix() {
        switch (str[0]) {
        case 'U':
            return Prefix_U;
        case 'L':
            return Prefix_L;
        case 'u':
            return str[1] == '8' ? Prefix_u8 : Prefix_u;
        default: return Prefix_none;
        }
    }
    // Return the start location.
    location_t getLoc() const {
        return loc;
    }
    // Return the end location(maybe equals to start)
    location_t getEndLoc() const {
        return loc + length;
    }
    unsigned getLength() const {
        return length;
    }
    SourceRange getSourceRange() const {
        return SourceRange(getLoc(), getEndLoc());
    }
    Token getToken() const {
        return tok;
    }
    bool is(Token tok) const {
        return this->tok == tok;
    }
    bool isNot(Token tok) const {
        return this->tok != tok;
    }
    bool matches(const ArrayRef<Token> pattern) {
        for (Token tok: pattern) {
            if (this->tok == tok)
                return true;
        }
        return false;
    }
    void setLoc(location_t loc) {
        this->loc = loc;
    }
    void setEndLoc(location_t loc) {
        assert(this->loc <= loc && "invalid endLoc!");
        length = loc - this->loc + 1;
    }
    void setLength(unsigned length) {
        assert(length && "empty length is invalid token");
        this->length = length;
    }
    bool isNull() const {
        return tok == TNul;
    }
    StringRef getStringLiteral() const {
        return StringRef(str, length);
    }
    StringRef getPPNumberLit() const {
        return StringRef(str, length);
    }
    enum StringPrefix getCharPrefix() const { return itag; }
    void dump(raw_ostream &OS = llvm::errs()) const {
        switch (tok) {
        default:
            if (tok >= kw_start) 
                OS << s->getKey();
            else 
                OS << show(tok); 
            break;
        case PPNumber: OS << getPPNumberLit(); break;
        case TStringLit: OS << getStringLiteral(); break;
        case TCharLit:
            if (isprint(i)) {
                OS << '\'' << i << '\'';
            } else {
                (OS << "<0x").write_hex(i) << '>';
            }
            break;
        }
    }
};
enum MacroFlags: unsigned char {
    Macro_Function = 0x1,
    Macro_VarArgs = 0x2
};
struct PPMacroDef {
    IdentRef Name;
    SmallVector<TokenV, 8> tokens;
    SmallVector<IdentRef, 2> params;
    unsigned char flags = 0;
    location_t loc; // where the macro is defined
    PPMacroDef(IdentRef Name, location_t loc = 0): Name{Name}, tokens{}, params{}, flags{}, loc{loc} {}
    location_t getMacroLoc() const {
        return loc;
    }
    ArrayRef<TokenV> getTokens() const {
        return tokens;
    }
    ArrayRef<IdentRef> getParams() const {
        assert(isFunction() && "Only function macro has parameters");
        return params;
    }
    IdentRef getName() const {
        return Name;
    }
    static bool tokensEq(const ArrayRef<TokenV> &a, const ArrayRef<TokenV> &b) {
        for (size_t i = 0;i < a.size();++i) 
            if (a[i] != b[i]) return false;
        return true;
    }
    bool equals(const PPMacroDef &other) const {
        return (flags == other.flags) && tokensEq(tokens, other.tokens);
    }
    bool isObj() const {
        return !(flags & Macro_Function);
    }
    bool isFunction() const {
        return flags & Macro_Function;
    }
    bool isVararg() const {
        return flags & Macro_VarArgs;
    }
    void setObjMacro() {
        flags &= ~Macro_Function;
    }
    void setFunctionMacro() {
        flags |= Macro_Function;
    }
    void setNoVarArgs() {
        flags &= ~Macro_VarArgs;
    }
    void setVarArgs() {
        flags |= Macro_VarArgs;
    }
};
static unsigned intRank(const_CType ty) { return ty->getIntegerKind().asLog2(); }
static unsigned floatRank(const_CType ty) { return ty->getFloatKind().getBitWidth(); }
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
enum GetBuiltinTypeError: unsigned char {
    /// No error
    GE_None,

    /// Missing a type
    GE_Missing_type,

    /// Missing a type from <stdio.h>
    GE_Missing_stdio,

    /// Missing a type from <setjmp.h>
    GE_Missing_setjmp,

    /// Missing a type from <ucontext.h>
    GE_Missing_ucontext
};

#ifdef XCC_JIT
#include "JIT.cpp"
#endif
#include "LLVMDiagnosticHandler.cpp"
#include "Scope.cpp"
#include "xcc_context.cpp"
#include "SourceMgr.cpp"
#include "TextDiagnosticPrinter.cpp"
#include "LLVMTypeConsumer.cpp"
#include "codegen.cpp"
#include "codegen_builtin.cpp"
#include "lexer.cpp"
#include "output.cpp"
#include "StmtVisitor.cpp"
#include "parser.cpp"
#include "lexerDefinition.cpp"
#include "CompilerInstance.cpp"
#include "interpreter/interpreter.cpp"
#ifdef XCC_TOOLCHAIN
#include "toolchains/ToolChain.cpp"
#endif
} // namespace xcc
