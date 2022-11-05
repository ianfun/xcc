#include "config.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/Twine.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/Support/ConvertUTF.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Errno.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Compiler.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

// va_list, va_arg
#include <cstdint>
#include <cstdarg>

// open/read/close
#include <fcntl.h>
#include <unistd.h>
#include <cstdint> // intmax_t, uintmax_t
#include <cmath> // pow
#include <ctime>
#include <cstdio>
#include <algorithm> // std::max
#include <deque>

#if !CC_NO_RAEADLINE
#include <readline/readline.h> // readline
#include <readline/history.h> // add_history
#endif

#include <cctype>

#if __cplusplus > 201402L
#include <string_view>
#endif

#define dbgprint(msg, ...) ((void)logtime(), (void)fprintf(stderr, msg, ##__VA_ARGS__))

namespace xcc {
static void logtime() {
    time_t now = time(nullptr);
    fprintf(stderr, "\33[01;33m[debug]: \33[01;34m%s\33[0m", ctime(&now));
}
#if WINDOWS
    HANDLE 
      hStdin = GetStdHandle(STD_INPUT_HANDLE),
      hStdout = GetStdHandle(STD_OUTPUT_HANDLE),
      hStderr = GetStdHandle(STD_ERROR_HANDLE);
#endif

using llvm::APInt;
using llvm::APFloat;
using llvm::GlobalValue;
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::sys::ProcessInfo;
using llvm::isa;
using llvm::isa_and_nonnull;
using llvm::isa_and_present;
using llvm::cast;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::dyn_cast_if_present;
using llvm::cast_or_null;
using llvm::cast_if_present;
using llvm::DenseMap;
using llvm::ArrayRef;
using llvm::StringRef;
using llvm::SmallVector;
using llvm::SmallString;
using llvm::raw_ostream;
using llvm::raw_fd_ostream;
using llvm::Twine;
using llvm::SmallVectorImpl;

enum PostFixOp {
    PostfixIncrement=1, PostfixDecrement
};
enum UnaryOp {
    UNeg=1, SNeg, FNeg,
    Not, AddressOf,
    Dereference, LogicalNot
};
enum BinOp {
    // Arithmetic operators
    UAdd=1, SAdd, FAdd, 
    USub, SSub, FSub,
    UMul, SMul, FMul,
    UDiv, SDiv, FDiv, 
    URem, SRem, FRem,
    Shr, AShr, Shl,

    And, Xor, Or,

    LogicalAnd, LogicalOr, 
    Assign,
    SAddP,
    PtrDiff,
    Comma,

    // Atomic ops
    AtomicrmwAdd,
    AtomicrmwSub,
    AtomicrmwAnd,
    AtomicrmwOr,
    AtomicrmwXor,

    // compare operators
    EQ, NE, 
    UGT, UGE, ULT, ULE, 
    SGT, SGE, SLT, SLE,
    // ordered float compare
    FEQ, FNE, 
    FGT, FGE, FLT, FLE
};
enum CastOp {
    Trunc=1,
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
#define BINOP3(two, r) case U##two: case S##two: case F##two: return r
    switch (op) {
        BINOP3(Add, "+");
        BINOP3(Sub, "-");
        BINOP3(Mul, "*");
        BINOP3(Div, "/");
        BINOP3(Rem, "%");
        case Shr: case AShr: return ">>";
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
        case AtomicrmwAdd: return "+";
        case AtomicrmwSub: return "-";
        case AtomicrmwAnd: return "&";
        case AtomicrmwOr: return "|";
        case AtomicrmwXor: return "^";
        case EQ: case FEQ: return "==";
        case NE: case FNE: return "!=";
#define CMPOP3(c, r) case U##c: case S##c: case F##c: return r
        CMPOP3(GT, ">");
        CMPOP3(GE, ">=");
        CMPOP3(LT, "<");
        CMPOP3(LE, "<=");
    }
    llvm_unreachable("bad binary operator");
}
static const char *show(enum UnaryOp o){
    switch(o){
        case UNeg: case SNeg: case FNeg:
            return "-";
        case Not:
            return "!";
        case AddressOf:
            return "&";
        case Dereference:
            return "";
        case LogicalNot:
            return "~";
        default:
            return "(unknown unary operator)";
    }
}
static const char* show(enum PostFixOp o){
    switch(o){
        case PostfixIncrement: return "++";
        case PostfixDecrement: return "--";
        default: return "(unknown postfix operator)";
    }
}
#include "xint128.cpp"
#include "Arena.cpp"
#include "tempAllocator.h"
#include "tokens.inc"
#include "types.inc"
#include "IdentifierTable.h"
#include "xstring.h"
#include "xvector.h"

#define kw_start Kextern
#define kw_end K_Generic
typedef unsigned label_t;
#define INVALID_LABEL ((label_t)-1)
typedef uint16_t fileid_t;
typedef uint16_t column_t;
typedef uint32_t line_t;
struct Location {
  line_t line;
  column_t col; // at most 65535 are supported
  // xxx: use bitfields? for example: line: 40 bits, col: 24 bits
  fileid_t id;
  // construct an invalid Location
  static Location make_invalid() {
    return Location {.line = 0, .col = 0, .id = 0};
  }
  bool isValid() const { return line != 0; }
};

enum Linker {
    LLD, GCCLD
};

typedef uint32_t Codepoint;

typedef struct OpaqueStmt *Stmt;
typedef struct OpaqueExpr *Expr;
typedef struct OpaqueCType *CType;

struct Declator {
    IdentRef name;
    CType ty;
    Declator(IdentRef Name = nullptr, CType ty = nullptr): name{Name}, ty{ty} {}
};
struct Param: public Declator {
    Param(IdentRef Name = nullptr, CType ty = nullptr): Declator {Name, ty} {}
    Param(const Declator &decl): Declator(decl) {}
};
struct EnumPair {
    IdentRef name;
    uint64_t val;
};
struct VarDecl
{
    IdentRef name;
    CType ty;
    Expr init; // maybe null
    size_t idx;
};
enum ReachableKind {
    Reachable, MayReachable, Unreachable
};
constexpr uint8_t 
  LBL_UNDEFINED = 0,
  LBL_FORWARD = 1,
  LBL_DECLARED = 2,
  LBL_OK = 4;

enum TypeIndex {
    voidty,
    i1ty,
    i8ty, u8ty,
    i16ty, u16ty,
    i32ty, u32ty,
    i64ty, u64ty,
    i128ty, u128ty,
    floatty, doublety, fp128ty,
    ptrty,
    TypeIndexHigh
};
enum NoSignTypeIndex {
    xvoid,
    x1,
    x8,
    x16,
    x32,
    x64,
    x128,
    xfloat,
    xdouble,
    xfp128,
    xptr,
    NoSignTypeIndexHigh
};
static TypeIndex getTypeIndex(uint32_t tags) {
    if (tags & TYVOID)
        return voidty;
    if (tags & TYBOOL)
        return i1ty;
    if (tags & TYINT8)
        return i8ty;
    if (tags & TYUINT8)
        return u8ty;
    if (tags & TYINT16)
        return i16ty;
    if (tags & TYUINT16)
        return u16ty;
    if (tags & TYINT32)
        return i32ty;
    if (tags & TYUINT32)
        return u32ty;
    if (tags & TYINT64)
        return i64ty;
    if (tags & TYUINT64)
        return u64ty;
    if (tags & TYINT128)
        return i128ty;
    if (tags & TYUINT128)
        return u128ty;
    if (tags & TYDOUBLE)
        return doublety;
    if (tags & TYFLOAT)
        return floatty;
    if (tags & TYF128)
        return fp128ty;
    llvm_unreachable("bad type provided for getTypeIndex()");
}
static NoSignTypeIndex getNoSignTypeIndex(uint32_t tags) {
    if (tags & TYVOID)
        return xvoid;
    if (tags & TYBOOL)
        return x1;
    if (tags & (TYINT8 | TYUINT8))
        return x8;
    if (tags & (TYINT16 | TYUINT16))
        return x16;
    if (tags & (TYINT32 | TYUINT32))
        return x32;
    if (tags & (TYINT64 | TYUINT64))
        return x64;
    if (tags & (TYINT128 | TYUINT128))
        return x128;
    if (tags & TYFLOAT)
        return xfloat;
    if (tags & TYDOUBLE)
        return xdouble;
    if (tags & TYF128)
        return xfp128;
    llvm_unreachable("bad type provided for getNoSignTypeIndex()");
}
static enum BinOp getAtomicrmwOp(Token tok) {
    switch (tok) {
        default:           return static_cast<BinOp>(0);
        case TAsignAdd:    return AtomicrmwAdd;
        case TAsignSub:    return AtomicrmwSub;
        case TAsignBitOr:  return AtomicrmwOr;
        case TAsignBitAnd: return AtomicrmwAnd;
        case TAsignBitXor: return AtomicrmwXor;
    }
}
#include "expressions.inc"
#include "ctypes.inc"
#include "statements.inc"
#include "printer.cpp"

static const char hexs[] = "0123456789ABCDEF";

static char hexed(unsigned a) {
    return hexs[a & 0b1111];
} 
enum PPFlags: uint8_t  {
  PFNormal=1, PFPP=2
};
// integer tags
enum ITag: uint8_t  {
      Iint,   Ilong,   Iulong,  Ilonglong, Iulonglong, Iuint
};
static bool isCSkip(char c){
  // space, tab, new line, form feed are translate into ' '
  return c == ' ' || c == '\t' || c == '\f' || c == '\v';
}
static bool is_declaration_specifier(Token a) {
    return a >= Kextern && a <= Kvolatile;
}
static const char months[12][4] = {
    "Jan", // January
    "Feb", // February
    "Mar", // March
    "Apr", // April
    "May", // May
    "Jun", // June
    "Jul", // July
    "Aug", // August
    "Sep", // September
    "Oct", // October
    "Nov", // November
    "Dec"  // December
};
static bool compatible(const CType p, const CType expected) {
    // https://en.cppreference.com/w/c/language/type
    if (p->k != expected->k)
        return false;
    else{
        switch(p->k){
        case TYPRIM:
            return (p->tags & ty_prim) == (expected->tags & ty_prim);
        case TYFUNCTION:
            if (!compatible(p->ret, expected->ret))
                return false;
            for (unsigned i=0;i<std::min(p->params.size(), expected->params.size());++i){
                if (p->params[i].ty == nullptr)
                    break;
                if (expected->params[i].ty == nullptr)
                    break;
                if (!compatible(p->params[i].ty, expected->params[i].ty))
                    return false;
            }
            return true;
        case TYSTRUCT:case TYENUM:case TYUNION:
            return (uintptr_t)p == (uintptr_t)expected;
        case TYPOINTER:{
            constexpr auto mask = TYRESTRICT | TYCONST | TYVOLATILE;
            return (p->p->tags & TYVOID) || (expected->p->tags & TYVOID) || (((p->tags & mask) == (expected->tags & mask)) && compatible(p->p, expected->p));
        }
        case TYINCOMPLETE:
            return p->tag == expected->tag && p->name == expected->name;
        case TYBITFIELD:
            llvm_unreachable("");
        case TYARRAY:
            return compatible(p->arrtype, expected->arrtype);
        }
    }
    llvm_unreachable("");
}
enum FTag: uint8_t  {
    Fdobule, Ffloat
};

// TokenV: A Token with a value, and a macro is a sequence of TokenVs

enum TokenVKind: uint8_t {
    ATokenVBase, ATokenIdent, ATokenVNumLit, ATokenVStrLit, ATokenVChar
};

struct TokenV {
    enum TokenVKind k = ATokenVBase; // 8 bits
    Token tok; // 8 bits
    union {
        // containing the encoding, .e.g: 8, 16, 32
        struct {
            xstring str; // 64 bits
            uint8_t enc;
        };
        IdentRef s; // 64 bits
        struct {
            unsigned char i; // 8 bits
            enum ITag itag; // 8 bits
        };
    };
    TokenV(): k{ATokenVBase}, tok{TNul} {};
    TokenV(enum TokenVKind k, Token tok): k{k}, tok{tok} {}
    TokenV(Token tok): k{ATokenVBase}, tok{tok} {}
    void dump(raw_ostream &OS) {
        switch (k) {
            case ATokenVBase: OS << show(tok); break;
            case ATokenIdent: OS.write_escaped(s->getKey()); OS << '(' << show(tok) << '-' << show(s->second.getToken()) << ')'; break;
            case ATokenVNumLit: OS << str.str(); break;
            case ATokenVStrLit: OS.write_escaped(str.str()); break;
            case ATokenVChar: if (isprint(i)) { OS << '\'' << i << '\''; } else { (OS << "<0x").write_hex(i) << '>'; } break;
            default: llvm_unreachable("bad TokenV kind");
        }
    }
};
enum PPMacroKind: uint8_t { 
    MOBJ, MFUNC
};

struct PPMacro {
    static bool tokensEq(ArrayRef<TokenV> a, ArrayRef<TokenV> b) {
        if (a.size() != b.size()) 
            return false;
        for(unsigned i=0;i<a.size();++i){
            TokenV x = a[i];
            TokenV y = b[i];
            if (x.tok > TIdentifier) x.tok = TIdentifier;
            if (y.tok > TIdentifier) y.tok = TIdentifier;
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
                default:
                    break;
            }
        }
        return true;
    }
    enum PPMacroKind k = MOBJ; // 1 byte
    bool ivarargs = false; // 1 byte
    llvm::SmallVector<TokenV, 20> tokens{};
    llvm::SmallVector<IdentRef, 0> params;
    bool equals(PPMacro& other) {
        return (
            (k == other.k) &&
            (k == MFUNC ? (ivarargs == other.ivarargs) : true) &&
            tokensEq(tokens, other.tokens)
        );
    }
};

static unsigned intRank(uint32_t a){
    if (a & TYBOOL)
        return 1;
    if (a & (TYINT8 | TYUINT8))
        return 2;
    if (a & (TYINT16 | TYUINT16))
        return 3;
    if (a & (TYINT32 | TYUINT32))
        return 4;
    if (a & (TYINT64 | TYUINT64))
        return 5;
    return 6;
}
#include "Scope.cpp"
#include "option.cpp"
#include "State.cpp"
#include "Diagnostic.cpp"
#include "console.cpp"
#include "SourceMgr.cpp"
#include "TextDiagnosticPrinter.cpp"
#include "codegen.cpp"
#include "utf8.cpp"
#include "lexer.h"
#include "parser.cpp"
#include "lexer.cpp"
#include "linker.cpp"
#include "output.cpp"
#include "LLVMDiagnosticHandler.cpp"
#include "JIT.cpp"

}