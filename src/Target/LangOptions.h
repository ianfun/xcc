enum class Language : uint8_t {
   Unknown,
  
   /// Assembly: we accept this only so that we can preprocess it.
   Asm,
  
   /// LLVM IR: we accept this so that we can run the optimizer on it,
   /// and compile it to assembly or object code.
   LLVM_IR,
  
   ///@{ Languages that the frontend can parse and compile.
   C,
   CXX,
   ObjC,
   ObjCXX,
   OpenCL,
   OpenCLCXX,
   CUDA,
   RenderScript,
   HIP,
   HLSL,
   ///@}
 };
  
 enum LangFeatures {
   LineComment = (1 << 0),
   C99 = (1 << 1),
   C11 = (1 << 2),
   C17 = (1 << 3),
   C2x = (1 << 4),
   CPlusPlus = (1 << 5),
   CPlusPlus11 = (1 << 6),
   CPlusPlus14 = (1 << 7),
   CPlusPlus17 = (1 << 8),
   CPlusPlus20 = (1 << 9),
   CPlusPlus2b = (1 << 10),
   Digraphs = (1 << 11),
   GNUMode = (1 << 12),
   HexFloat = (1 << 13),
   OpenCL = (1 << 14),
   HLSL = (1 << 15)
 };
  
 /// LangStandard - Information about the properties of a particular language
 /// standard.
 struct LangStandard {
   enum Kind {
 #define LANGSTANDARD(id, name, lang, desc, features) \
     lang_##id,
 #include "LangStandards.def"
     lang_unspecified
   };
  
   const char *ShortName;
   const char *Description;
   unsigned Flags;
   Language Language;
  
 public:
   /// getName - Get the name of this standard.
   const char *getName() const { return ShortName; }
  
   /// getDescription - Get the description of this standard.
   const char *getDescription() const { return Description; }
  
   /// Get the language that this standard describes.
   enum Language getLanguage() const { return Language; }
  
   /// Language supports '//' comments.
   bool hasLineComments() const { return Flags & LineComment; }
  
   /// isC99 - Language is a superset of C99.
   bool isC99() const { return Flags & C99; }
  
   /// isC11 - Language is a superset of C11.
   bool isC11() const { return Flags & C11; }
  
   /// isC17 - Language is a superset of C17.
   bool isC17() const { return Flags & C17; }
  
   /// isC2x - Language is a superset of C2x.
   bool isC2x() const { return Flags & C2x; }
  
   /// isCPlusPlus - Language is a C++ variant.
   bool isCPlusPlus() const { return Flags & CPlusPlus; }
  
   /// isCPlusPlus11 - Language is a C++11 variant (or later).
   bool isCPlusPlus11() const { return Flags & CPlusPlus11; }
  
   /// isCPlusPlus14 - Language is a C++14 variant (or later).
   bool isCPlusPlus14() const { return Flags & CPlusPlus14; }
  
   /// isCPlusPlus17 - Language is a C++17 variant (or later).
   bool isCPlusPlus17() const { return Flags & CPlusPlus17; }
  
   /// isCPlusPlus20 - Language is a C++20 variant (or later).
   bool isCPlusPlus20() const { return Flags & CPlusPlus20; }
  
   /// isCPlusPlus2b - Language is a post-C++20 variant (or later).
   bool isCPlusPlus2b() const { return Flags & CPlusPlus2b; }
  
   /// hasDigraphs - Language supports digraphs.
   bool hasDigraphs() const { return Flags & Digraphs; }
  
   /// isGNUMode - Language includes GNU extensions.
   bool isGNUMode() const { return Flags & GNUMode; }
  
   /// hasHexFloats - Language supports hexadecimal float constants.
   bool hasHexFloats() const { return Flags & HexFloat; }
  
   /// isOpenCL - Language is a OpenCL variant.
   bool isOpenCL() const { return Flags & OpenCL; }
  
   static Kind getLangKind(StringRef Name);
   static const LangStandard &getLangStandardForKind(Kind K);
   static const LangStandard *getLangStandardForName(StringRef Name);
 };
  
 LangStandard::Kind getDefaultLanguageStandard(Language Lang,
                                               const llvm::Triple &T);

struct LangOptionsBase {

};

struct LangOptions : public LangOptionsBase {
  bool POSIXThreads: 1;
  bool C11: 1;
  bool C99: 1;
  bool C17: 1;
  bool C2x: 1;
  bool MSVCCompat: 1;
  bool MicrosoftExt: 1;
  bool AsmBlocks: 1;
  bool Borland: 1;
  bool EnableAIXExtendedAltivecABI: 1;
  bool WChar: 1;
  bool GNUMode: 1;
  bool UnsafeFPMath: 1;
  bool ShortEnums: 1;
  bool BranchTargetEnforcement: 1;
  bool ROPI: 1;
  bool RWPI: 1;
  bool HexagonQdsp6Compat: 1;
  bool Cmse: 1;
  bool ThreadsafeStatics: 1;
  bool CUDAIsDevice: 1;
  unsigned WCharSize: 4;
  unsigned VScaleMin: 32;
  unsigned VScaleMax: 32;
  unsigned MSCompatibilityVersion: 32;
  unsigned LongDoubleSize: 32;
  bool AltiVec: 1;
  bool IEEE128: 1;
  bool Static: 1;
  bool PPCIEEELongDouble: 1;
  bool Bool: 1;
  bool ZVector: 1;
};

