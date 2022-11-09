ArrayRef<Builtin::Info> Le64TargetInfo::getTargetBuiltins() const {
  return {};
}

void Le64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "unix", Opts);
  defineCPUMacros(Builder, "le64", /*Tuning=*/false);
  Builder.defineMacro("__ELF__");
}
