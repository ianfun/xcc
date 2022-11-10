class LLVM_LIBRARY_VISIBILITY ZOS : public ToolChain {
public:

ZOS(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : ToolChain(D, Triple, Args) {}

~ZOS() {}

void addClangTargetOptions(const ArgList &DriverArgs,
                                ArgStringList &CC1Args,
                                Action::OffloadKind DeviceOffloadKind) const {
  // Pass "-faligned-alloc-unavailable" only when the user hasn't manually
  // enabled or disabled aligned allocations.
  if (!DriverArgs.hasArgNoClaim(options::OPT_faligned_allocation,
                                options::OPT_fno_aligned_allocation))
    CC1Args.push_back("-faligned-alloc-unavailable");
}
}
