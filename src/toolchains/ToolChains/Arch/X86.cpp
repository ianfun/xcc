namespace x86 {
std::string getX86TargetCPU(const Driver &D, const ArgList &Args,
                                 const llvm::Triple &Triple) {
  if (const Arg *A = Args.getLastArg(OPT_march_EQ)) {
    StringRef CPU = A->getValue();
    if (CPU != "native")
      return std::string(CPU);

    // FIXME: Reject attempts to use -march=native unless the target matches
    // the host.
    CPU = llvm::sys::getHostCPUName();
    if (!CPU.empty() && CPU != "generic")
      return std::string(CPU);
  }

  if (const Arg *A = Args.getLastArg(OPT__SLASH_arch)) {
    // Mapping built by looking at lib/Basic's X86TargetInfo::initFeatureMap().
    // The keys are case-sensitive; this matches link.exe.
    // 32-bit and 64-bit /arch: flags.
    llvm::StringMap<StringRef> ArchMap({
        {"AVX", "sandybridge"},
        {"AVX2", "haswell"},
        {"AVX512F", "knl"},
        {"AVX512", "skylake-avx512"},
    });
    if (Triple.getArch() == llvm::Triple::x86) {
      // 32-bit-only /arch: flags.
      ArchMap.insert({
          {"IA32", "i386"},
          {"SSE", "pentium3"},
          {"SSE2", "pentium4"},
      });
    }
    StringRef CPU = ArchMap.lookup(A->getValue());
    if (CPU.empty()) {
      std::vector<StringRef> ValidArchs{ArchMap.keys().begin(),
                                        ArchMap.keys().end()};
      sort(ValidArchs);
      printf("diag::warn_drv_invalid_arch_name_with_suggestion)<< A->getValue() << (Triple.getArch() == llvm::Triple::x86)<< join(ValidArchs, \", \");")
    }
    return std::string(CPU);
  }

  // Select the default CPU if none was given (or detection failed).

  if (!Triple.isX86())
    return ""; // This routine is only handling x86 targets.

  bool Is64Bit = Triple.getArch() == llvm::Triple::x86_64;

  // FIXME: Need target hooks.
  if (Triple.isOSDarwin()) {
    if (Triple.getArchName() == "x86_64h")
      return "core-avx2";
    // macosx10.12 drops support for all pre-Penryn Macs.
    // Simulators can still run on 10.11 though, like Xcode.
    if (Triple.isMacOSX() && !Triple.isOSVersionLT(10, 12))
      return "penryn";

    if (Triple.isDriverKit())
      return "nehalem";

    // The oldest x86_64 Macs have core2/Merom; the oldest x86 Macs have Yonah.
    return Is64Bit ? "core2" : "yonah";
  }

  // Set up default CPU name for PS4/PS5 compilers.
  if (Triple.isPS4())
    return "btver2";
  if (Triple.isPS5())
    return "znver2";

  // On Android use targets compatible with gcc
  if (Triple.isAndroid())
    return Is64Bit ? "x86-64" : "i686";

  // Everything else goes to x86-64 in 64-bit mode.
  if (Is64Bit)
    return "x86-64";

  switch (Triple.getOS()) {
  case llvm::Triple::NetBSD:
    return "i486";
  case llvm::Triple::Haiku:
  case llvm::Triple::OpenBSD:
    return "i586";
  case llvm::Triple::FreeBSD:
    return "i686";
  default:
    // Fallback to p4.
    return "pentium4";
  }
}

void getX86TargetFeatures(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args,
                               std::vector<StringRef> &Features) {
  // If -march=native, autodetect the feature list.
  if (const Arg *A = Args.getLastArg(OPT_march_EQ)) {
    if (StringRef(A->getValue()) == "native") {
      llvm::StringMap<bool> HostFeatures;
      if (llvm::sys::getHostCPUFeatures(HostFeatures))
        for (auto &F : HostFeatures)
          Features.push_back(
              Args.MakeArgString((F.second ? "+" : "-") + F.first()));
    }
  }

  if (Triple.getArchName() == "x86_64h") {
    // x86_64h implies quite a few of the more modern subtarget features
    // for Haswell class CPUs, but not all of them. Opt-out of a few.
    Features.push_back("-rdrnd");
    Features.push_back("-aes");
    Features.push_back("-pclmul");
    Features.push_back("-rtm");
    Features.push_back("-fsgsbase");
  }

  const llvm::Triple::ArchType ArchType = Triple.getArch();
  // Add features to be compatible with gcc for Android.
  if (Triple.isAndroid()) {
    if (ArchType == llvm::Triple::x86_64) {
      Features.push_back("+sse4.2");
      Features.push_back("+popcnt");
      Features.push_back("+cx16");
    } else
      Features.push_back("+ssse3");
  }

  // Translate the high level `-mretpoline` flag to the specific target feature
  // flags. We also detect if the user asked for retpoline external thunks but
  // failed to ask for retpolines themselves (through any of the different
  // flags). This is a bit hacky but keeps existing usages working. We should
  // consider deprecating this and instead warn if the user requests external
  // retpoline thunks and *doesn't* request some form of retpolines.
  auto SpectreOpt = OPT_INVALID;
  if (Args.hasArgNoClaim(OPT_mretpoline, OPT_mno_retpoline,
                         OPT_mspeculative_load_hardening,
                         OPT_mno_speculative_load_hardening)) {
    if (Args.hasFlag(OPT_mretpoline, OPT_mno_retpoline,
                     false)) {
      Features.push_back("+retpoline-indirect-calls");
      Features.push_back("+retpoline-indirect-branches");
      SpectreOpt = OPT_mretpoline;
    } else if (Args.hasFlag(OPT_mspeculative_load_hardening,
                            OPT_mno_speculative_load_hardening,
                            false)) {
      // On x86, speculative load hardening relies on at least using retpolines
      // for indirect calls.
      Features.push_back("+retpoline-indirect-calls");
      SpectreOpt = OPT_mspeculative_load_hardening;
    }
  } else if (Args.hasFlag(OPT_mretpoline_external_thunk,
                          OPT_mno_retpoline_external_thunk, false)) {
    // FIXME: Add a warning about failing to specify `-mretpoline` and
    // eventually switch to an error here.
    Features.push_back("+retpoline-indirect-calls");
    Features.push_back("+retpoline-indirect-branches");
    SpectreOpt = OPT_mretpoline_external_thunk;
  }

  auto LVIOpt = OPT_INVALID;
  if (Args.hasFlag(OPT_mlvi_hardening, OPT_mno_lvi_hardening,
                   false)) {
    Features.push_back("+lvi-load-hardening");
    Features.push_back("+lvi-cfi"); // load hardening implies CFI protection
    LVIOpt = OPT_mlvi_hardening;
  } else if (Args.hasFlag(OPT_mlvi_cfi, OPT_mno_lvi_cfi,
                          false)) {
    Features.push_back("+lvi-cfi");
    LVIOpt = OPT_mlvi_cfi;
  }

  if (Args.hasFlag(OPT_m_seses, OPT_mno_seses, false)) {
    if (LVIOpt == OPT_mlvi_hardening)
      printf("diag::err_drv_argument_not_allowed_with) << D.getOpts().getOptionName(OPT_mlvi_hardening) << D.getOpts().getOptionName(OPT_m_seses);")

    if (SpectreOpt != OPT_INVALID)
      printf("diag::err_drv_argument_not_allowed_with) << D.getOpts().getOptionName(SpectreOpt) << D.getOpts().getOptionName(OPT_m_seses);")

    Features.push_back("+seses");
    if (!Args.hasArg(OPT_mno_lvi_cfi)) {
      Features.push_back("+lvi-cfi");
      LVIOpt = OPT_mlvi_cfi;
    }
  }

  if (SpectreOpt != ID::OPT_INVALID &&
      LVIOpt != ID::OPT_INVALID) {
    printf("diag::err_drv_argument_not_allowed_with)<< D.getOpts().getOptionName(SpectreOpt)<< D.getOpts().getOptionName(LVIOpt)");
  }

  // Now add any that the user explicitly requested on the command line,
  // which may override the defaults.
  for (const Arg *A : Args.filtered(OPT_m_x86_Features_Group,
                                    OPT_mgeneral_regs_only)) {
    StringRef Name = A->getOption().getName();
    A->claim();

    // Skip over "-m".
    assert(Name.startswith("m") && "Invalid feature name.");
    Name = Name.substr(1);

    // Replace -mgeneral-regs-only with -x87, -mmx, -sse
    if (A->getOption().getID() == OPT_mgeneral_regs_only) {
      Features.insert(Features.end(), {"-x87", "-mmx", "-sse"});
      continue;
    }

    bool IsNegative = Name.startswith("no-");
    if (IsNegative)
      Name = Name.substr(3);
    Features.push_back(Args.MakeArgString((IsNegative ? "-" : "+") + Name));
  }

  // Enable/disable straight line speculation hardening.
  if (Arg *A = Args.getLastArg(OPT_mharden_sls_EQ)) {
    StringRef Scope = A->getValue();
    if (Scope == "all") {
      Features.push_back("+harden-sls-ijmp");
      Features.push_back("+harden-sls-ret");
    } else if (Scope == "return") {
      Features.push_back("+harden-sls-ret");
    } else if (Scope == "indirect-jmp") {
      Features.push_back("+harden-sls-ijmp");
    } else if (Scope != "none") {
      printf("diag::err_drv_unsupported_option_argument A->getSpelling() << Scope");
    }
  }
}
} // end namespace x86
