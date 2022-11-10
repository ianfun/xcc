namespace sparc {
enum class FloatABI {
  Invalid,
  Soft,
  Hard,
};
const char *getSparcAsmModeForCPU(StringRef Name,
                                         const llvm::Triple &Triple) {
  if (Triple.getArch() == llvm::Triple::sparcv9) {
    const char *DefV9CPU;

    if (Triple.isOSLinux() || Triple.isOSFreeBSD() || Triple.isOSOpenBSD())
      DefV9CPU = "-Av9a";
    else
      DefV9CPU = "-Av9";

    return llvm::StringSwitch<const char *>(Name)
        .Case("niagara", "-Av9b")
        .Case("niagara2", "-Av9b")
        .Case("niagara3", "-Av9d")
        .Case("niagara4", "-Av9d")
        .Default(DefV9CPU);
  } else {
    return llvm::StringSwitch<const char *>(Name)
        .Case("v8", "-Av8")
        .Case("supersparc", "-Av8")
        .Case("sparclite", "-Asparclite")
        .Case("f934", "-Asparclite")
        .Case("hypersparc", "-Av8")
        .Case("sparclite86x", "-Asparclite")
        .Case("sparclet", "-Asparclet")
        .Case("tsc701", "-Asparclet")
        .Case("v9", "-Av8plus")
        .Case("ultrasparc", "-Av8plus")
        .Case("ultrasparc3", "-Av8plus")
        .Case("niagara", "-Av8plusb")
        .Case("niagara2", "-Av8plusb")
        .Case("niagara3", "-Av8plusd")
        .Case("niagara4", "-Av8plusd")
        .Case("ma2100", "-Aleon")
        .Case("ma2150", "-Aleon")
        .Case("ma2155", "-Aleon")
        .Case("ma2450", "-Aleon")
        .Case("ma2455", "-Aleon")
        .Case("ma2x5x", "-Aleon")
        .Case("ma2080", "-Aleon")
        .Case("ma2085", "-Aleon")
        .Case("ma2480", "-Aleon")
        .Case("ma2485", "-Aleon")
        .Case("ma2x8x", "-Aleon")
        .Case("myriad2", "-Aleon")
        .Case("myriad2.1", "-Aleon")
        .Case("myriad2.2", "-Aleon")
        .Case("myriad2.3", "-Aleon")
        .Case("leon2", "-Av8")
        .Case("at697e", "-Av8")
        .Case("at697f", "-Av8")
        .Case("leon3", "-Aleon")
        .Case("ut699", "-Av8")
        .Case("gr712rc", "-Aleon")
        .Case("leon4", "-Aleon")
        .Case("gr740", "-Aleon")
        .Default("-Av8");
  }
}

enum FloatABI getSparcFloatABI(const Driver &D,
                                        const ArgList &Args) {
  enum FloatABI ABI = FloatABI::Invalid;
  if (Arg *A = Args.getLastArg(clang::driver::options::OPT_msoft_float,
                               options::OPT_mhard_float,
                               options::OPT_mfloat_abi_EQ)) {
    if (A->getOption().matches(clang::driver::options::OPT_msoft_float))
      ABI = FloatABI::Soft;
    else if (A->getOption().matches(options::OPT_mhard_float))
      ABI = FloatABI::Hard;
    else {
      ABI = llvm::StringSwitch<FloatABI>(A->getValue())
                .Case("soft", FloatABI::Soft)
                .Case("hard", FloatABI::Hard)
                .Default(FloatABI::Invalid);
      if (ABI == FloatABI::Invalid &&
          !StringRef(A->getValue()).empty()) {
        D.Diag(clang::diag::err_drv_invalid_mfloat_abi) << A->getAsString(Args);
        ABI = FloatABI::Hard;
      }
    }
  }

  // If unspecified, choose the default based on the platform.
  // Only the hard-float ABI on Sparc is standardized, and it is the
  // default. GCC also supports a nonstandard soft-float ABI mode, also
  // implemented in LLVM. However as this is not standard we set the default
  // to be hard-float.
  if (ABI == FloatABI::Invalid) {
    ABI = FloatABI::Hard;
  }

  return ABI;
}

std::string getSparcTargetCPU(const Driver &D, const ArgList &Args,
                                     const llvm::Triple &Triple) {
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    D.Diag(diag::err_drv_unsupported_opt_for_target)
        << A->getSpelling() << Triple.getTriple();
    return "";
  }

  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_mcpu_EQ)) {
    StringRef CPUName = A->getValue();
    if (CPUName == "native") {
      std::string CPU = std::string(llvm::sys::getHostCPUName());
      if (!CPU.empty() && CPU != "generic")
        return CPU;
      return "";
    }
    return std::string(CPUName);
  }

  if (Triple.getArch() == llvm::Triple::sparc && Triple.isOSSolaris())
    return "v9";
  return "";
}

void getSparcTargetFeatures(const Driver &D, const ArgList &Args,
                                   std::vector<StringRef> &Features) {
  enum FloatABI FloatABI = getSparcFloatABI(D, Args);
  if (FloatABI == FloatABI::Soft)
    Features.push_back("+soft-float");
}
} // end namespace sparc
