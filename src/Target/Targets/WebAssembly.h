class LLVM_LIBRARY_VISIBILITY WebAssemblyTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

  enum SIMDEnum {
    NoSIMD,
    SIMD128,
    RelaxedSIMD,
  } SIMDLevel = NoSIMD;

  bool HasNontrappingFPToInt = false;
  bool HasSignExt = false;
  bool HasExceptionHandling = false;
  bool HasBulkMemory = false;
  bool HasAtomics = false;
  bool HasMutableGlobals = false;
  bool HasMultivalue = false;
  bool HasTailCall = false;
  bool HasReferenceTypes = false;

  std::string ABI;

public:
  explicit WebAssemblyTargetInfo(const llvm::Triple &T, const TargetOptions &)
      : TargetInfo(T) {
    NoAsmVariants = true;
    SuitableAlign = 128;
    LargeArrayMinWidth = 128;
    LargeArrayAlign = 128;
    SimdDefaultAlign = 128;
    SigAtomicType = SignedLong;
    LongDoubleWidth = LongDoubleAlign = 128;
    LongDoubleFormat = &llvm::APFloat::IEEEquad();
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    // size_t being unsigned long for both wasm32 and wasm64 makes mangled names
    // more consistent between the two.
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

private:
  static void setSIMDLevel(llvm::StringMap<bool> &Features, SIMDEnum Level,
                           bool Enabled);

  bool
  initFeatureMap(llvm::StringMap<bool> &Features,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override;
  bool hasFeature(StringRef Feature) const final;

  void setFeatureEnabled(llvm::StringMap<bool> &Features, StringRef Name,
                         bool Enabled) const final;

  bool handleTargetFeatures(std::vector<std::string> &Features) final;

  bool isValidCPUName(StringRef Name) const final;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const final;

  bool setCPU(const std::string &Name) final { return isValidCPUName(Name); }

  ArrayRef<Builtin::Info> getTargetBuiltins() const final;

  BuiltinVaListKind getBuiltinVaListKind() const final {
    return VoidPtrBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const final { return None; }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const final {
    return None;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const final {
    return false;
  }

  const char *getClobbers() const final { return ""; }

  bool isCLZForZeroUndef() const final { return false; }

  bool hasInt128Type() const final { return true; }

  IntType getIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // WebAssembly prefers long long for explicitly 64-bit integers.
    return BitWidth == 64 ? (IsSigned ? SignedLongLong : UnsignedLongLong)
                          : TargetInfo::getIntTypeByWidth(BitWidth, IsSigned);
  }

  IntType getLeastIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // WebAssembly uses long long for int_least64_t and int_fast64_t.
    return BitWidth == 64
               ? (IsSigned ? SignedLongLong : UnsignedLongLong)
               : TargetInfo::getLeastIntTypeByWidth(BitWidth, IsSigned);
  }

  CallingConvCheckResult checkCallingConvention(llvm::CallingConv::ID CC) const override {
    switch (CC) {
    case llvm::CallingConv::C:
    case llvm::CallingConv::Swift:
      return CCCR_OK;
    case llvm::CallingConv::SwiftAsync:
      return CCCR_Error;
    default:
      return CCCR_Warning;
    }
  }

  bool hasBitIntType() const override { return true; }

  bool hasProtectedVisibility() const override { return false; }

  void adjust(LangOptions &Opts) override;
};

class LLVM_LIBRARY_VISIBILITY WebAssembly32TargetInfo
    : public WebAssemblyTargetInfo {
public:
  explicit WebAssembly32TargetInfo(const llvm::Triple &T,
                                   const TargetOptions &Opts)
      : WebAssemblyTargetInfo(T, Opts) {
    if (T.isOSEmscripten())
      resetDataLayout("e-m:e-p:32:32-p10:8:8-p20:8:8-i64:64-f128:64-n32:64-"
                      "S128-ni:1:10:20");
    else
      resetDataLayout(
          "e-m:e-p:32:32-p10:8:8-p20:8:8-i64:64-n32:64-S128-ni:1:10:20");
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY WebAssembly64TargetInfo
    : public WebAssemblyTargetInfo {
public:
  explicit WebAssembly64TargetInfo(const llvm::Triple &T,
                                   const TargetOptions &Opts)
      : WebAssemblyTargetInfo(T, Opts) {
    LongAlign = LongWidth = 64;
    PointerAlign = PointerWidth = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    if (T.isOSEmscripten())
      resetDataLayout("e-m:e-p:64:64-p10:8:8-p20:8:8-i64:64-f128:64-n32:64-"
                      "S128-ni:1:10:20");
    else
      resetDataLayout(
          "e-m:e-p:64:64-p10:8:8-p20:8:8-i64:64-n32:64-S128-ni:1:10:20");
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};
