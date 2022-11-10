namespace systemz {
enum class FloatABI {
  Soft,
  Hard,
};
enum FloatABI getSystemZFloatABI(const Driver &D,
                                              const ArgList &Args) {
  // Hard float is the default.
  FloatABI ABI = FloatABI::Hard;
  if (Args.hasArg(options::OPT_mfloat_abi_EQ))
    D.Diag(diag::err_drv_unsupported_opt)
      << Args.getLastArg(options::OPT_mfloat_abi_EQ)->getAsString(Args);

  if (Arg *A = Args.getLastArg(clang::driver::options::OPT_msoft_float,
                               options::OPT_mhard_float))
    if (A->getOption().matches(clang::driver::options::OPT_msoft_float))
      ABI = FloatABI::Soft;

  return ABI;
}

std::string getSystemZTargetCPU(const ArgList &Args) {
  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    llvm::StringRef CPUName = A->getValue();

    if (CPUName == "native") {
      std::string CPU = std::string(llvm::sys::getHostCPUName());
      if (!CPU.empty() && CPU != "generic")
        return CPU;
      else
        return "";
    }

    return std::string(CPUName);
  }
  return CLANG_SYSTEMZ_DEFAULT_ARCH;
}

void getSystemZTargetFeatures(const Driver &D, const ArgList &Args,
                                       std::vector<llvm::StringRef> &Features) {
  // -m(no-)htm overrides use of the transactional-execution facility.
  if (Arg *A = Args.getLastArg(options::OPT_mhtm, options::OPT_mno_htm)) {
    if (A->getOption().matches(options::OPT_mhtm))
      Features.push_back("+transactional-execution");
    else
      Features.push_back("-transactional-execution");
  }
  // -m(no-)vx overrides use of the vector facility.
  if (Arg *A = Args.getLastArg(options::OPT_mvx, options::OPT_mno_vx)) {
    if (A->getOption().matches(options::OPT_mvx))
      Features.push_back("+vector");
    else
      Features.push_back("-vector");
  }

  FloatABI FloatABI = getSystemZFloatABI(D, Args);
  if (FloatABI == FloatABI::Soft)
    Features.push_back("+soft-float");
}
} // end namespcace systemz
