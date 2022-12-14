namespace loongarch {
StringRef getLoongArchABI(const ArgList &Args,
                                     const llvm::Triple &Triple) {
  assert((Triple.getArch() == llvm::Triple::loongarch32 ||
          Triple.getArch() == llvm::Triple::loongarch64) &&
         "Unexpected triple");

  // If `-mabi=` is specified, use it.
  if (const Arg *A = Args.getLastArg(options::OPT_mabi_EQ))
    return A->getValue();

  // Choose a default based on the triple.
  // TODO: select appropiate ABI.
  return Triple.getArch() == llvm::Triple::loongarch32 ? "ilp32d" : "lp64d";
}

void getLoongArchTargetFeatures(const Driver &D,
                                           const llvm::Triple &Triple,
                                           const ArgList &Args,
                                           std::vector<StringRef> &Features) {
  // FIXME: hornor various clang options that may affect target features, e.g.
  // -march/-mtune/-mdouble-float/-msingle-float/-msoft-float/-mfpu. See:
  // https://loongson.github.io/LoongArch-Documentation/LoongArch-toolchain-conventions-EN.html
  Features.push_back("+f");
  Features.push_back("+d");
}
} // end namespace loongarch
