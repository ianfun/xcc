void TCETargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  DefineStd(Builder, "tce", Opts);
  Builder.defineMacro("__TCE__");
  Builder.defineMacro("__TCE_V1__");
}

void TCELETargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  DefineStd(Builder, "tcele", Opts);
  Builder.defineMacro("__TCE__");
  Builder.defineMacro("__TCE_V1__");
  Builder.defineMacro("__TCELE__");
  Builder.defineMacro("__TCELE_V1__");
}
