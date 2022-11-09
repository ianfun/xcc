class LLVM_LIBRARY_VISIBILITY BPFTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];
  bool HasAlu32 = false;

public:
  BPFTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    IntMaxType = SignedLong;
    Int64Type = SignedLong;
    RegParmMax = 5;
    if (Triple.getArch() == llvm::Triple::bpfeb) {
      resetDataLayout("E-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
    } else {
      resetDataLayout("e-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
    }
    MaxAtomicPromoteWidth = 64;
    MaxAtomicInlineWidth = 64;
    TLSSupported = false;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "bpf" || Feature == "alu32" || Feature == "dwarfris";
  }

  void setFeatureEnabled(llvm::StringMap<bool> &Features, StringRef Name,
                         bool Enabled) const override {
    Features[Name] = Enabled;
  }
  bool handleTargetFeatures(std::vector<std::string> &Features) override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  const char *getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  bool isValidGCCRegisterName(StringRef Name) const override { return true; }
  ArrayRef<const char *> getGCCRegNames() const override { return None; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      break;
    case 'w':
      if (HasAlu32) {
        Info.setAllowsRegister();
      }
      break;
    }
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  bool allowDebugInfoForExternalRef() const override { return true; }

  CallingConvCheckResult checkCallingConvention(llvm::CallingConv::ID CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case llvm::CallingConv::C:
    case llvm::CallingConv::OpenCLKernel:
      return CCCR_OK;
    }
  }

  bool isValidCPUName(StringRef Name) const override;

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    if (Name == "v3") {
      HasAlu32 = true;
    }

    StringRef CPUName(Name);
    return isValidCPUName(CPUName);
  }
};
