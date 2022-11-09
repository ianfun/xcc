ArrayRef<const char *> PNaClTargetInfo::getGCCRegNames() const { return None; }

ArrayRef<TargetInfo::GCCRegAlias> PNaClTargetInfo::getGCCRegAliases() const {
  return None;
}

void PNaClTargetInfo::getArchDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  Builder.defineMacro("__le32__");
  Builder.defineMacro("__pnacl__");
}
