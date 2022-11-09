class LLVM_LIBRARY_VISIBILITY AMDGPUTargetInfo final : public TargetInfo {

  static const Builtin::Info BuiltinInfo[];
  static const char *const GCCRegNames[];

  enum AddrSpace {
    Generic = 0,
    Global = 1,
    Local = 3,
    Constant = 4,
    Private = 5
  };
  llvm::AMDGPU::GPUKind GPUKind;
  unsigned GPUFeatures;
  unsigned WavefrontSize;

  /// Target ID is device name followed by optional feature name postfixed
  /// by plus or minus sign delimitted by colon, e.g. gfx908:xnack+:sramecc-.
  /// If the target ID contains feature+, map it to true.
  /// If the target ID contains feature-, map it to false.
  /// If the target ID does not contain a feature (default), do not map it.
  llvm::StringMap<bool> OffloadArchFeatures;
  std::string TargetID;

  bool hasFP64() const {
    return getTriple().getArch() == llvm::Triple::amdgcn ||
           !!(GPUFeatures & llvm::AMDGPU::FEATURE_FP64);
  }

  /// Has fast fma f32
  bool hasFastFMAF() const {
    return !!(GPUFeatures & llvm::AMDGPU::FEATURE_FAST_FMA_F32);
  }

  /// Has fast fma f64
  bool hasFastFMA() const {
    return getTriple().getArch() == llvm::Triple::amdgcn;
  }

  bool hasFMAF() const {
    return getTriple().getArch() == llvm::Triple::amdgcn ||
           !!(GPUFeatures & llvm::AMDGPU::FEATURE_FMA);
  }

  bool hasFullRateDenormalsF32() const {
    return !!(GPUFeatures & llvm::AMDGPU::FEATURE_FAST_DENORMAL_F32);
  }

  bool hasLDEXPF() const {
    return getTriple().getArch() == llvm::Triple::amdgcn ||
           !!(GPUFeatures & llvm::AMDGPU::FEATURE_LDEXP);
  }

  static bool isAMDGCN(const llvm::Triple &TT) {
    return TT.getArch() == llvm::Triple::amdgcn;
  }

  static bool isR600(const llvm::Triple &TT) {
    return TT.getArch() == llvm::Triple::r600;
  }

public:
  AMDGPUTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts);

  //void setAddressSpaceMap(bool DefaultIsPrivate);

  void adjust(LangOptions &Opts) override;

  uint64_t getPointerWidthV(unsigned AddrSpace) const override {
    if (isR600(getTriple()))
      return 32;

    if (AddrSpace == Private || AddrSpace == Local)
      return 32;

    return 64;
  }

  uint64_t getPointerAlignV(unsigned AddrSpace) const override {
    return getPointerWidthV(AddrSpace);
  }

  uint64_t getMaxPointerWidth() const override {
    return getTriple().getArch() == llvm::Triple::amdgcn ? 64 : 32;
  }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  /// Accepted register names: (n, m is unsigned integer, n < m)
  /// v
  /// s
  /// a
  /// {vn}, {v[n]}
  /// {sn}, {s[n]}
  /// {an}, {a[n]}
  /// {S} , where S is a special register name
  ////{v[n:m]}
  /// {s[n:m]}
  /// {a[n:m]}
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    static const ::llvm::StringSet<> SpecialRegs({
        "exec", "vcc", "flat_scratch", "m0", "scc", "tba", "tma",
        "flat_scratch_lo", "flat_scratch_hi", "vcc_lo", "vcc_hi", "exec_lo",
        "exec_hi", "tma_lo", "tma_hi", "tba_lo", "tba_hi",
    });

    switch (*Name) {
    case 'I':
      Info.setRequiresImmediate(-16, 64);
      return true;
    case 'J':
      Info.setRequiresImmediate(-32768, 32767);
      return true;
    case 'A':
    case 'B':
    case 'C':
      Info.setRequiresImmediate();
      return true;
    default:
      break;
    }

    StringRef S(Name);

    if (S == "DA" || S == "DB") {
      Name++;
      Info.setRequiresImmediate();
      return true;
    }

    bool HasLeftParen = false;
    if (S.front() == '{') {
      HasLeftParen = true;
      S = S.drop_front();
    }
    if (S.empty())
      return false;
    if (S.front() != 'v' && S.front() != 's' && S.front() != 'a') {
      if (!HasLeftParen)
        return false;
      auto E = S.find('}');
      if (!SpecialRegs.count(S.substr(0, E)))
        return false;
      S = S.drop_front(E + 1);
      if (!S.empty())
        return false;
      // Found {S} where S is a special register.
      Info.setAllowsRegister();
      Name = S.data() - 1;
      return true;
    }
    S = S.drop_front();
    if (!HasLeftParen) {
      if (!S.empty())
        return false;
      // Found s, v or a.
      Info.setAllowsRegister();
      Name = S.data() - 1;
      return true;
    }
    bool HasLeftBracket = false;
    if (!S.empty() && S.front() == '[') {
      HasLeftBracket = true;
      S = S.drop_front();
    }
    unsigned long long N;
    if (S.empty() || consumeUnsignedInteger(S, 10, N))
      return false;
    if (!S.empty() && S.front() == ':') {
      if (!HasLeftBracket)
        return false;
      S = S.drop_front();
      unsigned long long M;
      if (consumeUnsignedInteger(S, 10, M) || N >= M)
        return false;
    }
    if (HasLeftBracket) {
      if (S.empty() || S.front() != ']')
        return false;
      S = S.drop_front();
    }
    if (S.empty() || S.front() != '}')
      return false;
    S = S.drop_front();
    if (!S.empty())
      return false;
    // Found {vn}, {sn}, {an}, {v[n]}, {s[n]}, {a[n]}, {v[n:m]}, {s[n:m]}
    // or {a[n:m]}.
    Info.setAllowsRegister();
    Name = S.data() - 1;
    return true;
  }

  // \p Constraint will be left pointing at the last character of
  // the constraint.  In practice, it won't be changed unless the
  // constraint is longer than one character.
  std::string convertConstraint(const char *&Constraint) const override {

    StringRef S(Constraint);
    if (S == "DA" || S == "DB") {
      return std::string("^") + std::string(Constraint++, 2);
    }

    const char *Begin = Constraint;
    TargetInfo::ConstraintInfo Info("", "");
    if (validateAsmConstraint(Constraint, Info))
      return std::string(Begin).substr(0, Constraint - Begin + 1);

    Constraint = Begin;
    return std::string(1, *Constraint);
  }

  bool
  initFeatureMap(llvm::StringMap<bool> &Features,
                 StringRef CPU,
                 const std::vector<std::string> &FeatureVec) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  bool useFP16ConversionIntrinsics() const override { return false; }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  bool isValidCPUName(StringRef Name) const override {
    if (getTriple().getArch() == llvm::Triple::amdgcn)
      return llvm::AMDGPU::parseArchAMDGCN(Name) != llvm::AMDGPU::GK_NONE;
    return llvm::AMDGPU::parseArchR600(Name) != llvm::AMDGPU::GK_NONE;
  }

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    if (getTriple().getArch() == llvm::Triple::amdgcn) {
      GPUKind = llvm::AMDGPU::parseArchAMDGCN(Name);
      GPUFeatures = llvm::AMDGPU::getArchAttrAMDGCN(GPUKind);
    } else {
      GPUKind = llvm::AMDGPU::parseArchR600(Name);
      GPUFeatures = llvm::AMDGPU::getArchAttrR600(GPUKind);
    }

    return GPUKind != llvm::AMDGPU::GK_NONE;
  }
  const llvm::omp::GV &getGridValue() const override {
    switch (WavefrontSize) {
    case 32:
      return llvm::omp::getAMDGPUGridValues<32>();
    case 64:
      return llvm::omp::getAMDGPUGridValues<64>();
    default:
      llvm_unreachable("getGridValue not implemented for this wavesize");
    }
  }

  /// \returns If a target requires an address within a target specific address
  /// space \p AddressSpace to be converted in order to be used, then return the
  /// corresponding target specific DWARF address space.
  ///
  /// \returns Otherwise return None and no conversion will be emitted in the
  /// DWARF.
  Optional<unsigned>
  getDWARFAddressSpace(unsigned AddressSpace) const override {
    const unsigned DWARF_Private = 1;
    const unsigned DWARF_Local = 2;
    if (AddressSpace == Private) {
      return DWARF_Private;
    } else if (AddressSpace == Local) {
      return DWARF_Local;
    } else {
      return None;
    }
  }

  CallingConvCheckResult checkCallingConvention(llvm::CallingConv::ID CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case llvm::CallingConv::C:
    case llvm::CallingConv::OpenCLKernel:
      return CCCR_OK;
    }
  }

  // In amdgcn target the null pointer in global, constant, and generic
  // address space has value 0 but in private and local address space has
  // value ~0.
  uint64_t getNullPointerValue(LangAS AS) const override {
    // FIXME: Also should handle region.
    return (AS == LangAS::opencl_local || AS == LangAS::opencl_private)
      ? ~0 : 0;
  }

  void setAuxTarget(const TargetInfo *Aux) override;

  bool hasBitIntType() const override { return true; }

  // Record offload arch features since they are needed for defining the
  // pre-defined macros.
  static llvm::SmallVector<llvm::StringRef, 4> getAllPossibleTargetIDFeatures(const llvm::Triple &T,  llvm::StringRef Proc) { 
     llvm::SmallVector<llvm::StringRef, 4> Ret;
   auto ProcKind = T.isAMDGCN() ? llvm::AMDGPU::parseArchAMDGCN(Proc)
                                : llvm::AMDGPU::parseArchR600(Proc);
   if (ProcKind == llvm::AMDGPU::GK_NONE)
     return Ret;
   auto Features = T.isAMDGCN() ? llvm::AMDGPU::getArchAttrAMDGCN(ProcKind)
                                : llvm::AMDGPU::getArchAttrR600(ProcKind);
   if (Features & llvm::AMDGPU::FEATURE_SRAMECC)
     Ret.push_back("sramecc");
   if (Features & llvm::AMDGPU::FEATURE_XNACK)
     Ret.push_back("xnack");
   return Ret;
}
  bool handleTargetFeatures(std::vector<std::string> &Features) override {
    auto TargetIDFeatures =
        getAllPossibleTargetIDFeatures(getTriple(), getArchNameAMDGCN(GPUKind));
    llvm::for_each(Features, [&](const auto &F) {
      assert(F.front() == '+' || F.front() == '-');
      if (F == "+wavefrontsize64")
        WavefrontSize = 64;
      bool IsOn = F.front() == '+';
      StringRef Name = StringRef(F).drop_front();
      if (!llvm::is_contained(TargetIDFeatures, Name))
        return;
      assert(OffloadArchFeatures.find(Name) == OffloadArchFeatures.end());
      OffloadArchFeatures[Name] = IsOn;
    });
    return true;
  }
static std::string getCanonicalTargetID(llvm::StringRef Processor,
                                  const llvm::StringMap<bool> &Features) {
   std::string TargetID = Processor.str();
   std::map<const llvm::StringRef, bool> OrderedMap;
   for (const auto &F : Features)
     OrderedMap[F.first()] = F.second;
   for (auto F : OrderedMap)
     TargetID = TargetID + ':' + F.first.str() + (F.second ? "+" : "-");
   return TargetID;
 }
  Optional<std::string> getTargetID() const override {
    if (!isAMDGCN(getTriple()))
      return llvm::None;
    // When -target-cpu is not set, we assume generic code that it is valid
    // for all GPU and use an empty string as target ID to represent that.
    if (GPUKind == llvm::AMDGPU::GK_NONE)
      return std::string("");
    return getCanonicalTargetID(getArchNameAMDGCN(GPUKind),
                                OffloadArchFeatures);
  }
};
