// ZOS.cpp - implements ZOS ToolChain class
struct ZOS : public ToolChain {
  ZOS(const Driver &D, const llvm::Triple &Triple,
      const llvm::opt::ArgList &Args) : ToolChain(D, Triple, Args) {}
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const llvm::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }
  bool IsIntegratedAssemblerDefault() const override { return true; }
  unsigned GetDefaultDwarfVersion() const override { return 4; }
};
