class LLVM_LIBRARY_VISIBILITY SystemZTargetInfo : public TargetInfo {

  static const Builtin::Info BuiltinInfo[];
  static const char *const GCCRegNames[];
  std::string CPU;
  int ISARevision;
  bool HasTransactionalExecution;
  bool HasVector;
  bool SoftFloat;

public:
  SystemZTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple), CPU("z10"), ISARevision(8),
        HasTransactionalExecution(false), HasVector(false), SoftFloat(false) {
    IntMaxType = SignedLong;
    Int64Type = SignedLong;
    TLSSupported = true;
    IntWidth = IntAlign = 32;
    LongWidth = LongLongWidth = LongAlign = LongLongAlign = 64;
    PointerWidth = PointerAlign = 64;
    LongDoubleWidth = 128;
    LongDoubleAlign = 64;
    LongDoubleFormat = &llvm::APFloat::IEEEquad();
    DefaultAlignForAttributeAligned = 64;
    MinGlobalAlign = 16;
    if (Triple.isOSzOS()) {
      // All vector types are default aligned on an 8-byte boundary, even if the
      // vector facility is not available. That is different from Linux.
      MaxVectorAlign = 64;
      // Compared to Linux/ELF, the data layout differs only in some details:
      // - name mangling is GOFF
      // - 128 bit vector types are 64 bit aligned
      resetDataLayout(
          "E-m:l-i1:8:16-i8:8:16-i64:64-f128:64-v128:64-a:8:16-n32:64");
    } else
      resetDataLayout("E-m:e-i1:8:16-i8:8:16-i64:64-f128:64-a:8:16-n32:64");
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    HasStrictFP = true;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    // No aliases.
    return None;
  }

  ArrayRef<TargetInfo::AddlRegName> getGCCAddlRegNames() const override;

  bool isSPRegName(StringRef RegName) const override {
    return RegName.equals("r15");
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override;

  const char *getClobbers() const override {
    // FIXME: Is this really right?
    return "";
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::SystemZBuiltinVaList;
  }

  int getISARevision(StringRef Name) const;

  bool isValidCPUName(StringRef Name) const override {
    return getISARevision(Name) != -1;
  }

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    CPU = Name;
    ISARevision = getISARevision(CPU);
    return ISARevision != -1;
  }

  bool
  initFeatureMap(llvm::StringMap<bool> &Features,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override {
    int ISARevision = getISARevision(CPU);
    if (ISARevision >= 10)
      Features["transactional-execution"] = true;
    if (ISARevision >= 11)
      Features["vector"] = true;
    if (ISARevision >= 12)
      Features["vector-enhancements-1"] = true;
    if (ISARevision >= 13)
      Features["vector-enhancements-2"] = true;
    if (ISARevision >= 14)
      Features["nnp-assist"] = true;
    return TargetInfo::initFeatureMap(Features, CPU, FeaturesVec);
  }

  bool handleTargetFeatures(std::vector<std::string> &Features) override {
    HasTransactionalExecution = false;
    HasVector = false;
    SoftFloat = false;
    for (const auto &Feature : Features) {
      if (Feature == "+transactional-execution")
        HasTransactionalExecution = true;
      else if (Feature == "+vector")
        HasVector = true;
      else if (Feature == "+soft-float")
        SoftFloat = true;
    }
    HasVector &= !SoftFloat;

    // If we use the vector ABI, vector types are 64-bit aligned.
    if (HasVector && !getTriple().isOSzOS()) {
      MaxVectorAlign = 64;
      resetDataLayout("E-m:e-i1:8:16-i8:8:16-i64:64-f128:64"
                      "-v128:64-a:8:16-n32:64");
    }
    return true;
  }

  bool hasFeature(StringRef Feature) const override;

  CallingConvCheckResult checkCallingConvention(llvm::CallingConv::ID CC) const override {
    switch (CC) {
    case llvm::CallingConv::C:
    case llvm::CallingConv::Swift:
    case llvm::CallingConv::OpenCLKernel:
      return CCCR_OK;
    case llvm::CallingConv::SwiftAsync:
      return CCCR_Error;
    default:
      return CCCR_Warning;
    }
  }

  const char *getLongDoubleMangling() const override { return "g"; }

  bool hasBitIntType() const override { return true; }

  int getEHDataRegisterNumber(unsigned RegNo) const override {
    return RegNo < 4 ? 6 + RegNo : -1;
  }
};
