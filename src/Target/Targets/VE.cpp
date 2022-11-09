void VETargetInfo::getTargetDefines(const LangOptions &Opts,
                                    MacroBuilder &Builder) const {
  Builder.defineMacro("_LP64", "1");
  Builder.defineMacro("unix", "1");
  Builder.defineMacro("__unix__", "1");
  Builder.defineMacro("__linux__", "1");
  Builder.defineMacro("__ve", "1");
  Builder.defineMacro("__ve__", "1");
  Builder.defineMacro("__STDC_HOSTED__", "1");
  Builder.defineMacro("__STDC__", "1");
  Builder.defineMacro("__NEC__", "1");
  // FIXME: define __FAST_MATH__ 1 if -ffast-math is enabled
  // FIXME: define __OPTIMIZE__ n if -On is enabled
  // FIXME: define __VECTOR__ n 1 if automatic vectorization is enabled
}

ArrayRef<Builtin::Info> VETargetInfo::getTargetBuiltins() const {
  return ArrayRef<Builtin::Info>();
}
