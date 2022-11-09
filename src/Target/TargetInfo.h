// mapping clang call-conv to llvm
// see clang/lib/CodeGen/CGCall.cpp
#define SwiftAsync SwiftTail
#define X86_64SysV X86_64_SysV
#define AAPCS ARM_AAPCS
#define AAPCS_VFP ARM_AAPCS_VFP
#define IntelOclBicc Intel_OCL_BI
#define X86Pascal  C
#define X86VectorCall X86_VectorCall
#define AArch64VectorCall AArch64_VectorCall
#define SpirFunction SPIR_FUNC
#define OpenCLKernel AMDGPU_KERNEL
#define PreserveMost PreserveMost
#define PreserveAll PreserveAll

#include "MacroBuilder.h"
#include "Visibility.h"
#include "LangOptions.h"
#include "TargetOptions.h"
#include "CUDA.h"

class TargetInfo;

enum LanguageID {
  GNU_LANG = 0x1,            // builtin requires GNU mode.
  C_LANG = 0x2,              // builtin for c only.
  CXX_LANG = 0x4,            // builtin for cplusplus only.
  OBJC_LANG = 0x8,           // builtin for objective-c and objective-c++
  MS_LANG = 0x10,            // builtin requires MS mode.
  OMP_LANG = 0x20,           // builtin requires OpenMP.
  CUDA_LANG = 0x40,          // builtin requires CUDA.
  COR_LANG = 0x80,           // builtin requires use of 'fcoroutine-ts' option.
  OCL_GAS = 0x100,           // builtin requires OpenCL generic address space.
  OCL_PIPE = 0x200,          // builtin requires OpenCL pipe.
  OCL_DSE = 0x400,           // builtin requires OpenCL device side enqueue.
  ALL_OCL_LANGUAGES = 0x800, // builtin for OCL languages.
  ALL_LANGUAGES = C_LANG | CXX_LANG | OBJC_LANG, // builtin for all languages.
  ALL_GNU_LANGUAGES = ALL_LANGUAGES | GNU_LANG,  // builtin requires GNU mode.
  ALL_MS_LANGUAGES = ALL_LANGUAGES | MS_LANG     // builtin requires MS mode.
};

namespace Builtin {
enum ID {
  NotBuiltin  = 0,      // This is not a builtin function.
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Builtins.def"
  FirstTSBuiltin
};

struct Info {
  const char *Name, *Type, *Attributes, *HeaderName;
  LanguageID Langs;
  const char *Features;
};
   /// Flags to identify the types for overloaded Neon builtins.
   ///
   /// These must be kept in sync with the flags in utils/TableGen/NeonEmitter.h.
   class NeonTypeFlags {
     enum {
       EltTypeMask = 0xf,
       UnsignedFlag = 0x10,
       QuadFlag = 0x20
     };
     uint32_t Flags;
  
   public:
     enum EltType {
       Int8,
       Int16,
       Int32,
       Int64,
       Poly8,
       Poly16,
       Poly64,
       Poly128,
       Float16,
       Float32,
       Float64,
       BFloat16
     };
  
     NeonTypeFlags(unsigned F) : Flags(F) {}
     NeonTypeFlags(EltType ET, bool IsUnsigned, bool IsQuad) : Flags(ET) {
       if (IsUnsigned)
         Flags |= UnsignedFlag;
       if (IsQuad)
         Flags |= QuadFlag;
     }
  
     EltType getEltType() const { return (EltType)(Flags & EltTypeMask); }
     bool isPoly() const {
       EltType ET = getEltType();
       return ET == Poly8 || ET == Poly16 || ET == Poly64;
     }
     bool isUnsigned() const { return (Flags & UnsignedFlag) != 0; }
     bool isQuad() const { return (Flags & QuadFlag) != 0; }
   };
  
   /// Flags to identify the types for overloaded SVE builtins.
   class SVETypeFlags {
     uint64_t Flags;
     unsigned EltTypeShift;
     unsigned MemEltTypeShift;
     unsigned MergeTypeShift;
     unsigned SplatOperandMaskShift;
  
   public:
 #define LLVM_GET_SVE_TYPEFLAGS
 #include "arm_sve_typeflags.inc"
 #undef LLVM_GET_SVE_TYPEFLAGS
  
     enum EltType {
 #define LLVM_GET_SVE_ELTTYPES
 #include "arm_sve_typeflags.inc"
 #undef LLVM_GET_SVE_ELTTYPES
     };
  
     enum MemEltType {
 #define LLVM_GET_SVE_MEMELTTYPES
 #include "arm_sve_typeflags.inc"
 #undef LLVM_GET_SVE_MEMELTTYPES
     };
  
     enum MergeType {
 #define LLVM_GET_SVE_MERGETYPES
 #include "arm_sve_typeflags.inc"
 #undef LLVM_GET_SVE_MERGETYPES
     };
  
     enum ImmCheckType {
 #define LLVM_GET_SVE_IMMCHECKTYPES
 #include "arm_sve_typeflags.inc"
 #undef LLVM_GET_SVE_IMMCHECKTYPES
     };
  
     SVETypeFlags(uint64_t F) : Flags(F) {
       EltTypeShift = llvm::countTrailingZeros(EltTypeMask);
       MemEltTypeShift = llvm::countTrailingZeros(MemEltTypeMask);
       MergeTypeShift = llvm::countTrailingZeros(MergeTypeMask);
       SplatOperandMaskShift = llvm::countTrailingZeros(SplatOperandMask);
     }
  
     EltType getEltType() const {
       return (EltType)((Flags & EltTypeMask) >> EltTypeShift);
     }
  
     MemEltType getMemEltType() const {
       return (MemEltType)((Flags & MemEltTypeMask) >> MemEltTypeShift);
     }
  
     MergeType getMergeType() const {
       return (MergeType)((Flags & MergeTypeMask) >> MergeTypeShift);
     }
  
     unsigned getSplatOperand() const {
       return ((Flags & SplatOperandMask) >> SplatOperandMaskShift) - 1;
     }
  
     bool hasSplatOperand() const {
       return Flags & SplatOperandMask;
     }
  
     bool isLoad() const { return Flags & IsLoad; }
     bool isStore() const { return Flags & IsStore; }
     bool isGatherLoad() const { return Flags & IsGatherLoad; }
     bool isScatterStore() const { return Flags & IsScatterStore; }
     bool isStructLoad() const { return Flags & IsStructLoad; }
     bool isStructStore() const { return Flags & IsStructStore; }
     bool isZExtReturn() const { return Flags & IsZExtReturn; }
     bool isByteIndexed() const { return Flags & IsByteIndexed; }
     bool isOverloadNone() const { return Flags & IsOverloadNone; }
     bool isOverloadWhile() const { return Flags & IsOverloadWhile; }
     bool isOverloadDefault() const { return !(Flags & OverloadKindMask); }
     bool isOverloadWhileRW() const { return Flags & IsOverloadWhileRW; }
     bool isOverloadCvt() const { return Flags & IsOverloadCvt; }
     bool isPrefetch() const { return Flags & IsPrefetch; }
     bool isReverseCompare() const { return Flags & ReverseCompare; }
     bool isAppendSVALL() const { return Flags & IsAppendSVALL; }
     bool isInsertOp1SVALL() const { return Flags & IsInsertOp1SVALL; }
     bool isGatherPrefetch() const { return Flags & IsGatherPrefetch; }
     bool isReverseUSDOT() const { return Flags & ReverseUSDOT; }
     bool isUndef() const { return Flags & IsUndef; }
     bool isTupleCreate() const { return Flags & IsTupleCreate; }
     bool isTupleGet() const { return Flags & IsTupleGet; }
     bool isTupleSet() const { return Flags & IsTupleSet; }
  
     uint64_t getBits() const { return Flags; }
     bool isFlagSet(uint64_t Flag) const { return Flags & Flag; }
   };


/// Holds information about both target-independent and
/// target-specific builtins, allowing easy queries by clients.
///
/// Builtins from an optional auxiliary target are stored in
/// AuxTSRecords. Their IDs are shifted up by TSRecords.size() and need to
/// be translated back with getAuxBuiltinID() before use.
class Context {
  llvm::ArrayRef<Info> TSRecords;
  llvm::ArrayRef<Info> AuxTSRecords;

public:
  Context() {}

  /// Perform target-specific initialization
  /// \param AuxTarget Target info to incorporate builtins from. May be nullptr.
  void InitializeTarget(const TargetInfo &Target, const TargetInfo *AuxTarget);

  /// Mark the identifiers for all the builtins with their
  /// appropriate builtin ID # and mark any non-portable builtin identifiers as
  /// such.
  void initializeBuiltins(/*IdentifierTable &Table, const LangOptions& LangOpts*/);

  /// Return the identifier name for the specified builtin,
  /// e.g. "__builtin_abs".
  const char *getName(unsigned ID) const {
    return getRecord(ID).Name;
  }

  /// Get the type descriptor string for the specified builtin.
  const char *getTypeString(unsigned ID) const {
    return getRecord(ID).Type;
  }

  /// Return true if this function is a target-specific builtin.
  bool isTSBuiltin(unsigned ID) const {
    return ID >= Builtin::FirstTSBuiltin;
  }

  /// Return true if this function has no side effects.
  bool isPure(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'U') != nullptr;
  }

  /// Return true if this function has no side effects and doesn't
  /// read memory.
  bool isConst(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'c') != nullptr;
  }

  /// Return true if we know this builtin never throws an exception.
  bool isNoThrow(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'n') != nullptr;
  }

  /// Return true if we know this builtin never returns.
  bool isNoReturn(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'r') != nullptr;
  }

  /// Return true if we know this builtin can return twice.
  bool isReturnsTwice(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'j') != nullptr;
  }

  /// Returns true if this builtin does not perform the side-effects
  /// of its arguments.
  bool isUnevaluated(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'u') != nullptr;
  }

  /// Return true if this is a builtin for a libc/libm function,
  /// with a "__builtin_" prefix (e.g. __builtin_abs).
  bool isLibFunction(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'F') != nullptr;
  }

  /// Determines whether this builtin is a predefined libc/libm
  /// function, such as "malloc", where we know the signature a
  /// priori.
  bool isPredefinedLibFunction(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'f') != nullptr;
  }

  /// Returns true if this builtin requires appropriate header in other
  /// compilers. In Clang it will work even without including it, but we can emit
  /// a warning about missing header.
  bool isHeaderDependentFunction(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'h') != nullptr;
  }

  /// Determines whether this builtin is a predefined compiler-rt/libgcc
  /// function, such as "__clear_cache", where we know the signature a
  /// priori.
  bool isPredefinedRuntimeFunction(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'i') != nullptr;
  }

  /// Determines whether this builtin has custom typechecking.
  bool hasCustomTypechecking(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 't') != nullptr;
  }

  /// Determines whether a declaration of this builtin should be recognized
  /// even if the type doesn't match the specified signature.
  bool allowTypeMismatch(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'T') != nullptr ||
           hasCustomTypechecking(ID);
  }

  /// Determines whether this builtin has a result or any arguments which
  /// are pointer types.
  bool hasPtrArgsOrResult(unsigned ID) const {
    return strchr(getRecord(ID).Type, '*') != nullptr;
  }

  /// Return true if this builtin has a result or any arguments which are
  /// reference types.
  bool hasReferenceArgsOrResult(unsigned ID) const {
    return strchr(getRecord(ID).Type, '&') != nullptr ||
           strchr(getRecord(ID).Type, 'A') != nullptr;
  }

  /// If this is a library function that comes from a specific
  /// header, retrieve that header name.
  const char *getHeaderName(unsigned ID) const {
    return getRecord(ID).HeaderName;
  }

  /// Determine whether this builtin is like printf in its
  /// formatting rules and, if so, set the index to the format string
  /// argument and whether this function as a va_list argument.
  bool isPrintfLike(unsigned ID, unsigned &FormatIdx, bool &HasVAListArg);

  /// Determine whether this builtin is like scanf in its
  /// formatting rules and, if so, set the index to the format string
  /// argument and whether this function as a va_list argument.
  bool isScanfLike(unsigned ID, unsigned &FormatIdx, bool &HasVAListArg);

  /// Determine whether this builtin has callback behavior (see
  /// llvm::AbstractCallSites for details). If so, add the index to the
  /// callback callee argument and the callback payload arguments.
  bool performsCallback(unsigned ID,
                        llvm::SmallVectorImpl<int> &Encoding) const;

  /// Return true if this function has no side effects and doesn't
  /// read memory, except for possibly errno.
  ///
  /// Such functions can be const when the MathErrno lang option is disabled.
  bool isConstWithoutErrno(unsigned ID) const {
    return strchr(getRecord(ID).Attributes, 'e') != nullptr;
  }

  const char *getRequiredFeatures(unsigned ID) const {
    return getRecord(ID).Features;
  }

  unsigned getRequiredVectorWidth(unsigned ID) const;

  /// Return true if builtin ID belongs to AuxTarget.
  bool isAuxBuiltinID(unsigned ID) const {
    return ID >= (Builtin::FirstTSBuiltin + TSRecords.size());
  }

  /// Return real builtin ID (i.e. ID it would have during compilation
  /// for AuxTarget).
  unsigned getAuxBuiltinID(unsigned ID) const { return ID - TSRecords.size(); }

  /// Returns true if this is a libc/libm function without the '__builtin_'
  /// prefix.
  static bool isBuiltinFunc(llvm::StringRef Name);

  /// Returns true if this is a builtin that can be redeclared.  Returns true
  /// for non-builtins.
  bool canBeRedeclared(unsigned ID) const;

private:
  const Info &getRecord(unsigned ID) const;

  /// Is this builtin supported according to the given language options?
  bool builtinIsSupported(const Builtin::Info &BuiltinInfo,
                          const LangOptions &LangOpts);

  /// Helper function for isPrintfLike and isScanfLike.
  bool isLike(unsigned ID, unsigned &FormatIdx, bool &HasVAListArg,
              const char *Fmt) const;
};

}
   namespace NEON {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE) BI##ID,
 #include "BuiltinsNEON.def"
     FirstTSBuiltin
   };
   }
  
   /// ARM builtins
   namespace ARM {
     enum {
       LastTIBuiltin = Builtin::FirstTSBuiltin-1,
       LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsARM.def"
       LastTSBuiltin
     };
   }
  
   namespace SVE {
   enum {
     LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsSVE.def"
     FirstTSBuiltin,
   };
   }
  
   /// AArch64 builtins
   namespace AArch64 {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
     LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
     FirstSVEBuiltin = NEON::FirstTSBuiltin,
     LastSVEBuiltin = SVE::FirstTSBuiltin - 1,
   #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
   #include "BuiltinsAArch64.def"
     LastTSBuiltin
   };
   }
  
   /// BPF builtins
   namespace BPF {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
   #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
   #include "BuiltinsBPF.def"
     LastTSBuiltin
   };
   }
  
   /// PPC builtins
   namespace PPC {
     enum {
         LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsPPC.def"
         LastTSBuiltin
     };
   }
  
   /// NVPTX builtins
   namespace NVPTX {
     enum {
         LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsNVPTX.def"
         LastTSBuiltin
     };
   }
  
   /// AMDGPU builtins
   namespace AMDGPU {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
   #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
   #include "BuiltinsAMDGPU.def"
     LastTSBuiltin
   };
   }
  
   /// X86 builtins
   namespace X86 {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsX86.def"
     FirstX86_64Builtin,
     LastX86CommonBuiltin = FirstX86_64Builtin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsX86_64.def"
     LastTSBuiltin
   };
   }
     /// Hexagon builtins
   namespace Hexagon {
     enum {
         LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsHexagon.def"
         LastTSBuiltin
     };
   }
  
   /// MIPS builtins
   namespace Mips {
     enum {
         LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsMips.def"
         LastTSBuiltin
     };
   }
  
   /// XCore builtins
   namespace XCore {
     enum {
         LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsXCore.def"
         LastTSBuiltin
     };
   }
  
   /// SystemZ builtins
   namespace SystemZ {
     enum {
         LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsSystemZ.def"
         LastTSBuiltin
     };
   }
  
   /// WebAssembly builtins
   namespace WebAssembly {
     enum {
       LastTIBuiltin = Builtin::FirstTSBuiltin-1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsWebAssembly.def"
       LastTSBuiltin
     };
   }
   /// VE builtins
   namespace VE {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsVE.def"
     LastTSBuiltin
   };
   }
  
   namespace RISCVVector {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsRISCVVector.def"
     FirstTSBuiltin,
   };
   }
  
   /// RISCV builtins
   namespace RISCV {
   enum {
     LastTIBuiltin = Builtin::FirstTSBuiltin - 1,
     FirstRVVBuiltin = Builtin::FirstTSBuiltin,
     LastRVVBuiltin = RISCVVector::FirstTSBuiltin - 1,
 #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
 #include "BuiltinsRISCV.def"
     LastTSBuiltin
   };
   } // namespace RISCV
static constexpr uint64_t LargestBuiltinID = std::max<uint64_t>(
       {ARM::LastTSBuiltin, AArch64::LastTSBuiltin, BPF::LastTSBuiltin,
        PPC::LastTSBuiltin, NVPTX::LastTSBuiltin, AMDGPU::LastTSBuiltin,
        X86::LastTSBuiltin, VE::LastTSBuiltin, RISCV::LastTSBuiltin,
        Hexagon::LastTSBuiltin, Mips::LastTSBuiltin, XCore::LastTSBuiltin,
        SystemZ::LastTSBuiltin, WebAssembly::LastTSBuiltin});
 enum class LangAS : unsigned {
   // The default value 0 is the value used in QualType for the situation
   // where there is no address space qualifier.
   Default = 0,
  
   // OpenCL specific address spaces.
   // In OpenCL each l-value must have certain non-default address space, each
   // r-value must have no address space (i.e. the default address space). The
   // pointee of a pointer must have non-default address space.
   opencl_global,
   opencl_local,
   opencl_constant,
   opencl_private,
   opencl_generic,
   opencl_global_device,
   opencl_global_host,
  
   // CUDA specific address spaces.
   cuda_device,
   cuda_constant,
   cuda_shared,
  
   // SYCL specific address spaces.
   sycl_global,
   sycl_global_device,
   sycl_global_host,
   sycl_local,
   sycl_private,
  
   // Pointer size and extension address spaces.
   ptr32_sptr,
   ptr32_uptr,
   ptr64,
  
   // HLSL specific address spaces.
   hlsl_groupshared,
  
   // This denotes the count of language-specific address spaces and also
   // the offset added to the target-specific address spaces, which are usually
   // specified by address space attributes __attribute__(address_space(n))).
   FirstTargetAddressSpace
 };
  
 /// The type of a lookup table which maps from language-specific address spaces
 /// to target-specific ones.
 using LangASMap = unsigned[(unsigned)LangAS::FirstTargetAddressSpace];
  
 /// \return whether \p AS is a target-specific address space rather than a
 /// clang AST address space
 inline bool isTargetAddressSpace(LangAS AS) {
   return (unsigned)AS >= (unsigned)LangAS::FirstTargetAddressSpace;
 }
  
 inline unsigned toTargetAddressSpace(LangAS AS) {
   assert(isTargetAddressSpace(AS));
   return (unsigned)AS - (unsigned)LangAS::FirstTargetAddressSpace;
 }
  
 inline LangAS getLangASFromTargetAS(unsigned TargetAS) {
   return static_cast<LangAS>((TargetAS) +
                              (unsigned)LangAS::FirstTargetAddressSpace);
 }
  
 inline bool isPtrSizeAddressSpace(LangAS AS) {
   return (AS == LangAS::ptr32_sptr || AS == LangAS::ptr32_uptr ||
           AS == LangAS::ptr64);
 }
/// Kinds of BuiltinTemplateDecl.
enum BuiltinTemplateKind : int {
  /// This names the __make_integer_seq BuiltinTemplateDecl.
  BTK__make_integer_seq,

  /// This names the __type_pack_element BuiltinTemplateDecl.
  BTK__type_pack_element
};

enum class FloatModeKind {
  NoFloat = 255,
  Float = 0,
  Double,
  LongDouble,
  Float128,
  Ibm128
};

/// Fields controlling how types are laid out in memory; these may need to
/// be copied for targets like AMDGPU that base their ABIs on an auxiliary
/// CPU target.
struct TransferrableTargetInfo {
  unsigned char PointerWidth, PointerAlign;
  unsigned char BoolWidth, BoolAlign;
  unsigned char IntWidth, IntAlign;
  unsigned char HalfWidth, HalfAlign;
  unsigned char BFloat16Width, BFloat16Align;
  unsigned char FloatWidth, FloatAlign;
  unsigned char DoubleWidth, DoubleAlign;
  unsigned char LongDoubleWidth, LongDoubleAlign, Float128Align, Ibm128Align;
  unsigned char LargeArrayMinWidth, LargeArrayAlign;
  unsigned char LongWidth, LongAlign;
  unsigned char LongLongWidth, LongLongAlign;

  // Fixed point bit widths
  unsigned char ShortAccumWidth, ShortAccumAlign;
  unsigned char AccumWidth, AccumAlign;
  unsigned char LongAccumWidth, LongAccumAlign;
  unsigned char ShortFractWidth, ShortFractAlign;
  unsigned char FractWidth, FractAlign;
  unsigned char LongFractWidth, LongFractAlign;

  // If true, unsigned fixed point types have the same number of fractional bits
  // as their signed counterparts, forcing the unsigned types to have one extra
  // bit of padding. Otherwise, unsigned fixed point types have
  // one more fractional bit than its corresponding signed type. This is false
  // by default.
  bool PaddingOnUnsignedFixedPoint;

  // Fixed point integral and fractional bit sizes
  // Saturated types share the same integral/fractional bits as their
  // corresponding unsaturated types.
  // For simplicity, the fractional bits in a _Fract type will be one less the
  // width of that _Fract type. This leaves all signed _Fract types having no
  // padding and unsigned _Fract types will only have 1 bit of padding after the
  // sign if PaddingOnUnsignedFixedPoint is set.
  unsigned char ShortAccumScale;
  unsigned char AccumScale;
  unsigned char LongAccumScale;

  unsigned char SuitableAlign;
  unsigned char DefaultAlignForAttributeAligned;
  unsigned char MinGlobalAlign;

  unsigned short NewAlign;
  unsigned MaxVectorAlign;
  unsigned MaxTLSAlign;

  const llvm::fltSemantics *HalfFormat, *BFloat16Format, *FloatFormat,
      *DoubleFormat, *LongDoubleFormat, *Float128Format, *Ibm128Format;

  ///===---- Target Data Type Query Methods -------------------------------===//
  enum IntType {
    NoInt = 0,
    SignedChar,
    UnsignedChar,
    SignedShort,
    UnsignedShort,
    SignedInt,
    UnsignedInt,
    SignedLong,
    UnsignedLong,
    SignedLongLong,
    UnsignedLongLong
  };

protected:
  IntType SizeType, IntMaxType, PtrDiffType, IntPtrType, WCharType, WIntType,
      Char16Type, Char32Type, Int64Type, Int16Type, SigAtomicType,
      ProcessIDType;

  /// Whether Objective-C's built-in boolean type should be signed char.
  ///
  /// Otherwise, when this flag is not set, the normal built-in boolean type is
  /// used.
  unsigned UseSignedCharForObjCBool : 1;

  /// Control whether the alignment of bit-field types is respected when laying
  /// out structures. If true, then the alignment of the bit-field type will be
  /// used to (a) impact the alignment of the containing structure, and (b)
  /// ensure that the individual bit-field will not straddle an alignment
  /// boundary.
  unsigned UseBitFieldTypeAlignment : 1;

  /// Whether zero length bitfields (e.g., int : 0;) force alignment of
  /// the next bitfield.
  ///
  /// If the alignment of the zero length bitfield is greater than the member
  /// that follows it, `bar', `bar' will be aligned as the type of the
  /// zero-length bitfield.
  unsigned UseZeroLengthBitfieldAlignment : 1;

  /// Whether zero length bitfield alignment is respected if they are the
  /// leading members.
  unsigned UseLeadingZeroLengthBitfield : 1;

  ///  Whether explicit bit field alignment attributes are honored.
  unsigned UseExplicitBitFieldAlignment : 1;

  /// If non-zero, specifies a fixed alignment value for bitfields that follow
  /// zero length bitfield, regardless of the zero length bitfield type.
  unsigned ZeroLengthBitfieldBoundary;

  /// If non-zero, specifies a maximum alignment to truncate alignment
  /// specified in the aligned attribute of a static variable to this value.
  unsigned MaxAlignedAttribute;
};
/*
/// OpenCL type kinds.
enum OpenCLTypeKind : uint8_t {
  OCLTK_Default,
  OCLTK_ClkEvent,
  OCLTK_Event,
  OCLTK_Image,
  OCLTK_Pipe,
  OCLTK_Queue,
  OCLTK_ReserveID,
  OCLTK_Sampler,
};*/

/// Exposes information about the current target.
///
class TargetInfo : public virtual TransferrableTargetInfo,
                   public RefCountedBase<TargetInfo> {
  std::shared_ptr<TargetOptions> TargetOpts;
  llvm::Triple Triple;
protected:
  // Target values set by the ctor of the actual target implementation.  Default
  // values are specified by the TargetInfo constructor.
  bool BigEndian;
  bool TLSSupported;
  bool VLASupported;
  bool NoAsmVariants;  // True if {|} are normal characters.
  bool HasLegalHalfType; // True if the backend supports operations on the half
                         // LLVM IR type.
  bool HasFloat128;
  bool HasFloat16;
  bool HasBFloat16;
  bool HasIbm128;
  bool HasLongDouble;
  bool HasFPReturn;
  bool HasStrictFP;

  unsigned char MaxAtomicPromoteWidth, MaxAtomicInlineWidth;
  unsigned short SimdDefaultAlign;
  std::string DataLayoutString;
  const char *UserLabelPrefix;
  const char *MCountName;
  unsigned char RegParmMax, SSERegParmMax;
//  TargetCXXABI TheCXXABI;
  unsigned ProgramAddrSpace;

  mutable StringRef PlatformName;
  mutable VersionTuple PlatformMinVersion;

  unsigned HasAlignMac68kSupport : 1;
  unsigned RealTypeUsesObjCFPRet : 3;
  unsigned ComplexLongDoubleUsesFP2Ret : 1;

  unsigned HasBuiltinMSVaList : 1;

  unsigned IsRenderScriptTarget : 1;

  unsigned HasAArch64SVETypes : 1;

  unsigned HasRISCVVTypes : 1;

  unsigned AllowAMDGPUUnsafeFPAtomics : 1;

  unsigned ARMCDECoprocMask : 8;

  unsigned MaxOpenCLWorkGroupSize;

  // TargetInfo Constructor.  Default initializes all fields.
TargetInfo(const llvm::Triple &T) : Triple(T) {
  // Set defaults.  Defaults are set for a 32-bit RISC platform, like PPC or
  // SPARC.  These should be overridden by concrete targets as needed.
  BigEndian = !T.isLittleEndian();
  TLSSupported = true;
  VLASupported = true;
  NoAsmVariants = false;
  HasLegalHalfType = false;
  HasFloat128 = false;
  HasIbm128 = false;
  HasFloat16 = false;
  HasBFloat16 = false;
  HasLongDouble = true;
  HasFPReturn = true;
  HasStrictFP = false;
  PointerWidth = PointerAlign = 32;
  BoolWidth = BoolAlign = 8;
  IntWidth = IntAlign = 32;
  LongWidth = LongAlign = 32;
  LongLongWidth = LongLongAlign = 64;

  // Fixed point default bit widths
  ShortAccumWidth = ShortAccumAlign = 16;
  AccumWidth = AccumAlign = 32;
  LongAccumWidth = LongAccumAlign = 64;
  ShortFractWidth = ShortFractAlign = 8;
  FractWidth = FractAlign = 16;
  LongFractWidth = LongFractAlign = 32;

  // Fixed point default integral and fractional bit sizes
  // We give the _Accum 1 fewer fractional bits than their corresponding _Fract
  // types by default to have the same number of fractional bits between _Accum
  // and _Fract types.
  PaddingOnUnsignedFixedPoint = false;
  ShortAccumScale = 7;
  AccumScale = 15;
  LongAccumScale = 31;

  SuitableAlign = 64;
  DefaultAlignForAttributeAligned = 128;
  MinGlobalAlign = 0;
  // From the glibc documentation, on GNU systems, malloc guarantees 16-byte
  // alignment on 64-bit systems and 8-byte alignment on 32-bit systems. See
  // https://www.gnu.org/software/libc/manual/html_node/Malloc-Examples.html.
  // This alignment guarantee also applies to Windows and Android. On Darwin
  // and OpenBSD, the alignment is 16 bytes on both 64-bit and 32-bit systems.
  if (T.isGNUEnvironment() || T.isWindowsMSVCEnvironment() || T.isAndroid())
    NewAlign = Triple.isArch64Bit() ? 128 : Triple.isArch32Bit() ? 64 : 0;
  else if (T.isOSDarwin() || T.isOSOpenBSD())
    NewAlign = 128;
  else
    NewAlign = 0; // Infer from basic type alignment.
  HalfWidth = 16;
  HalfAlign = 16;
  FloatWidth = 32;
  FloatAlign = 32;
  DoubleWidth = 64;
  DoubleAlign = 64;
  LongDoubleWidth = 64;
  LongDoubleAlign = 64;
  Float128Align = 128;
  Ibm128Align = 128;
  LargeArrayMinWidth = 0;
  LargeArrayAlign = 0;
  MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 0;
  MaxVectorAlign = 0;
  MaxTLSAlign = 0;
  SimdDefaultAlign = 0;
  SizeType = UnsignedLong;
  PtrDiffType = SignedLong;
  IntMaxType = SignedLongLong;
  IntPtrType = SignedLong;
  WCharType = SignedInt;
  WIntType = SignedInt;
  Char16Type = UnsignedShort;
  Char32Type = UnsignedInt;
  Int64Type = SignedLongLong;
  Int16Type = SignedShort;
  SigAtomicType = SignedInt;
  ProcessIDType = SignedInt;
  UseSignedCharForObjCBool = true;
  UseBitFieldTypeAlignment = true;
  UseZeroLengthBitfieldAlignment = false;
  UseLeadingZeroLengthBitfield = true;
  UseExplicitBitFieldAlignment = true;
  ZeroLengthBitfieldBoundary = 0;
  MaxAlignedAttribute = 0;
  HalfFormat = &llvm::APFloat::IEEEhalf();
  FloatFormat = &llvm::APFloat::IEEEsingle();
  DoubleFormat = &llvm::APFloat::IEEEdouble();
  LongDoubleFormat = &llvm::APFloat::IEEEdouble();
  Float128Format = &llvm::APFloat::IEEEquad();
  Ibm128Format = &llvm::APFloat::PPCDoubleDouble();
  MCountName = "mcount";
  UserLabelPrefix = "_";
  RegParmMax = 0;
  SSERegParmMax = 0;
  HasAlignMac68kSupport = false;
  HasBuiltinMSVaList = false;
  IsRenderScriptTarget = false;
  HasAArch64SVETypes = false;
  HasRISCVVTypes = false;
  AllowAMDGPUUnsafeFPAtomics = false;
  ARMCDECoprocMask = 0;

  // Default to no types using fpret.
  RealTypeUsesObjCFPRet = 0;

  // Default to not using fp2ret for __Complex long double
  ComplexLongDoubleUsesFP2Ret = false;

  UseAddrSpaceMapMangling = false;

  // Default to an unknown platform name.
  PlatformName = "unknown";
  PlatformMinVersion = VersionTuple();

  ProgramAddrSpace = 0;
}

  // UserLabelPrefix must match DL's getGlobalPrefix() when interpreted
  // as a DataLayout object.
  void resetDataLayout(StringRef DL, const char *ULP="") {
    DataLayoutString = DL.str();
    UserLabelPrefix = ULP;
  }

public:
  /// Construct a target for the given options.
  ///
  /// \param Opts - The options to use to initialize the target. The target may
  /// modify the options to canonicalize the target feature information to match
  /// what the backend expects.
  static TargetInfo *
  CreateTargetInfo(/*DiagnosticsEngine &Diags,*/
                   const std::shared_ptr<TargetOptions> &Opts);

  virtual ~TargetInfo() {};

  /// Retrieve the target options.
  TargetOptions &getTargetOpts() const {
    assert(TargetOpts && "Missing target options");
    return *TargetOpts;
  }

  /// The different kinds of __builtin_va_list types defined by
  /// the target implementation.
  enum BuiltinVaListKind {
    /// typedef char* __builtin_va_list;
    CharPtrBuiltinVaList = 0,

    /// typedef void* __builtin_va_list;
    VoidPtrBuiltinVaList,

    /// __builtin_va_list as defined by the AArch64 ABI
    /// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055a/IHI0055A_aapcs64.pdf
    AArch64ABIBuiltinVaList,

    /// __builtin_va_list as defined by the PNaCl ABI:
    /// http://www.chromium.org/nativeclient/pnacl/bitcode-abi#TOC-Machine-Types
    PNaClABIBuiltinVaList,

    /// __builtin_va_list as defined by the Power ABI:
    /// https://www.power.org
    ///        /resources/downloads/Power-Arch-32-bit-ABI-supp-1.0-Embedded.pdf
    PowerABIBuiltinVaList,

    /// __builtin_va_list as defined by the x86-64 ABI:
    /// http://refspecs.linuxbase.org/elf/x86_64-abi-0.21.pdf
    X86_64ABIBuiltinVaList,

    /// __builtin_va_list as defined by ARM AAPCS ABI
    /// http://infocenter.arm.com
    //        /help/topic/com.arm.doc.ihi0042d/IHI0042D_aapcs.pdf
    AAPCSABIBuiltinVaList,

    // typedef struct __va_list_tag
    //   {
    //     long __gpr;
    //     long __fpr;
    //     void *__overflow_arg_area;
    //     void *__reg_save_area;
    //   } va_list[1];
    SystemZBuiltinVaList,

    // typedef struct __va_list_tag {
    //    void *__current_saved_reg_area_pointer;
    //    void *__saved_reg_area_end_pointer;
    //    void *__overflow_area_pointer;
    //} va_list;
    HexagonBuiltinVaList
  };

protected:
  /// Specify if mangling based on address space map should be used or
  /// not for language specific address spaces
  bool UseAddrSpaceMapMangling;

public:
  IntType getSizeType() const { return SizeType; }
  IntType getSignedSizeType() const {
    switch (SizeType) {
    case UnsignedShort:
      return SignedShort;
    case UnsignedInt:
      return SignedInt;
    case UnsignedLong:
      return SignedLong;
    case UnsignedLongLong:
      return SignedLongLong;
    default:
      llvm_unreachable("Invalid SizeType");
    }
  }
  IntType getIntMaxType() const { return IntMaxType; }
  IntType getUIntMaxType() const {
    return getCorrespondingUnsignedType(IntMaxType);
  }
  IntType getPtrDiffType(unsigned AddrSpace) const {
    return AddrSpace == 0 ? PtrDiffType : getPtrDiffTypeV(AddrSpace);
  }
  IntType getUnsignedPtrDiffType(unsigned AddrSpace) const {
    return getCorrespondingUnsignedType(getPtrDiffType(AddrSpace));
  }
  IntType getIntPtrType() const { return IntPtrType; }
  IntType getUIntPtrType() const {
    return getCorrespondingUnsignedType(IntPtrType);
  }
  IntType getWCharType() const { return WCharType; }
  IntType getWIntType() const { return WIntType; }
  IntType getChar16Type() const { return Char16Type; }
  IntType getChar32Type() const { return Char32Type; }
  IntType getInt64Type() const { return Int64Type; }
  IntType getUInt64Type() const {
    return getCorrespondingUnsignedType(Int64Type);
  }
  IntType getInt16Type() const { return Int16Type; }
  IntType getUInt16Type() const {
    return getCorrespondingUnsignedType(Int16Type);
  }
  IntType getSigAtomicType() const { return SigAtomicType; }
  IntType getProcessIDType() const { return ProcessIDType; }

  static IntType getCorrespondingUnsignedType(IntType T) {
    switch (T) {
    case SignedChar:
      return UnsignedChar;
    case SignedShort:
      return UnsignedShort;
    case SignedInt:
      return UnsignedInt;
    case SignedLong:
      return UnsignedLong;
    case SignedLongLong:
      return UnsignedLongLong;
    default:
      llvm_unreachable("Unexpected signed integer type");
    }
  }

  /// In the event this target uses the same number of fractional bits for its
  /// unsigned types as it does with its signed counterparts, there will be
  /// exactly one bit of padding.
  /// Return true if unsigned fixed point types have padding for this target.
  bool doUnsignedFixedPointTypesHavePadding() const {
    return PaddingOnUnsignedFixedPoint;
  }

  /// Return the width (in bits) of the specified integer type enum.
  ///
  /// For example, SignedInt -> getIntWidth().
unsigned getTypeWidth(IntType T) const {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case UnsignedChar:     return getCharWidth();
  case SignedShort:
  case UnsignedShort:    return getShortWidth();
  case SignedInt:
  case UnsignedInt:      return getIntWidth();
  case SignedLong:
  case UnsignedLong:     return getLongWidth();
  case SignedLongLong:
  case UnsignedLongLong: return getLongLongWidth();
  };
}

  /// Return integer type with specified width.
  virtual IntType getIntTypeByWidth(unsigned BitWidth, bool IsSigned) const {
    if (getCharWidth() == BitWidth)
      return IsSigned ? SignedChar : UnsignedChar;
    if (getShortWidth() == BitWidth)
      return IsSigned ? SignedShort : UnsignedShort;
    if (getIntWidth() == BitWidth)
      return IsSigned ? SignedInt : UnsignedInt;
    if (getLongWidth() == BitWidth)
      return IsSigned ? SignedLong : UnsignedLong;
    if (getLongLongWidth() == BitWidth)
      return IsSigned ? SignedLongLong : UnsignedLongLong;
    return NoInt;
}

  /// Return the smallest integer type with at least the specified width.
  virtual IntType getLeastIntTypeByWidth(unsigned BitWidth,
                                         bool IsSigned) const {
    if (getCharWidth() >= BitWidth)
      return IsSigned ? SignedChar : UnsignedChar;
    if (getShortWidth() >= BitWidth)
      return IsSigned ? SignedShort : UnsignedShort;
    if (getIntWidth() >= BitWidth)
      return IsSigned ? SignedInt : UnsignedInt;
    if (getLongWidth() >= BitWidth)
      return IsSigned ? SignedLong : UnsignedLong;
    if (getLongLongWidth() >= BitWidth)
      return IsSigned ? SignedLongLong : UnsignedLongLong;
    return NoInt;
}

  /// Return floating point type with specified width. On PPC, there are
  /// three possible types for 128-bit floating point: "PPC double-double",
  /// IEEE 754R quad precision, and "long double" (which under the covers
  /// is represented as one of those two). At this time, there is no support
  /// for an explicit "PPC double-double" type (i.e. __ibm128) so we only
  /// need to differentiate between "long double" and IEEE quad precision.
  FloatModeKind getRealTypeByWidth(unsigned BitWidth,
                                   FloatModeKind ExplicitType) const {
    if (getFloatWidth() == BitWidth)
      return FloatModeKind::Float;
    if (getDoubleWidth() == BitWidth)
      return FloatModeKind::Double;
  
    switch (BitWidth) {
    case 96:
      if (&getLongDoubleFormat() == &llvm::APFloat::x87DoubleExtended())
        return FloatModeKind::LongDouble;
      break;
    case 128:
      // The caller explicitly asked for an IEEE compliant type but we still
      // have to check if the target supports it.
      if (ExplicitType == FloatModeKind::Float128)
        return hasFloat128Type() ? FloatModeKind::Float128
                                 : FloatModeKind::NoFloat;
      if (ExplicitType == FloatModeKind::Ibm128)
        return hasIbm128Type() ? FloatModeKind::Ibm128
                               : FloatModeKind::NoFloat;
      if (&getLongDoubleFormat() == &llvm::APFloat::PPCDoubleDouble() ||
          &getLongDoubleFormat() == &llvm::APFloat::IEEEquad())
        return FloatModeKind::LongDouble;
      if (hasFloat128Type())
        return FloatModeKind::Float128;
      break;
    }
  
    return FloatModeKind::NoFloat;
}

  /// Return the alignment (in bits) of the specified integer type enum.
  ///
  /// For example, SignedInt -> getIntAlign().
  unsigned getTypeAlign(IntType T) const {
    switch (T) {
      default: llvm_unreachable("not an integer!");
      case SignedChar:
      case UnsignedChar:     return getCharAlign();
      case SignedShort:
      case UnsignedShort:    return getShortAlign();
      case SignedInt:
      case UnsignedInt:      return getIntAlign();
      case SignedLong:
      case UnsignedLong:     return getLongAlign();
      case SignedLongLong:
      case UnsignedLongLong: return getLongLongAlign();
    };
}


  /// Returns true if the type is signed; false otherwise.
  static bool isTypeSigned(IntType T) {
    switch (T) {
      default: llvm_unreachable("not an integer!");
      case SignedChar:
      case SignedShort:
      case SignedInt:
      case SignedLong:
      case SignedLongLong:
        return true;
      case UnsignedChar:
      case UnsignedShort:
      case UnsignedInt:
      case UnsignedLong:
      case UnsignedLongLong:
        return false;
    };
  }

  /// Return the width of pointers on this target, for the
  /// specified address space.
  uint64_t getPointerWidth(unsigned AddrSpace) const {
    return AddrSpace == 0 ? PointerWidth : getPointerWidthV(AddrSpace);
  }
  uint64_t getPointerAlign(unsigned AddrSpace) const {
    return AddrSpace == 0 ? PointerAlign : getPointerAlignV(AddrSpace);
  }

  /// Return the maximum width of pointers on this target.
  virtual uint64_t getMaxPointerWidth() const {
    return PointerWidth;
  }

  /// Get integer value for null pointer.
  /// \param AddrSpace address space of pointee in source language.
  virtual uint64_t getNullPointerValue(LangAS AddrSpace) const { return 0; }

  /// Return the size of '_Bool' and C++ 'bool' for this target, in bits.
  unsigned getBoolWidth() const { return BoolWidth; }

  /// Return the alignment of '_Bool' and C++ 'bool' for this target.
  unsigned getBoolAlign() const { return BoolAlign; }

  unsigned getCharWidth() const { return 8; } // FIXME
  unsigned getCharAlign() const { return 8; } // FIXME

  /// Return the size of 'signed short' and 'unsigned short' for this
  /// target, in bits.
  unsigned getShortWidth() const { return 16; } // FIXME

  /// Return the alignment of 'signed short' and 'unsigned short' for
  /// this target.
  unsigned getShortAlign() const { return 16; } // FIXME

  /// getIntWidth/Align - Return the size of 'signed int' and 'unsigned int' for
  /// this target, in bits.
  unsigned getIntWidth() const { return IntWidth; }
  unsigned getIntAlign() const { return IntAlign; }

  /// getLongWidth/Align - Return the size of 'signed long' and 'unsigned long'
  /// for this target, in bits.
  unsigned getLongWidth() const { return LongWidth; }
  unsigned getLongAlign() const { return LongAlign; }

  /// getLongLongWidth/Align - Return the size of 'signed long long' and
  /// 'unsigned long long' for this target, in bits.
  unsigned getLongLongWidth() const { return LongLongWidth; }
  unsigned getLongLongAlign() const { return LongLongAlign; }

  /// getShortAccumWidth/Align - Return the size of 'signed short _Accum' and
  /// 'unsigned short _Accum' for this target, in bits.
  unsigned getShortAccumWidth() const { return ShortAccumWidth; }
  unsigned getShortAccumAlign() const { return ShortAccumAlign; }

  /// getAccumWidth/Align - Return the size of 'signed _Accum' and
  /// 'unsigned _Accum' for this target, in bits.
  unsigned getAccumWidth() const { return AccumWidth; }
  unsigned getAccumAlign() const { return AccumAlign; }

  /// getLongAccumWidth/Align - Return the size of 'signed long _Accum' and
  /// 'unsigned long _Accum' for this target, in bits.
  unsigned getLongAccumWidth() const { return LongAccumWidth; }
  unsigned getLongAccumAlign() const { return LongAccumAlign; }

  /// getShortFractWidth/Align - Return the size of 'signed short _Fract' and
  /// 'unsigned short _Fract' for this target, in bits.
  unsigned getShortFractWidth() const { return ShortFractWidth; }
  unsigned getShortFractAlign() const { return ShortFractAlign; }

  /// getFractWidth/Align - Return the size of 'signed _Fract' and
  /// 'unsigned _Fract' for this target, in bits.
  unsigned getFractWidth() const { return FractWidth; }
  unsigned getFractAlign() const { return FractAlign; }

  /// getLongFractWidth/Align - Return the size of 'signed long _Fract' and
  /// 'unsigned long _Fract' for this target, in bits.
  unsigned getLongFractWidth() const { return LongFractWidth; }
  unsigned getLongFractAlign() const { return LongFractAlign; }

  /// getShortAccumScale/IBits - Return the number of fractional/integral bits
  /// in a 'signed short _Accum' type.
  unsigned getShortAccumScale() const { return ShortAccumScale; }
  unsigned getShortAccumIBits() const {
    return ShortAccumWidth - ShortAccumScale - 1;
  }

  /// getAccumScale/IBits - Return the number of fractional/integral bits
  /// in a 'signed _Accum' type.
  unsigned getAccumScale() const { return AccumScale; }
  unsigned getAccumIBits() const { return AccumWidth - AccumScale - 1; }

  /// getLongAccumScale/IBits - Return the number of fractional/integral bits
  /// in a 'signed long _Accum' type.
  unsigned getLongAccumScale() const { return LongAccumScale; }
  unsigned getLongAccumIBits() const {
    return LongAccumWidth - LongAccumScale - 1;
  }

  /// getUnsignedShortAccumScale/IBits - Return the number of
  /// fractional/integral bits in a 'unsigned short _Accum' type.
  unsigned getUnsignedShortAccumScale() const {
    return PaddingOnUnsignedFixedPoint ? ShortAccumScale : ShortAccumScale + 1;
  }
  unsigned getUnsignedShortAccumIBits() const {
    return PaddingOnUnsignedFixedPoint
               ? getShortAccumIBits()
               : ShortAccumWidth - getUnsignedShortAccumScale();
  }

  /// getUnsignedAccumScale/IBits - Return the number of fractional/integral
  /// bits in a 'unsigned _Accum' type.
  unsigned getUnsignedAccumScale() const {
    return PaddingOnUnsignedFixedPoint ? AccumScale : AccumScale + 1;
  }
  unsigned getUnsignedAccumIBits() const {
    return PaddingOnUnsignedFixedPoint ? getAccumIBits()
                                       : AccumWidth - getUnsignedAccumScale();
  }

  /// getUnsignedLongAccumScale/IBits - Return the number of fractional/integral
  /// bits in a 'unsigned long _Accum' type.
  unsigned getUnsignedLongAccumScale() const {
    return PaddingOnUnsignedFixedPoint ? LongAccumScale : LongAccumScale + 1;
  }
  unsigned getUnsignedLongAccumIBits() const {
    return PaddingOnUnsignedFixedPoint
               ? getLongAccumIBits()
               : LongAccumWidth - getUnsignedLongAccumScale();
  }

  /// getShortFractScale - Return the number of fractional bits
  /// in a 'signed short _Fract' type.
  unsigned getShortFractScale() const { return ShortFractWidth - 1; }

  /// getFractScale - Return the number of fractional bits
  /// in a 'signed _Fract' type.
  unsigned getFractScale() const { return FractWidth - 1; }

  /// getLongFractScale - Return the number of fractional bits
  /// in a 'signed long _Fract' type.
  unsigned getLongFractScale() const { return LongFractWidth - 1; }

  /// getUnsignedShortFractScale - Return the number of fractional bits
  /// in a 'unsigned short _Fract' type.
  unsigned getUnsignedShortFractScale() const {
    return PaddingOnUnsignedFixedPoint ? getShortFractScale()
                                       : getShortFractScale() + 1;
  }

  /// getUnsignedFractScale - Return the number of fractional bits
  /// in a 'unsigned _Fract' type.
  unsigned getUnsignedFractScale() const {
    return PaddingOnUnsignedFixedPoint ? getFractScale() : getFractScale() + 1;
  }

  /// getUnsignedLongFractScale - Return the number of fractional bits
  /// in a 'unsigned long _Fract' type.
  unsigned getUnsignedLongFractScale() const {
    return PaddingOnUnsignedFixedPoint ? getLongFractScale()
                                       : getLongFractScale() + 1;
  }

  /// Determine whether the __int128 type is supported on this target.
  virtual bool hasInt128Type() const {
    return (getPointerWidth(0) >= 64) || getTargetOpts().ForceEnableInt128;
  } // FIXME

  /// Determine whether the _BitInt type is supported on this target. This
  /// limitation is put into place for ABI reasons.
  /// FIXME: _BitInt is a required type in C23, so there's not much utility in
  /// asking whether the target supported it or not; I think this should be
  /// removed once backends have been alerted to the type and have had the
  /// chance to do implementation work if needed.
  virtual bool hasBitIntType() const {
    return false;
  }

  // Different targets may support a different maximum width for the _BitInt
  // type, depending on what operations are supported.
  virtual size_t getMaxBitIntWidth() const {
    // FIXME: this value should be llvm::IntegerType::MAX_INT_BITS, which is
    // maximum bit width that LLVM claims its IR can support. However, most
    // backends currently have a bug where they only support division
    // operations on types that are <= 128 bits and crash otherwise. We're
    // setting the max supported value to 128 to be conservative.
    return 128;
  }

  /// Determine whether _Float16 is supported on this target.
  virtual bool hasLegalHalfType() const { return HasLegalHalfType; }

  /// Determine whether the __float128 type is supported on this target.
  virtual bool hasFloat128Type() const { return HasFloat128; }

  /// Determine whether the _Float16 type is supported on this target.
  virtual bool hasFloat16Type() const { return HasFloat16; }

  /// Determine whether the _BFloat16 type is supported on this target.
  virtual bool hasBFloat16Type() const { return HasBFloat16; }

  /// Determine whether the __ibm128 type is supported on this target.
  virtual bool hasIbm128Type() const { return HasIbm128; }

  /// Determine whether the long double type is supported on this target.
  virtual bool hasLongDoubleType() const { return HasLongDouble; }

  /// Determine whether return of a floating point value is supported
  /// on this target.
  virtual bool hasFPReturn() const { return HasFPReturn; }

  /// Determine whether constrained floating point is supported on this target.
  virtual bool hasStrictFP() const { return HasStrictFP; }

  /// Return the alignment that is the largest alignment ever used for any
  /// scalar/SIMD data type on the target machine you are compiling for
  /// (including types with an extended alignment requirement).
  unsigned getSuitableAlign() const { return SuitableAlign; }

  /// Return the default alignment for __attribute__((aligned)) on
  /// this target, to be used if no alignment value is specified.
  unsigned getDefaultAlignForAttributeAligned() const {
    return DefaultAlignForAttributeAligned;
  }

  /// getMinGlobalAlign - Return the minimum alignment of a global variable,
  /// unless its alignment is explicitly reduced via attributes.
  virtual unsigned getMinGlobalAlign (uint64_t) const {
    return MinGlobalAlign;
  }

  /// Return the largest alignment for which a suitably-sized allocation with
  /// '::operator new(size_t)' is guaranteed to produce a correctly-aligned
  /// pointer.
  unsigned getNewAlign() const {
    return NewAlign ? NewAlign : std::max(LongDoubleAlign, LongLongAlign);
  }

  /// getWCharWidth/Align - Return the size of 'wchar_t' for this target, in
  /// bits.
  unsigned getWCharWidth() const { return getTypeWidth(WCharType); }
  unsigned getWCharAlign() const { return getTypeAlign(WCharType); }

  /// getChar16Width/Align - Return the size of 'char16_t' for this target, in
  /// bits.
  unsigned getChar16Width() const { return getTypeWidth(Char16Type); }
  unsigned getChar16Align() const { return getTypeAlign(Char16Type); }

  /// getChar32Width/Align - Return the size of 'char32_t' for this target, in
  /// bits.
  unsigned getChar32Width() const { return getTypeWidth(Char32Type); }
  unsigned getChar32Align() const { return getTypeAlign(Char32Type); }

  /// getHalfWidth/Align/Format - Return the size/align/format of 'half'.
  unsigned getHalfWidth() const { return HalfWidth; }
  unsigned getHalfAlign() const { return HalfAlign; }
  const llvm::fltSemantics &getHalfFormat() const { return *HalfFormat; }

  /// getFloatWidth/Align/Format - Return the size/align/format of 'float'.
  unsigned getFloatWidth() const { return FloatWidth; }
  unsigned getFloatAlign() const { return FloatAlign; }
  const llvm::fltSemantics &getFloatFormat() const { return *FloatFormat; }

  /// getBFloat16Width/Align/Format - Return the size/align/format of '__bf16'.
  unsigned getBFloat16Width() const { return BFloat16Width; }
  unsigned getBFloat16Align() const { return BFloat16Align; }
  const llvm::fltSemantics &getBFloat16Format() const { return *BFloat16Format; }

  /// getDoubleWidth/Align/Format - Return the size/align/format of 'double'.
  unsigned getDoubleWidth() const { return DoubleWidth; }
  unsigned getDoubleAlign() const { return DoubleAlign; }
  const llvm::fltSemantics &getDoubleFormat() const { return *DoubleFormat; }

  /// getLongDoubleWidth/Align/Format - Return the size/align/format of 'long
  /// double'.
  unsigned getLongDoubleWidth() const { return LongDoubleWidth; }
  unsigned getLongDoubleAlign() const { return LongDoubleAlign; }
  const llvm::fltSemantics &getLongDoubleFormat() const {
    return *LongDoubleFormat;
  }

  /// getFloat128Width/Align/Format - Return the size/align/format of
  /// '__float128'.
  unsigned getFloat128Width() const { return 128; }
  unsigned getFloat128Align() const { return Float128Align; }
  const llvm::fltSemantics &getFloat128Format() const {
    return *Float128Format;
  }

  /// getIbm128Width/Align/Format - Return the size/align/format of
  /// '__ibm128'.
  unsigned getIbm128Width() const { return 128; }
  unsigned getIbm128Align() const { return Ibm128Align; }
  const llvm::fltSemantics &getIbm128Format() const { return *Ibm128Format; }

  /// Return the mangled code of long double.
  virtual const char *getLongDoubleMangling() const { return "e"; }

  /// Return the mangled code of __float128.
  virtual const char *getFloat128Mangling() const { return "g"; }

  /// Return the mangled code of __ibm128.
  virtual const char *getIbm128Mangling() const {
    llvm_unreachable("ibm128 not implemented on this target");
  }

  /// Return the mangled code of bfloat.
  virtual const char *getBFloat16Mangling() const {
    llvm_unreachable("bfloat not implemented on this target");
  }

  /// Return the value for the C99 FLT_EVAL_METHOD macro.
  virtual unsigned getFloatEvalMethod() const { return 0; }

  // getLargeArrayMinWidth/Align - Return the minimum array size that is
  // 'large' and its alignment.
  unsigned getLargeArrayMinWidth() const { return LargeArrayMinWidth; }
  unsigned getLargeArrayAlign() const { return LargeArrayAlign; }

  /// Return the maximum width lock-free atomic operation which will
  /// ever be supported for the given target
  unsigned getMaxAtomicPromoteWidth() const { return MaxAtomicPromoteWidth; }
  /// Return the maximum width lock-free atomic operation which can be
  /// inlined given the supported features of the given target.
  unsigned getMaxAtomicInlineWidth() const { return MaxAtomicInlineWidth; }
  /// Set the maximum inline or promote width lock-free atomic operation
  /// for the given target.
  virtual void setMaxAtomicWidth() {}
  /// Returns true if the given target supports lock-free atomic
  /// operations at the specified width and alignment.
  virtual bool hasBuiltinAtomic(uint64_t AtomicSizeInBits,
                                uint64_t AlignmentInBits) const {
    return AtomicSizeInBits <= AlignmentInBits &&
           AtomicSizeInBits <= getMaxAtomicInlineWidth() &&
           (AtomicSizeInBits <= getCharWidth() ||
            llvm::isPowerOf2_64(AtomicSizeInBits / getCharWidth()));
  }

  /// Return the maximum vector alignment supported for the given target.
  unsigned getMaxVectorAlign() const { return MaxVectorAlign; }
  /// Return default simd alignment for the given target. Generally, this
  /// value is type-specific, but this alignment can be used for most of the
  /// types for the given target.
  unsigned getSimdDefaultAlign() const { return SimdDefaultAlign; }

//  unsigned getMaxOpenCLWorkGroupSize() const { return MaxOpenCLWorkGroupSize; }

  /// Return the alignment (in bits) of the thrown exception object. This is
  /// only meaningful for targets that allocate C++ exceptions in a system
  /// runtime, such as those using the Itanium C++ ABI.
  virtual unsigned getExnObjectAlignment() const {
    // Itanium says that an _Unwind_Exception has to be "double-word"
    // aligned (and thus the end of it is also so-aligned), meaning 16
    // bytes.  Of course, that was written for the actual Itanium,
    // which is a 64-bit platform.  Classically, the ABI doesn't really
    // specify the alignment on other platforms, but in practice
    // libUnwind declares the struct with __attribute__((aligned)), so
    // we assume that alignment here.  (It's generally 16 bytes, but
    // some targets overwrite it.)
    return getDefaultAlignForAttributeAligned();
  }

  /// Return the size of intmax_t and uintmax_t for this target, in bits.
  unsigned getIntMaxTWidth() const {
    return getTypeWidth(IntMaxType);
  }

  /// Return the address space for functions for the given target.
  unsigned getProgramAddressSpace() const { return ProgramAddrSpace; }

  // Return the size of unwind_word for this target.
  virtual unsigned getUnwindWordWidth() const { return getPointerWidth(0); }

  /// Return the "preferred" register width on this target.
  virtual unsigned getRegisterWidth() const {
    // Currently we assume the register width on the target matches the pointer
    // width, we can introduce a new variable for this if/when some target wants
    // it.
    return PointerWidth;
  }

  /// \brief Returns the default value of the __USER_LABEL_PREFIX__ macro,
  /// which is the prefix given to user symbols by default.
  ///
  /// On most platforms this is "", but it is "_" on some.
  const char *getUserLabelPrefix() const { return UserLabelPrefix; }

  /// Returns the name of the mcount instrumentation function.
  const char *getMCountName() const {
    return MCountName;
  }

  /// Check if the Objective-C built-in boolean type should be signed
  /// char.
  ///
  /// Otherwise, if this returns false, the normal built-in boolean type
  /// should also be used for Objective-C.
  bool useSignedCharForObjCBool() const {
    return UseSignedCharForObjCBool;
  }
  void noSignedCharForObjCBool() {
    UseSignedCharForObjCBool = false;
  }

  /// Check whether the alignment of bit-field types is respected
  /// when laying out structures.
  bool useBitFieldTypeAlignment() const {
    return UseBitFieldTypeAlignment;
  }

  /// Check whether zero length bitfields should force alignment of
  /// the next member.
  bool useZeroLengthBitfieldAlignment() const {
    return UseZeroLengthBitfieldAlignment;
  }

  /// Check whether zero length bitfield alignment is respected if they are
  /// leading members.
  bool useLeadingZeroLengthBitfield() const {
    return UseLeadingZeroLengthBitfield;
  }

  /// Get the fixed alignment value in bits for a member that follows
  /// a zero length bitfield.
  unsigned getZeroLengthBitfieldBoundary() const {
    return ZeroLengthBitfieldBoundary;
  }

  /// Get the maximum alignment in bits for a static variable with
  /// aligned attribute.
  unsigned getMaxAlignedAttribute() const { return MaxAlignedAttribute; }

  /// Check whether explicit bitfield alignment attributes should be
  //  honored, as in "__attribute__((aligned(2))) int b : 1;".
  bool useExplicitBitFieldAlignment() const {
    return UseExplicitBitFieldAlignment;
  }

  /// Check whether this target support '\#pragma options align=mac68k'.
  bool hasAlignMac68kSupport() const {
    return HasAlignMac68kSupport;
  }

  /// Return the user string for the specified integer type enum.
  ///
  /// For example, SignedShort -> "short".
  static const char *getTypeName(IntType T) {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:       return "signed char";
  case UnsignedChar:     return "unsigned char";
  case SignedShort:      return "short";
  case UnsignedShort:    return "unsigned short";
  case SignedInt:        return "int";
  case UnsignedInt:      return "unsigned int";
  case SignedLong:       return "long int";
  case UnsignedLong:     return "long unsigned int";
  case SignedLongLong:   return "long long int";
  case UnsignedLongLong: return "long long unsigned int";
  }
}

  /// Return the constant suffix for the specified integer type enum.
  ///
  /// For example, SignedLong -> "L".
  const char *getTypeConstantSuffix(IntType T) const {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case SignedShort:
  case SignedInt:        return "";
  case SignedLong:       return "L";
  case SignedLongLong:   return "LL";
  case UnsignedChar:
    if (getCharWidth() < getIntWidth())
      return "";
    LLVM_FALLTHROUGH;
  case UnsignedShort:
    if (getShortWidth() < getIntWidth())
      return "";
    LLVM_FALLTHROUGH;
  case UnsignedInt:      return "U";
  case UnsignedLong:     return "UL";
  case UnsignedLongLong: return "ULL";
  }
}
  /// Return the printf format modifier for the specified
  /// integer type enum.
  ///
  /// For example, SignedLong -> "l".
  static const char *getTypeFormatModifier(IntType T) {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case UnsignedChar:     return "hh";
  case SignedShort:
  case UnsignedShort:    return "h";
  case SignedInt:
  case UnsignedInt:      return "";
  case SignedLong:
  case UnsignedLong:     return "l";
  case SignedLongLong:
  case UnsignedLongLong: return "ll";
  }
}

  /// Check whether the given real type should use the "fpret" flavor of
  /// Objective-C message passing on this target.
  bool useObjCFPRetForRealType(FloatModeKind T) const {
    return RealTypeUsesObjCFPRet & (1 << (int)T);
  }

  /// Check whether _Complex long double should use the "fp2ret" flavor
  /// of Objective-C message passing on this target.
  bool useObjCFP2RetForComplexLongDouble() const {
    return ComplexLongDoubleUsesFP2Ret;
  }

  /// Check whether llvm intrinsics such as llvm.convert.to.fp16 should be used
  /// to convert to and from __fp16.
  /// FIXME: This function should be removed once all targets stop using the
  /// conversion intrinsics.
  virtual bool useFP16ConversionIntrinsics() const {
    return true;
  }

  /// Specify if mangling based on address space map should be used or
  /// not for language specific address spaces
  bool useAddressSpaceMapMangling() const {
    return UseAddrSpaceMapMangling;
  }

  ///===---- Other target property query methods --------------------------===//

  /// Appends the target-specific \#define values for this
  /// target set to the specified buffer.
  virtual void getTargetDefines(const LangOptions &Opts,
                                MacroBuilder &Builder) const = 0;


  /// Return information about target-specific builtins for
  /// the current primary target, and info about which builtins are non-portable
  /// across the current set of primary and secondary targets.
  virtual ArrayRef<Builtin::Info> getTargetBuiltins() const = 0;

  /// Returns target-specific min and max values VScale_Range.
  virtual Optional<std::pair<unsigned, unsigned>>
  getVScaleRange(const LangOptions &LangOpts) const {
    return None;
  }
  /// The __builtin_clz* and __builtin_ctz* built-in
  /// functions are specified to have undefined results for zero inputs, but
  /// on targets that support these operations in a way that provides
  /// well-defined results for zero without loss of performance, it is a good
  /// idea to avoid optimizing based on that undef behavior.
  virtual bool isCLZForZeroUndef() const { return true; }

  /// Returns the kind of __builtin_va_list type that should be used
  /// with this target.
  virtual BuiltinVaListKind getBuiltinVaListKind() const = 0;

  /// Returns whether or not type \c __builtin_ms_va_list type is
  /// available on this target.
  bool hasBuiltinMSVaList() const { return HasBuiltinMSVaList; }

  /// Returns true for RenderScript.
  bool isRenderScriptTarget() const { return IsRenderScriptTarget; }

  /// Returns whether or not the AArch64 SVE built-in types are
  /// available on this target.
  bool hasAArch64SVETypes() const { return HasAArch64SVETypes; }

  /// Returns whether or not the RISC-V V built-in types are
  /// available on this target.
  bool hasRISCVVTypes() const { return HasRISCVVTypes; }

  /// Returns whether or not the AMDGPU unsafe floating point atomics are
  /// allowed.
  bool allowAMDGPUUnsafeFPAtomics() const { return AllowAMDGPUUnsafeFPAtomics; }

  /// For ARM targets returns a mask defining which coprocessors are configured
  /// as Custom Datapath.
  uint32_t getARMCDECoprocMask() const { return ARMCDECoprocMask; }

static StringRef removeGCCRegisterPrefix(StringRef Name) {
  if (Name[0] == '%' || Name[0] == '#')
    Name = Name.substr(1);

  return Name;
}

/// isValidClobber - Returns whether the passed in string is
/// a valid clobber in an inline asm statement. This is used by
/// Sema.
bool isValidClobber(StringRef Name) const {
  return (isValidGCCRegisterName(Name) || Name == "memory" || Name == "cc" ||
          Name == "unwind");
}

  /// Returns whether the passed in string is a valid register name
  /// according to GCC.
  ///
  /// This is used by Sema for inline asm statements.
  virtual bool isValidGCCRegisterName(StringRef Name) const {
  if (Name.empty())
    return false;

  // Get rid of any register prefix.
  Name = removeGCCRegisterPrefix(Name);
  if (Name.empty())
    return false;

  ArrayRef<const char *> Names = getGCCRegNames();

  // If we have a number it maps to an entry in the register name array.
  if (llvm::isDigit(Name[0])) {
    unsigned n;
    if (!Name.getAsInteger(0, n))
      return n < Names.size();
  }

  // Check register names.
  if (llvm::is_contained(Names, Name))
    return true;

  // Check any additional names that we have.
  for (const AddlRegName &ARN : getGCCAddlRegNames())
    for (const char *AN : ARN.Names) {
      if (!AN)
        break;
      // Make sure the register that the additional name is for is within
      // the bounds of the register names from above.
      if (AN == Name && ARN.RegNum < Names.size())
        return true;
    }

  // Now check aliases.
  for (const GCCRegAlias &GRA : getGCCRegAliases())
    for (const char *A : GRA.Aliases) {
      if (!A)
        break;
      if (A == Name)
        return true;
    }

  return false;
}

  /// Returns the "normalized" GCC register name.
  ///
  /// ReturnCannonical true will return the register name without any additions
  /// such as "{}" or "%" in it's canonical form, for example:
  /// ReturnCanonical = true and Name = "rax", will return "ax".
  StringRef getNormalizedGCCRegisterName(StringRef Name,
                                         bool ReturnCanonical = false) const {
  assert(isValidGCCRegisterName(Name) && "Invalid register passed in");

  // Get rid of any register prefix.
  Name = removeGCCRegisterPrefix(Name);

  ArrayRef<const char *> Names = getGCCRegNames();

  // First, check if we have a number.
  if (llvm::isDigit(Name[0])) {
    unsigned n;
    if (!Name.getAsInteger(0, n)) {
      assert(n < Names.size() && "Out of bounds register number!");
      return Names[n];
    }
  }

  // Check any additional names that we have.
  for (const AddlRegName &ARN : getGCCAddlRegNames())
    for (const char *AN : ARN.Names) {
      if (!AN)
        break;
      // Make sure the register that the additional name is for is within
      // the bounds of the register names from above.
      if (AN == Name && ARN.RegNum < Names.size())
        return ReturnCanonical ? Names[ARN.RegNum] : Name;
    }

  // Now check aliases.
  for (const GCCRegAlias &RA : getGCCRegAliases())
    for (const char *A : RA.Aliases) {
      if (!A)
        break;
      if (A == Name)
        return RA.Register;
    }

  return Name;
}

  virtual bool isSPRegName(StringRef) const { return false; }

  /// Extracts a register from the passed constraint (if it is a
  /// single-register constraint) and the asm label expression related to a
  /// variable in the input or output list of an inline asm statement.
  ///
  /// This function is used by Sema in order to diagnose conflicts between
  /// the clobber list and the input/output lists.
  virtual StringRef getConstraintRegister(StringRef Constraint,
                                          StringRef Expression) const {
    return "";
  }

  struct ConstraintInfo {
    enum {
      CI_None = 0x00,
      CI_AllowsMemory = 0x01,
      CI_AllowsRegister = 0x02,
      CI_ReadWrite = 0x04,         // "+r" output constraint (read and write).
      CI_HasMatchingInput = 0x08,  // This output operand has a matching input.
      CI_ImmediateConstant = 0x10, // This operand must be an immediate constant
      CI_EarlyClobber = 0x20,      // "&" output constraint (early clobber).
    };
    unsigned Flags;
    int TiedOperand;
    struct {
      int Min;
      int Max;
      bool isConstrained;
    } ImmRange;
    llvm::SmallSet<int, 4> ImmSet;

    std::string ConstraintStr;  // constraint: "=rm"
    std::string Name;           // Operand name: [foo] with no []'s.
  public:
    ConstraintInfo(StringRef ConstraintStr, StringRef Name)
        : Flags(0), TiedOperand(-1), ConstraintStr(ConstraintStr.str()),
          Name(Name.str()) {
      ImmRange.Min = ImmRange.Max = 0;
      ImmRange.isConstrained = false;
    }

    const std::string &getConstraintStr() const { return ConstraintStr; }
    const std::string &getName() const { return Name; }
    bool isReadWrite() const { return (Flags & CI_ReadWrite) != 0; }
    bool earlyClobber() { return (Flags & CI_EarlyClobber) != 0; }
    bool allowsRegister() const { return (Flags & CI_AllowsRegister) != 0; }
    bool allowsMemory() const { return (Flags & CI_AllowsMemory) != 0; }

    /// Return true if this output operand has a matching
    /// (tied) input operand.
    bool hasMatchingInput() const { return (Flags & CI_HasMatchingInput) != 0; }

    /// Return true if this input operand is a matching
    /// constraint that ties it to an output operand.
    ///
    /// If this returns true then getTiedOperand will indicate which output
    /// operand this is tied to.
    bool hasTiedOperand() const { return TiedOperand != -1; }
    unsigned getTiedOperand() const {
      assert(hasTiedOperand() && "Has no tied operand!");
      return (unsigned)TiedOperand;
    }

    bool requiresImmediateConstant() const {
      return (Flags & CI_ImmediateConstant) != 0;
    }
    bool isValidAsmImmediate(const llvm::APInt &Value) const {
      if (!ImmSet.empty())
        return Value.isSignedIntN(32) && ImmSet.contains(Value.getZExtValue());
      return !ImmRange.isConstrained ||
             (Value.sge(ImmRange.Min) && Value.sle(ImmRange.Max));
    }

    void setIsReadWrite() { Flags |= CI_ReadWrite; }
    void setEarlyClobber() { Flags |= CI_EarlyClobber; }
    void setAllowsMemory() { Flags |= CI_AllowsMemory; }
    void setAllowsRegister() { Flags |= CI_AllowsRegister; }
    void setHasMatchingInput() { Flags |= CI_HasMatchingInput; }
    void setRequiresImmediate(int Min, int Max) {
      Flags |= CI_ImmediateConstant;
      ImmRange.Min = Min;
      ImmRange.Max = Max;
      ImmRange.isConstrained = true;
    }
    void setRequiresImmediate(llvm::ArrayRef<int> Exacts) {
      Flags |= CI_ImmediateConstant;
      for (int Exact : Exacts)
        ImmSet.insert(Exact);
    }
    void setRequiresImmediate(int Exact) {
      Flags |= CI_ImmediateConstant;
      ImmSet.insert(Exact);
    }
    void setRequiresImmediate() {
      Flags |= CI_ImmediateConstant;
    }

    /// Indicate that this is an input operand that is tied to
    /// the specified output operand.
    ///
    /// Copy over the various constraint information from the output.
    void setTiedOperand(unsigned N, ConstraintInfo &Output) {
      Output.setHasMatchingInput();
      Flags = Output.Flags;
      TiedOperand = N;
      // Don't copy Name or constraint string.
    }
  };

  /// Validate register name used for global register variables.
  ///
  /// This function returns true if the register passed in RegName can be used
  /// for global register variables on this target. In addition, it returns
  /// true in HasSizeMismatch if the size of the register doesn't match the
  /// variable size passed in RegSize.
  virtual bool validateGlobalRegisterVariable(StringRef RegName,
                                              unsigned RegSize,
                                              bool &HasSizeMismatch) const {
    HasSizeMismatch = false;
    return true;
  }

  // validateOutputConstraint, validateInputConstraint - Checks that
  // a constraint is valid and provides information about it.
  // FIXME: These should return a real error instead of just true/false.
  bool validateOutputConstraint(ConstraintInfo &Info) const {
  const char *Name = Info.getConstraintStr().c_str();
  // An output constraint must start with '=' or '+'
  if (*Name != '=' && *Name != '+')
    return false;

  if (*Name == '+')
    Info.setIsReadWrite();

  Name++;
  while (*Name) {
    switch (*Name) {
    default:
      if (!validateAsmConstraint(Name, Info)) {
        // FIXME: We temporarily return false
        // so we can add more constraints as we hit it.
        // Eventually, an unknown constraint should just be treated as 'g'.
        return false;
      }
      break;
    case '&': // early clobber.
      Info.setEarlyClobber();
      break;
    case '%': // commutative.
      // FIXME: Check that there is a another register after this one.
      break;
    case 'r': // general register.
      Info.setAllowsRegister();
      break;
    case 'm': // memory operand.
    case 'o': // offsetable memory operand.
    case 'V': // non-offsetable memory operand.
    case '<': // autodecrement memory operand.
    case '>': // autoincrement memory operand.
      Info.setAllowsMemory();
      break;
    case 'g': // general register, memory operand or immediate integer.
    case 'X': // any operand.
      Info.setAllowsRegister();
      Info.setAllowsMemory();
      break;
    case ',': // multiple alternative constraint.  Pass it.
      // Handle additional optional '=' or '+' modifiers.
      if (Name[1] == '=' || Name[1] == '+')
        Name++;
      break;
    case '#': // Ignore as constraint.
      while (Name[1] && Name[1] != ',')
        Name++;
      break;
    case '?': // Disparage slightly code.
    case '!': // Disparage severely.
    case '*': // Ignore for choosing register preferences.
    case 'i': // Ignore i,n,E,F as output constraints (match from the other
              // chars)
    case 'n':
    case 'E':
    case 'F':
      break;  // Pass them.
    }

    Name++;
  }

  // Early clobber with a read-write constraint which doesn't permit registers
  // is invalid.
  if (Info.earlyClobber() && Info.isReadWrite() && !Info.allowsRegister())
    return false;

  // If a constraint allows neither memory nor register operands it contains
  // only modifiers. Reject it.
  return Info.allowsMemory() || Info.allowsRegister();
}

  bool validateInputConstraint(MutableArrayRef<ConstraintInfo> OutputConstraints,
                               ConstraintInfo &Info) const {
  const char *Name = Info.ConstraintStr.c_str();

  if (!*Name)
    return false;

  while (*Name) {
    switch (*Name) {
    default:
      // Check if we have a matching constraint
      if (*Name >= '0' && *Name <= '9') {
        const char *DigitStart = Name;
        while (Name[1] >= '0' && Name[1] <= '9')
          Name++;
        const char *DigitEnd = Name;
        unsigned i;
        if (StringRef(DigitStart, DigitEnd - DigitStart + 1)
                .getAsInteger(10, i))
          return false;

        // Check if matching constraint is out of bounds.
        if (i >= OutputConstraints.size()) return false;

        // A number must refer to an output only operand.
        if (OutputConstraints[i].isReadWrite())
          return false;

        // If the constraint is already tied, it must be tied to the
        // same operand referenced to by the number.
        if (Info.hasTiedOperand() && Info.getTiedOperand() != i)
          return false;

        // The constraint should have the same info as the respective
        // output constraint.
        Info.setTiedOperand(i, OutputConstraints[i]);
      } else if (!validateAsmConstraint(Name, Info)) {
        // FIXME: This error return is in place temporarily so we can
        // add more constraints as we hit it.  Eventually, an unknown
        // constraint should just be treated as 'g'.
        return false;
      }
      break;
    case '[': {
      unsigned Index = 0;
      if (!resolveSymbolicName(Name, OutputConstraints, Index))
        return false;

      // If the constraint is already tied, it must be tied to the
      // same operand referenced to by the number.
      if (Info.hasTiedOperand() && Info.getTiedOperand() != Index)
        return false;

      // A number must refer to an output only operand.
      if (OutputConstraints[Index].isReadWrite())
        return false;

      Info.setTiedOperand(Index, OutputConstraints[Index]);
      break;
    }
    case '%': // commutative
      // FIXME: Fail if % is used with the last operand.
      break;
    case 'i': // immediate integer.
      break;
    case 'n': // immediate integer with a known value.
      Info.setRequiresImmediate();
      break;
    case 'I':  // Various constant constraints with target-specific meanings.
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
      if (!validateAsmConstraint(Name, Info))
        return false;
      break;
    case 'r': // general register.
      Info.setAllowsRegister();
      break;
    case 'm': // memory operand.
    case 'o': // offsettable memory operand.
    case 'V': // non-offsettable memory operand.
    case '<': // autodecrement memory operand.
    case '>': // autoincrement memory operand.
      Info.setAllowsMemory();
      break;
    case 'g': // general register, memory operand or immediate integer.
    case 'X': // any operand.
      Info.setAllowsRegister();
      Info.setAllowsMemory();
      break;
    case 'E': // immediate floating point.
    case 'F': // immediate floating point.
    case 'p': // address operand.
      break;
    case ',': // multiple alternative constraint.  Ignore comma.
      break;
    case '#': // Ignore as constraint.
      while (Name[1] && Name[1] != ',')
        Name++;
      break;
    case '?': // Disparage slightly code.
    case '!': // Disparage severely.
    case '*': // Ignore for choosing register preferences.
      break;  // Pass them.
    }

    Name++;
  }

  return true;
}

  virtual bool validateOutputSize(const llvm::StringMap<bool> &FeatureMap,
                                  StringRef /*Constraint*/,
                                  unsigned /*Size*/) const {
    return true;
  }

  virtual bool validateInputSize(const llvm::StringMap<bool> &FeatureMap,
                                 StringRef /*Constraint*/,
                                 unsigned /*Size*/) const {
    return true;
  }
  virtual bool
  validateConstraintModifier(StringRef /*Constraint*/,
                             char /*Modifier*/,
                             unsigned /*Size*/,
                             std::string &/*SuggestedModifier*/) const {
    return true;
  }
  virtual bool
  validateAsmConstraint(const char *&Name,
                        ConstraintInfo &info) const = 0;

  bool resolveSymbolicName(const char *&Name,
                           ArrayRef<ConstraintInfo> OutputConstraints,
                           unsigned &Index) const {
  assert(*Name == '[' && "Symbolic name did not start with '['");
  Name++;
  const char *Start = Name;
  while (*Name && *Name != ']')
    Name++;

  if (!*Name) {
    // Missing ']'
    return false;
  }

  std::string SymbolicName(Start, Name - Start);

  for (Index = 0; Index != OutputConstraints.size(); ++Index)
    if (SymbolicName == OutputConstraints[Index].getName())
      return true;

  return false;
}

  // Constraint parm will be left pointing at the last character of
  // the constraint.  In practice, it won't be changed unless the
  // constraint is longer than one character.
  virtual std::string convertConstraint(const char *&Constraint) const {
    // 'p' defaults to 'r', but can be overridden by targets.
    if (*Constraint == 'p')
      return std::string("r");
    return std::string(1, *Constraint);
  }

  /// Replace some escaped characters with another string based on
  /// target-specific rules
  virtual llvm::Optional<std::string> handleAsmEscapedChar(char C) const {
    return llvm::None;
  }

  /// Returns a string of target-specific clobbers, in LLVM format.
  virtual const char *getClobbers() const = 0;

  /// Returns true if NaN encoding is IEEE 754-2008.
  /// Only MIPS allows a different encoding.
  virtual bool isNan2008() const {
    return true;
  }

  /// Returns the target triple of the primary target.
  const llvm::Triple &getTriple() const {
    return Triple;
  }

  /// Returns the target ID if supported.
  virtual llvm::Optional<std::string> getTargetID() const { return llvm::None; }

  const char *getDataLayoutString() const {
    assert(!DataLayoutString.empty() && "Uninitialized DataLayout!");
    return DataLayoutString.c_str();
  }

  struct GCCRegAlias {
    const char * const Aliases[5];
    const char * const Register;
  };

  struct AddlRegName {
    const char * const Names[5];
    const unsigned RegNum;
  };

  /// Does this target support "protected" visibility?
  ///
  /// Any target which dynamic libraries will naturally support
  /// something like "default" (meaning that the symbol is visible
  /// outside this shared object) and "hidden" (meaning that it isn't)
  /// visibilities, but "protected" is really an ELF-specific concept
  /// with weird semantics designed around the convenience of dynamic
  /// linker implementations.  Which is not to suggest that there's
  /// consistent target-independent semantics for "default" visibility
  /// either; the entire thing is pretty badly mangled.
  virtual bool hasProtectedVisibility() const { return true; }

  /// Does this target aim for semantic compatibility with
  /// Microsoft C++ code using dllimport/export attributes?
  virtual bool shouldDLLImportComdatSymbols() const {
    return getTriple().isWindowsMSVCEnvironment() ||
           getTriple().isWindowsItaniumEnvironment() || getTriple().isPS4();
  }

  // Does this target have PS4 specific dllimport/export handling?
  virtual bool hasPS4DLLImportExport() const {
    return getTriple().isPS4() ||
           // Windows Itanium support allows for testing the SCEI flavour of
           // dllimport/export handling on a Windows system.
           (getTriple().isWindowsItaniumEnvironment() &&
            getTriple().getVendor() == llvm::Triple::SCEI);
  }

  /// Set forced language options.
  ///
  /// Apply changes to the target information with respect to certain
  /// language options which change the target configuration and adjust
  /// the language based on the target options where applicable.
  virtual void adjust(/*DiagnosticsEngine &Diags, */LangOptions &Opts) {
  /*if (Opts.NoBitFieldTypeAlign)
    UseBitFieldTypeAlignment = false;*/

/*  switch (Opts.WCharSize) {
  default: llvm_unreachable("invalid wchar_t width");
  case 0: break;
  case 1: WCharType = Opts.WCharIsSigned ? SignedChar : UnsignedChar; break;
  case 2: WCharType = Opts.WCharIsSigned ? SignedShort : UnsignedShort; break;
  case 4: WCharType = Opts.WCharIsSigned ? SignedInt : UnsignedInt; break;
  }*/
  WCharType = UnsignedShort;
  /*if (Opts.AlignDouble) {
    DoubleAlign = LongLongAlign = 64;
    LongDoubleAlign = 64;
  }*/

/*  if (Opts.DoubleSize) {
    if (Opts.DoubleSize == 32) {
      DoubleWidth = 32;
      LongDoubleWidth = 32;
      DoubleFormat = &llvm::APFloat::IEEEsingle();
      LongDoubleFormat = &llvm::APFloat::IEEEsingle();
    } else if (Opts.DoubleSize == 64) {
      DoubleWidth = 64;
      LongDoubleWidth = 64;
      DoubleFormat = &llvm::APFloat::IEEEdouble();
      LongDoubleFormat = &llvm::APFloat::IEEEdouble();
    }
  }
*/
  /*
  if (Opts.LongDoubleSize) {
    if (Opts.LongDoubleSize == DoubleWidth) {
      LongDoubleWidth = DoubleWidth;
      LongDoubleAlign = DoubleAlign;
      LongDoubleFormat = DoubleFormat;
    } else if (Opts.LongDoubleSize == 128) {
      LongDoubleWidth = LongDoubleAlign = 128;
      LongDoubleFormat = &llvm::APFloat::IEEEquad();
    }
  }

  if (Opts.NewAlignOverride)
    NewAlign = Opts.NewAlignOverride * getCharWidth();
*/
  // Each unsigned fixed point type has the same number of fractional bits as
  // its corresponding signed type.
  //PaddingOnUnsignedFixedPoint |= Opts.PaddingOnUnsignedFixedPoint;
  //CheckFixedPointBits();
/*
  if (Opts.ProtectParens && !checkArithmeticFenceSupported()) {
    printf("diag::err_opt_not_valid_on_target) << \"-fprotect-parens\"");
    Opts.ProtectParens = false;
  }*/
}


  /// Adjust target options based on codegen options.
  virtual void adjustTargetOptions(/*const CodeGenOptions &CGOpts,*/
                                   TargetOptions &TargetOpts) const {}

  /// Initialize the map with the default set of target features for the
  /// CPU this should include all legal feature strings on the target.
  ///
  /// \return False on error (invalid features).
  virtual bool initFeatureMap(llvm::StringMap<bool> &Features,
                              StringRef CPU,
                              const std::vector<std::string> &FeatureVec) const {
  for (const auto &F : FeatureVec) {
    StringRef Name = F;
    // Apply the feature via the target.
    bool Enabled = Name[0] == '+';
    setFeatureEnabled(Features, Name.substr(1), Enabled);
  }
  return true;
}

  /// Get the ABI currently in use.
//  virtual StringRef getABI() const { return StringRef(); }

  /// Target the specified CPU.
  ///
  /// \return  False on error (invalid CPU name).
  virtual bool setCPU(const std::string &Name) {
    return false;
  }

  /// Fill a SmallVectorImpl with the valid values to setCPU.
  virtual void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {}

  /// Fill a SmallVectorImpl with the valid values for tuning CPU.
  virtual void fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values) const {
    fillValidCPUList(Values);
  }

  /// brief Determine whether this TargetInfo supports the given CPU name.
  virtual bool isValidCPUName(StringRef Name) const {
    return true;
  }

  /// brief Determine whether this TargetInfo supports the given CPU name for
  // tuning.
  virtual bool isValidTuneCPUName(StringRef Name) const {
    return isValidCPUName(Name);
  }

  /// brief Determine whether this TargetInfo supports tune in target attribute.
  virtual bool supportsTargetAttributeTune() const {
    return false;
  }


  /// Use the specified unit for FP math.
  ///
  /// \return False on error (invalid unit name).
  virtual bool setFPMath(StringRef Name) {
    return false;
  }

  /// Check if target has a given feature enabled
  virtual bool hasFeatureEnabled(const llvm::StringMap<bool> &Features,
                                 StringRef Name) const {
    return Features.lookup(Name);
  }

  /// Enable or disable a specific target feature;
  /// the feature name must be valid.
  virtual void setFeatureEnabled(llvm::StringMap<bool> &Features,
                                 StringRef Name,
                                 bool Enabled) const {
    Features[Name] = Enabled;
  }

  /// Determine whether this TargetInfo supports the given feature.
  virtual bool isValidFeatureName(StringRef Feature) const {
    return true;
  }

  struct BranchProtectionInfo {
/*    LangOptions::SignReturnAddressScopeKind SignReturnAddr =
        LangOptions::SignReturnAddressScopeKind::None;
    LangOptions::SignReturnAddressKeyKind SignKey =
        LangOptions::SignReturnAddressKeyKind::AKey;*/
    bool BranchTargetEnforcement = false;
  };

  /// Determine if the Architecture in this TargetInfo supports branch
  /// protection
  virtual bool isBranchProtectionSupportedArch(StringRef Arch) const {
    return false;
  }

  /// Determine if this TargetInfo supports the given branch protection
  /// specification
  virtual bool validateBranchProtection(StringRef Spec, StringRef Arch,
                                        BranchProtectionInfo &BPI,
                                        StringRef &Err) const {
    Err = "";
    return false;
  }

  /// Perform initialization based on the user configured
  /// set of features (e.g., +sse4).
  ///
  /// The list is guaranteed to have at most one entry per feature.
  ///
  /// The target may modify the features list, to change which options are
  /// passed onwards to the backend.
  /// FIXME: This part should be fixed so that we can change handleTargetFeatures
  /// to merely a TargetInfo initialization routine.
  ///
  /// \return  False on error.
  virtual bool handleTargetFeatures(std::vector<std::string> &Features/*,
                                    DiagnosticsEngine &Diags*/) {
    return true;
  }

  /// Determine whether the given target has the given feature.
  virtual bool hasFeature(StringRef Feature) const {
    return false;
  }

  /// Identify whether this target supports multiversioning of functions,
  /// which requires support for cpu_supports and cpu_is functionality.
  bool supportsMultiVersioning() const { return getTriple().isX86(); }

  /// Identify whether this target supports IFuncs.
  bool supportsIFunc() const { return getTriple().isOSBinFormatELF(); }

  // Validate the contents of the __builtin_cpu_supports(const char*)
  // argument.
  virtual bool validateCpuSupports(StringRef Name) const { return false; }

  // Return the target-specific priority for features/cpus/vendors so
  // that they can be properly sorted for checking.
  virtual unsigned multiVersionSortPriority(StringRef Name) const {
    return 0;
  }

  // Validate the contents of the __builtin_cpu_is(const char*)
  // argument.
  virtual bool validateCpuIs(StringRef Name) const { return false; }

  // Validate a cpu_dispatch/cpu_specific CPU option, which is a different list
  // from cpu_is, since it checks via features rather than CPUs directly.
  virtual bool validateCPUSpecificCPUDispatch(StringRef Name) const {
    return false;
  }

  // Get the character to be added for mangling purposes for cpu_specific.
  virtual char CPUSpecificManglingCharacter(StringRef Name) const {
    llvm_unreachable(
        "cpu_specific Multiversioning not implemented on this target");
  }

  // Get a list of the features that make up the CPU option for
  // cpu_specific/cpu_dispatch so that it can be passed to llvm as optimization
  // options.
  virtual void getCPUSpecificCPUDispatchFeatures(
      StringRef Name, llvm::SmallVectorImpl<StringRef> &Features) const {
    llvm_unreachable(
        "cpu_specific Multiversioning not implemented on this target");
  }

  // Get the cache line size of a given cpu. This method switches over
  // the given cpu and returns "None" if the CPU is not found.
  virtual Optional<unsigned> getCPUCacheLineSize() const { return None; }

  // Returns maximal number of args passed in registers.
  unsigned getRegParmMax() const {
    assert(RegParmMax < 7 && "RegParmMax value is larger than AST can handle");
    return RegParmMax;
  }

  /// Whether the target supports thread-local storage.
  bool isTLSSupported() const {
    return TLSSupported;
  }

  /// Return the maximum alignment (in bits) of a TLS variable
  ///
  /// Gets the maximum alignment (in bits) of a TLS variable on this target.
  /// Returns zero if there is no such constraint.
  unsigned getMaxTLSAlign() const { return MaxTLSAlign; }

  /// Whether target supports variable-length arrays.
  bool isVLASupported() const { return VLASupported; }

  /// Whether the target supports SEH __try.
  bool isSEHTrySupported() const {
    return getTriple().isOSWindows() &&
           (getTriple().isX86() ||
            getTriple().getArch() == llvm::Triple::aarch64);
  }

  /// Return true if {|} are normal characters in the asm string.
  ///
  /// If this returns false (the default), then {abc|xyz} is syntax
  /// that says that when compiling for asm variant #0, "abc" should be
  /// generated, but when compiling for asm variant #1, "xyz" should be
  /// generated.
  bool hasNoAsmVariants() const {
    return NoAsmVariants;
  }

  /// Return the register number that __builtin_eh_return_regno would
  /// return with the specified argument.
  /// This corresponds with TargetLowering's getExceptionPointerRegister
  /// and getExceptionSelectorRegister in the backend.
  virtual int getEHDataRegisterNumber(unsigned RegNo) const {
    return -1;
  }

  /// Return the section to use for C++ static initialization functions.
  virtual const char *getStaticInitSectionSpecifier() const {
    return nullptr;
  }
  /// Map from the address space field in builtin description strings to the
  /// language address space.
/*  virtual LangAS getOpenCLBuiltinAddressSpace(unsigned AS) const {
    return getLangASFromTargetAS(AS);
  }
*/
  /// Map from the address space field in builtin description strings to the
  /// language address space.
  /*virtual LangAS getCUDABuiltinAddressSpace(unsigned AS) const {
    return getLangASFromTargetAS(AS);
  }*/

  /// Return an AST address space which can be used opportunistically
  /// for constant global memory. It must be possible to convert pointers into
  /// this address space to LangAS::Default. If no such address space exists,
  /// this may return None, and such optimizations will be disabled.
  /*virtual llvm::Optional<LangAS> getConstantAddressSpace() const {
    return LangAS::Default;
  }*/

  // access target-specific GPU grid values that must be consistent between
  // host RTL (plugin), deviceRTL and clang.
  virtual const llvm::omp::GV &getGridValue() const {
    llvm_unreachable("getGridValue not implemented on this target");
  }

  /// Retrieve the name of the platform as it is used in the
  /// availability attribute.
  StringRef getPlatformName() const { return PlatformName; }

  /// Retrieve the minimum desired version of the platform, to
  /// which the program should be compiled.
  VersionTuple getPlatformMinVersion() const { return PlatformMinVersion; }

  bool isBigEndian() const { return BigEndian; }
  bool isLittleEndian() const { return !BigEndian; }

  /// Whether the option -fextend-arguments={32,64} is supported on the target.
  virtual bool supportsExtendIntArgs() const { return false; }

  /// Controls if __arithmetic_fence is supported in the targeted backend.
  virtual bool checkArithmeticFenceSupported() const { return false; }

  /// Gets the default calling convention for the given target and
  /// declaration context.
  virtual unsigned getDefaultCallingConv() const {
    // Not all targets will specify an explicit calling convention that we can
    // express.  This will always do the right thing, even though it's not
    // an explicit calling convention.
    return llvm::CallingConv::C;
  }

  enum CallingConvCheckResult {
    CCCR_OK,
    CCCR_Warning,
    CCCR_Ignore,
    CCCR_Error,
  };

  /// Determines whether a given calling convention is valid for the
  /// target. A calling convention can either be accepted, produce a warning
  /// and be substituted with the default calling convention, or (someday)
  /// produce an error (such as using thiscall on a non-instance function).
  virtual CallingConvCheckResult checkCallingConvention(llvm::CallingConv::ID CC) const {
    switch (CC) {
      default:
        return CCCR_Warning;
      case llvm::CallingConv::C:
        return CCCR_OK;
    }
  }

  enum CallingConvKind {
    CCK_Default,
    CCK_ClangABI4OrPS4,
    CCK_MicrosoftWin64
  };

  virtual CallingConvKind getCallingConvKind(bool ClangABICompat4) const {
  if (ClangABICompat4 || getTriple().getOS() == llvm::Triple::PS4)
    return CCK_ClangABI4OrPS4;
  return CCK_Default;
}

  /// Controls if __builtin_longjmp / __builtin_setjmp can be lowered to
  /// llvm.eh.sjlj.longjmp / llvm.eh.sjlj.setjmp.
  virtual bool hasSjLjLowering() const {
    return false;
  }

  /// Check if the target supports CFProtection branch.
  virtual bool
  checkCFProtectionBranchSupported(/*DiagnosticsEngine &Diags*/) const {
    printf("err_opt_not_valid_on_target: \"cf-protection=branch\";\n");
    return false;
}

  /// Check if the target supports CFProtection branch.
  virtual bool
  checkCFProtectionReturnSupported(/*DiagnosticsEngine &Diags*/) const {
    printf("err_opt_not_valid_on_target: \"cf-protection=return\";\n");
    return false;
  }

  /// Whether target allows to overalign ABI-specified preferred alignment
  virtual bool allowsLargerPreferedTypeAlignment() const { return true; }

  /// Whether target defaults to the `power` alignment rules of AIX.
  virtual bool defaultsToAIXPowerAlignment() const { return false; }

/*  /// Set supported OpenCL extensions and optional core features.
  virtual void setSupportedOpenCLOpts() {}

  virtual void supportAllOpenCLOpts(bool V = true) {
#define OPENCLEXTNAME(Ext)                                                     \
  setFeatureEnabled(getTargetOpts().OpenCLFeaturesMap, #Ext, V);
#include "OpenCLExtensions.def"
  }
*/
  /// Set supported OpenCL extensions as written on command line
  /*virtual void setCommandLineOpenCLOpts() {
    for (const auto &Ext : getTargetOpts().OpenCLExtensionsAsWritten) {
      bool IsPrefixed = (Ext[0] == '+' || Ext[0] == '-');
      std::string Name = IsPrefixed ? Ext.substr(1) : Ext;
      bool V = IsPrefixed ? Ext[0] == '+' : true;

      if (Name == "all") {
        supportAllOpenCLOpts(V);
        continue;
      }

      getTargetOpts().OpenCLFeaturesMap[Name] = V;
    }
  }
*/
  /// Get supported OpenCL extensions and optional core features.
  /*llvm::StringMap<bool> &getSupportedOpenCLOpts() {
    return getTargetOpts().OpenCLFeaturesMap;
  }*/

  /// Get const supported OpenCL extensions and optional core features.
  /*const llvm::StringMap<bool> &getSupportedOpenCLOpts() const {
    return getTargetOpts().OpenCLFeaturesMap;
  }*/

  /// Get address space for OpenCL type.
//  virtual LangAS getOpenCLTypeAddrSpace(OpenCLTypeKind TK) const;

  /// \returns Target specific vtbl ptr address space.
  /*virtual unsigned getVtblPtrAddressSpace() const {
    return 0;
  }*/

  /// \returns If a target requires an address within a target specific address
  /// space \p AddressSpace to be converted in order to be used, then return the
  /// corresponding target specific DWARF address space.
  ///
  /// \returns Otherwise return None and no conversion will be emitted in the
  /// DWARF.
  virtual Optional<unsigned> getDWARFAddressSpace(unsigned AddressSpace) const {
    return None;
  }

  /// \returns The version of the SDK which was used during the compilation if
  /// one was specified, or an empty version otherwise.
  const llvm::VersionTuple &getSDKVersion() const {
    return getTargetOpts().SDKVersion;
  }

  /// Check the target is valid after it is fully initialized.
  virtual bool validateTarget(/*DiagnosticsEngine &Diags*/) const {
    return true;
  }

  /// Check that OpenCL target has valid options setting based on OpenCL
  /// version.
/*  virtual bool validateOpenCLTarget(const LangOptions &Opts,
                                    DiagnosticsEngine &Diags) const;*/

  virtual void setAuxTarget(const TargetInfo *Aux) {}

  /// Whether target allows debuginfo types for decl only variables/functions.
  virtual bool allowDebugInfoForExternalRef() const { return false; }

protected:
  /// Copy type and layout related info.
  void copyAuxTarget(const TargetInfo *Aux)  {
    auto *Target = static_cast<TransferrableTargetInfo*>(this);
    auto *Src = static_cast<const TransferrableTargetInfo*>(Aux);
    *Target = *Src;
  }

  virtual uint64_t getPointerWidthV(unsigned AddrSpace) const {
    return PointerWidth;
  }
  virtual uint64_t getPointerAlignV(unsigned AddrSpace) const {
    return PointerAlign;
  }
  virtual enum IntType getPtrDiffTypeV(unsigned AddrSpace) const {
    return PtrDiffType;
  }
  virtual ArrayRef<const char *> getGCCRegNames() const = 0;
  virtual ArrayRef<GCCRegAlias> getGCCRegAliases() const = 0;
  virtual ArrayRef<AddlRegName> getGCCAddlRegNames() const {
    return None;
  }

 private:
  // Assert the values for the fractional and integral bits for each fixed point
  // type follow the restrictions given in clause 6.2.6.3 of N1169.
  void CheckFixedPointBits() const {
    // Check that the number of fractional and integral bits (and maybe sign) can
    // fit into the bits given for a fixed point type.
    assert(ShortAccumScale + getShortAccumIBits() + 1 <= ShortAccumWidth);
    assert(AccumScale + getAccumIBits() + 1 <= AccumWidth);
    assert(LongAccumScale + getLongAccumIBits() + 1 <= LongAccumWidth);
    assert(getUnsignedShortAccumScale() + getUnsignedShortAccumIBits() <=
           ShortAccumWidth);
    assert(getUnsignedAccumScale() + getUnsignedAccumIBits() <= AccumWidth);
    assert(getUnsignedLongAccumScale() + getUnsignedLongAccumIBits() <=
           LongAccumWidth);
  
    assert(getShortFractScale() + 1 <= ShortFractWidth);
    assert(getFractScale() + 1 <= FractWidth);
    assert(getLongFractScale() + 1 <= LongFractWidth);
    assert(getUnsignedShortFractScale() <= ShortFractWidth);
    assert(getUnsignedFractScale() <= FractWidth);
    assert(getUnsignedLongFractScale() <= LongFractWidth);
  
    // Each unsigned fract type has either the same number of fractional bits
    // as, or one more fractional bit than, its corresponding signed fract type.
    assert(getShortFractScale() == getUnsignedShortFractScale() ||
           getShortFractScale() == getUnsignedShortFractScale() - 1);
    assert(getFractScale() == getUnsignedFractScale() ||
           getFractScale() == getUnsignedFractScale() - 1);
    assert(getLongFractScale() == getUnsignedLongFractScale() ||
           getLongFractScale() == getUnsignedLongFractScale() - 1);
  
    // When arranged in order of increasing rank (see 6.3.1.3a), the number of
    // fractional bits is nondecreasing for each of the following sets of
    // fixed-point types:
    // - signed fract types
    // - unsigned fract types
    // - signed accum types
    // - unsigned accum types.
    assert(getLongFractScale() >= getFractScale() &&
           getFractScale() >= getShortFractScale());
    assert(getUnsignedLongFractScale() >= getUnsignedFractScale() &&
           getUnsignedFractScale() >= getUnsignedShortFractScale());
    assert(LongAccumScale >= AccumScale && AccumScale >= ShortAccumScale);
    assert(getUnsignedLongAccumScale() >= getUnsignedAccumScale() &&
           getUnsignedAccumScale() >= getUnsignedShortAccumScale());
  
    // When arranged in order of increasing rank (see 6.3.1.3a), the number of
    // integral bits is nondecreasing for each of the following sets of
    // fixed-point types:
    // - signed accum types
    // - unsigned accum types
    assert(getLongAccumIBits() >= getAccumIBits() &&
           getAccumIBits() >= getShortAccumIBits());
    assert(getUnsignedLongAccumIBits() >= getUnsignedAccumIBits() &&
           getUnsignedAccumIBits() >= getUnsignedShortAccumIBits());
  
    // Each signed accum type has at least as many integral bits as its
    // corresponding unsigned accum type.
    assert(getShortAccumIBits() >= getUnsignedShortAccumIBits());
    assert(getAccumIBits() >= getUnsignedAccumIBits());
    assert(getLongAccumIBits() >= getUnsignedLongAccumIBits());
}
};

namespace targets {

void DefineStd(MacroBuilder &Builder, StringRef MacroName, const LangOptions &Opts) {
  assert(MacroName[0] != '_' && "Identifier should be in the user's namespace");
  Builder.defineMacro("__" + MacroName);
  Builder.defineMacro("__" + MacroName + "__");
}

void defineCPUMacros(MacroBuilder &Builder, StringRef CPUName, bool Tuning = true) {
  Builder.defineMacro("__" + CPUName);
  Builder.defineMacro("__" + CPUName + "__");
  if (Tuning)
    Builder.defineMacro("__tune_" + CPUName + "__");
}

void addCygMingDefines(const LangOptions &Opts, MacroBuilder &Builder) {
  // Mingw and cygwin define __declspec(a) to __attribute__((a)).  Clang
  // supports __declspec natively under -fms-extensions, but we define a no-op
  // __declspec macro anyway for pre-processor compatibility.
  Builder.defineMacro("__declspec(a)", "__attribute__((a))");
  // Provide macros for all the calling convention keywords.  Provide both
  // single and double underscore prefixed variants.  These are available on
  // x64 as well as x86, even though they have no effect.
  const char *CCs[] = {"cdecl", "stdcall", "fastcall", "thiscall", "pascal"};
  for (const char *CC : CCs) {
    std::string GCCSpelling = "__attribute__((__";
    GCCSpelling += CC;
    GCCSpelling += "__))";
    Builder.defineMacro(Twine("_") + CC, GCCSpelling);
    Builder.defineMacro(Twine("__") + CC, GCCSpelling);
  }
}
TargetInfo *AllocateTarget(const llvm::Triple &Triple,
                           const TargetOptions &Opts);
#include "Targets/OSTargets.h"
#include "Targets/AArch64.h"
#include "Targets/AMDGPU.h"
#include "Targets/ARC.h"
#include "Targets/ARM.h"
#include "Targets/AVR.h"
#include "Targets/BPF.h"
#include "Targets/Hexagon.h"
#include "Targets/Lanai.h"
#include "Targets/Le64.h"
#include "Targets/M68k.h"
#include "Targets/MSP430.h"
#include "Targets/Mips.h"
#include "Targets/NVPTX.h"
#include "Targets/PNaCl.h"
#include "Targets/PPC.h"
#include "Targets/RISCV.h"
#include "Targets/SPIR.h"
#include "Targets/Sparc.h"
#include "Targets/SystemZ.h"
#include "Targets/TCE.h"
#include "Targets/VE.h"
#include "Targets/WebAssembly.h"
#include "Targets/X86.h"
#include "Targets/XCore.h"

#include "Targets/AArch64.cpp"
#include "Targets/AMDGPU.cpp"
#include "Targets/ARC.cpp"
#include "Targets/ARM.cpp"
#include "Targets/AVR.cpp"
#include "Targets/BPF.cpp"
#include "Targets/Hexagon.cpp"
#include "Targets/Lanai.cpp"
#include "Targets/Le64.cpp"
#include "Targets/M68k.cpp"
#include "Targets/MSP430.cpp"
#include "Targets/Mips.cpp"
#include "Targets/NVPTX.cpp"
#include "Targets/OSTargets.cpp"
#include "Targets/PNaCl.cpp"
#include "Targets/PPC.cpp"
#include "Targets/RISCV.cpp"
#include "Targets/SPIR.cpp"
#include "Targets/Sparc.cpp"
#include "Targets/SystemZ.cpp"
#include "Targets/TCE.cpp"
#include "Targets/VE.cpp"
#include "Targets/WebAssembly.cpp"
#include "Targets/X86.cpp"
#include "Targets/XCore.cpp"

//===----------------------------------------------------------------------===//
// Driver code
//===----------------------------------------------------------------------===//

TargetInfo *AllocateTarget(const llvm::Triple &Triple,
                           const TargetOptions &Opts) {
  llvm::Triple::OSType os = Triple.getOS();

  switch (Triple.getArch()) {
  default:
    return nullptr;

  case llvm::Triple::arc:
    return new ARCTargetInfo(Triple, Opts);

  case llvm::Triple::xcore:
    return new XCoreTargetInfo(Triple, Opts);

  case llvm::Triple::hexagon:
    if (os == llvm::Triple::Linux &&
        Triple.getEnvironment() == llvm::Triple::Musl)
      return new LinuxTargetInfo<HexagonTargetInfo>(Triple, Opts);
    return new HexagonTargetInfo(Triple, Opts);

  case llvm::Triple::lanai:
    return new LanaiTargetInfo(Triple, Opts);

  case llvm::Triple::aarch64_32:
    if (Triple.isOSDarwin())
      return new DarwinAArch64TargetInfo(Triple, Opts);

    return nullptr;
  case llvm::Triple::aarch64:
    if (Triple.isOSDarwin())
      return new DarwinAArch64TargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case llvm::Triple::GNU:
        return new MinGWARM64TargetInfo(Triple, Opts);
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftARM64TargetInfo(Triple, Opts);
      }
    default:
      return new AArch64leTargetInfo(Triple, Opts);
    }

  case llvm::Triple::aarch64_be:
    switch (os) {
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    default:
      return new AArch64beTargetInfo(Triple, Opts);
    }

  case llvm::Triple::arm:
  case llvm::Triple::thumb:
    if (Triple.isOSBinFormatMachO())
      return new DarwinARMTargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case llvm::Triple::Cygnus:
        return new CygwinARMTargetInfo(Triple, Opts);
      case llvm::Triple::GNU:
        return new MinGWARMTargetInfo(Triple, Opts);
      case llvm::Triple::Itanium:
        return new ItaniumWindowsARMleTargetInfo(Triple, Opts);
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftARMleTargetInfo(Triple, Opts);
      }
    default:
      return new ARMleTargetInfo(Triple, Opts);
    }

  case llvm::Triple::armeb:
  case llvm::Triple::thumbeb:
    if (Triple.isOSDarwin())
      return new DarwinARMTargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    default:
      return new ARMbeTargetInfo(Triple, Opts);
    }

  case llvm::Triple::avr:
    return new AVRTargetInfo(Triple, Opts);
  case llvm::Triple::bpfeb:
  case llvm::Triple::bpfel:
    return new BPFTargetInfo(Triple, Opts);

  case llvm::Triple::msp430:
    return new MSP430TargetInfo(Triple, Opts);

  case llvm::Triple::mips:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::mipsel:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<NaClMips32TargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::mips64:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::mips64el:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::m68k:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<M68kTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<M68kTargetInfo>(Triple, Opts);
    default:
      return new M68kTargetInfo(Triple, Opts);
    }

  case llvm::Triple::le32:
    switch (os) {
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<PNaClTargetInfo>(Triple, Opts);
    default:
      return nullptr;
    }

  case llvm::Triple::le64:
    return new Le64TargetInfo(Triple, Opts);

  case llvm::Triple::ppc:
    if (Triple.isOSDarwin())
      return new DarwinPPC32TargetInfo(Triple, Opts);
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::AIX:
      return new AIXPPC32TargetInfo(Triple, Opts);
    default:
      return new PPC32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::ppcle:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    default:
      return new PPC32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::ppc64:
    if (Triple.isOSDarwin())
      return new DarwinPPC64TargetInfo(Triple, Opts);
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::Lv2:
      return new PS3PPUTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::AIX:
      return new AIXPPC64TargetInfo(Triple, Opts);
    default:
      return new PPC64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::ppc64le:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    default:
      return new PPC64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::nvptx:
    return new NVPTXTargetInfo(Triple, Opts, /*TargetPointerWidth=*/32);
  case llvm::Triple::nvptx64:
    return new NVPTXTargetInfo(Triple, Opts, /*TargetPointerWidth=*/64);

  case llvm::Triple::amdgcn:
  case llvm::Triple::r600:
    return new AMDGPUTargetInfo(Triple, Opts);

  case llvm::Triple::riscv32:
    // TODO: add cases for NetBSD, RTEMS once tested.
    switch (os) {
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<RISCV32TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<RISCV32TargetInfo>(Triple, Opts);
    default:
      return new RISCV32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::riscv64:
    // TODO: add cases for NetBSD, RTEMS once tested.
    switch (os) {
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    default:
      return new RISCV64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::sparc:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    default:
      return new SparcV8TargetInfo(Triple, Opts);
    }

  // The 'sparcel' architecture copies all the above cases except for Solaris.
  case llvm::Triple::sparcel:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    default:
      return new SparcV8elTargetInfo(Triple, Opts);
    }

  case llvm::Triple::sparcv9:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    default:
      return new SparcV9TargetInfo(Triple, Opts);
    }

  case llvm::Triple::systemz:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SystemZTargetInfo>(Triple, Opts);
    case llvm::Triple::ZOS:
      return new ZOSTargetInfo<SystemZTargetInfo>(Triple, Opts);
    default:
      return new SystemZTargetInfo(Triple, Opts);
    }

  case llvm::Triple::tce:
    return new TCETargetInfo(Triple, Opts);

  case llvm::Triple::tcele:
    return new TCELETargetInfo(Triple, Opts);

  case llvm::Triple::x86:
    if (Triple.isOSDarwin())
      return new DarwinI386TargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::Ananas:
      return new AnanasTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux: {
      switch (Triple.getEnvironment()) {
      default:
        return new LinuxTargetInfo<X86_32TargetInfo>(Triple, Opts);
      case llvm::Triple::Android:
        return new AndroidX86_32TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::DragonFly:
      return new DragonFlyBSDTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDI386TargetInfo(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDI386TargetInfo(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::KFreeBSD:
      return new KFreeBSDTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Minix:
      return new MinixTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Win32: {
      switch (Triple.getEnvironment()) {
      case llvm::Triple::Cygnus:
        return new CygwinX86_32TargetInfo(Triple, Opts);
      case llvm::Triple::GNU:
        return new MinGWX86_32TargetInfo(Triple, Opts);
      case llvm::Triple::Itanium:
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftX86_32TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::Haiku:
      return new HaikuX86_32TargetInfo(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSX86_32TargetInfo(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::ELFIAMCU:
      return new MCUX86_32TargetInfo(Triple, Opts);
    case llvm::Triple::Hurd:
      return new HurdTargetInfo<X86_32TargetInfo>(Triple, Opts);
    default:
      return new X86_32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::x86_64:
    if (Triple.isOSDarwin() || Triple.isOSBinFormatMachO())
      return new DarwinX86_64TargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::Ananas:
      return new AnanasTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux: {
      switch (Triple.getEnvironment()) {
      default:
        return new LinuxTargetInfo<X86_64TargetInfo>(Triple, Opts);
      case llvm::Triple::Android:
        return new AndroidX86_64TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::DragonFly:
      return new DragonFlyBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDX86_64TargetInfo(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::KFreeBSD:
      return new KFreeBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Win32: {
      switch (Triple.getEnvironment()) {
      case llvm::Triple::Cygnus:
        return new CygwinX86_64TargetInfo(Triple, Opts);
      case llvm::Triple::GNU:
        return new MinGWX86_64TargetInfo(Triple, Opts);
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftX86_64TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::Haiku:
      return new HaikuTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::PS4:
      return new PS4OSTargetInfo<X86_64TargetInfo>(Triple, Opts);
    default:
      return new X86_64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::spir: {
    if (os != llvm::Triple::UnknownOS ||
        Triple.getEnvironment() != llvm::Triple::UnknownEnvironment)
      return nullptr;
    return new SPIR32TargetInfo(Triple, Opts);
  }
  case llvm::Triple::spir64: {
    if (os != llvm::Triple::UnknownOS ||
        Triple.getEnvironment() != llvm::Triple::UnknownEnvironment)
      return nullptr;
    return new SPIR64TargetInfo(Triple, Opts);
  }
  case llvm::Triple::spirv32: {
    if (os != llvm::Triple::UnknownOS ||
        Triple.getEnvironment() != llvm::Triple::UnknownEnvironment)
      return nullptr;
    return new SPIRV32TargetInfo(Triple, Opts);
  }
  case llvm::Triple::spirv64: {
    if (os != llvm::Triple::UnknownOS ||
        Triple.getEnvironment() != llvm::Triple::UnknownEnvironment)
      return nullptr;
    return new SPIRV64TargetInfo(Triple, Opts);
  }
  case llvm::Triple::wasm32:
    if (Triple.getSubArch() != llvm::Triple::NoSubArch ||
        Triple.getVendor() != llvm::Triple::UnknownVendor ||
        !Triple.isOSBinFormatWasm())
      return nullptr;
    switch (os) {
      case llvm::Triple::WASI:
        return new WASITargetInfo<WebAssembly32TargetInfo>(Triple, Opts);
      case llvm::Triple::Emscripten:
        return new EmscriptenTargetInfo<WebAssembly32TargetInfo>(Triple, Opts);
      case llvm::Triple::UnknownOS:
        return new WebAssemblyOSTargetInfo<WebAssembly32TargetInfo>(Triple, Opts);
      default:
        return nullptr;
    }
  case llvm::Triple::wasm64:
    if (Triple.getSubArch() != llvm::Triple::NoSubArch ||
        Triple.getVendor() != llvm::Triple::UnknownVendor ||
        !Triple.isOSBinFormatWasm())
      return nullptr;
    switch (os) {
      case llvm::Triple::WASI:
        return new WASITargetInfo<WebAssembly64TargetInfo>(Triple, Opts);
      case llvm::Triple::Emscripten:
        return new EmscriptenTargetInfo<WebAssembly64TargetInfo>(Triple, Opts);
      case llvm::Triple::UnknownOS:
        return new WebAssemblyOSTargetInfo<WebAssembly64TargetInfo>(Triple, Opts);
      default:
        return nullptr;
    }

  case llvm::Triple::renderscript32:
    return new LinuxTargetInfo<RenderScript32TargetInfo>(Triple, Opts);
  case llvm::Triple::renderscript64:
    return new LinuxTargetInfo<RenderScript64TargetInfo>(Triple, Opts);

  case llvm::Triple::ve:
    return new LinuxTargetInfo<VETargetInfo>(Triple, Opts);
  }
}
} // namespace targets

/// CreateTargetInfo - Return the target info object for the specified target
/// options.
TargetInfo *
TargetInfo::CreateTargetInfo(/*DiagnosticsEngine &Diags,*/
                             const std::shared_ptr<TargetOptions> &Opts) {
  llvm::Triple Triple(Opts->Triple);

  // Construct the target
  std::unique_ptr<TargetInfo> Target(targets::AllocateTarget(Triple, *Opts));
  if (!Target) {
    printf("unknown target triple '%s', please use -triple or -arch", Triple.str().data());
    return nullptr;
  }
  Target->TargetOpts = Opts;

  // Set the target CPU if specified.
  if (!Opts->CPU.empty() && !Target->setCPU(Opts->CPU)) {
    printf("diag::err_target_unknown_cpu) << Opts->CPU");
    SmallVector<StringRef, 32> ValidList;
    Target->fillValidCPUList(ValidList);
    if (!ValidList.empty())
      printf("diag::note_valid_options) << llvm::join(ValidList, \", \"\")");
    return nullptr;
  }

  // Check the TuneCPU name if specified.
  if (!Opts->TuneCPU.empty() &&
      !Target->isValidTuneCPUName(Opts->TuneCPU)) {
    printf("diag::err_target_unknown_cpu) << Opts->TuneCPU");
    SmallVector<StringRef, 32> ValidList;
    Target->fillValidTuneCPUList(ValidList);
    if (!ValidList.empty())
      printf("diag::note_valid_options) << llvm::join(ValidList, \", \")");
    return nullptr;
  }

  // Set the target ABI if specified.
/*  if (!Opts->ABI.empty() && !Target->setABI(Opts->ABI)) {
    printf("diag::err_target_unknown_abi) << Opts->ABI");
    return nullptr;
  }
*/
  // Set the fp math unit.
  if (!Opts->FPMath.empty() && !Target->setFPMath(Opts->FPMath)) {
    printf("diag::err_target_unknown_fpmath) << Opts->FPMath");
    return nullptr;
  }

  // Compute the default target features, we need the target to handle this
  // because features may have dependencies on one another.
  if (!Target->initFeatureMap(Opts->FeatureMap, Opts->CPU,
                              Opts->FeaturesAsWritten))
    return nullptr;

  // Add the features to the compile options.
  Opts->Features.clear();
  for (const auto &F : Opts->FeatureMap)
    Opts->Features.push_back((F.getValue() ? "+" : "-") + F.getKey().str());
  // Sort here, so we handle the features in a predictable order. (This matters
  // when we're dealing with features that overlap.)
  llvm::sort(Opts->Features);

  if (!Target->handleTargetFeatures(Opts->Features))
    return nullptr;

//  Target->setSupportedOpenCLOpts();
//  Target->setCommandLineOpenCLOpts();
  Target->setMaxAtomicWidth();

  if (!Target->validateTarget())
    return nullptr;

  Target->CheckFixedPointBits();

  return Target.release();
}
