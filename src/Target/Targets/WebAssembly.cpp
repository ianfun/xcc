const Builtin::Info WebAssemblyTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},
#include "../BuiltinsWebAssembly.def"
};

static constexpr llvm::StringLiteral ValidCPUNames3[] = {
    {"mvp"}, {"bleeding-edge"}, {"generic"}};

bool WebAssemblyTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("simd128", SIMDLevel >= SIMD128)
      .Case("relaxed-simd", SIMDLevel >= RelaxedSIMD)
      .Case("nontrapping-fptoint", HasNontrappingFPToInt)
      .Case("sign-ext", HasSignExt)
      .Case("exception-handling", HasExceptionHandling)
      .Case("bulk-memory", HasBulkMemory)
      .Case("atomics", HasAtomics)
      .Case("mutable-globals", HasMutableGlobals)
      .Case("multivalue", HasMultivalue)
      .Case("tail-call", HasTailCall)
      .Case("reference-types", HasReferenceTypes)
      .Default(false);
}

bool WebAssemblyTargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::is_contained(ValidCPUNames3, Name);
}

void WebAssemblyTargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  Values.append(std::begin(ValidCPUNames3), std::end(ValidCPUNames3));
}

void WebAssemblyTargetInfo::getTargetDefines(const LangOptions &Opts,
                                             MacroBuilder &Builder) const {
  defineCPUMacros(Builder, "wasm", /*Tuning=*/false);
  if (SIMDLevel >= SIMD128)
    Builder.defineMacro("__wasm_simd128__");
  if (SIMDLevel >= RelaxedSIMD)
    Builder.defineMacro("__wasm_relaxed_simd__");
  if (HasNontrappingFPToInt)
    Builder.defineMacro("__wasm_nontrapping_fptoint__");
  if (HasSignExt)
    Builder.defineMacro("__wasm_sign_ext__");
  if (HasExceptionHandling)
    Builder.defineMacro("__wasm_exception_handling__");
  if (HasBulkMemory)
    Builder.defineMacro("__wasm_bulk_memory__");
  if (HasAtomics)
    Builder.defineMacro("__wasm_atomics__");
  if (HasMutableGlobals)
    Builder.defineMacro("__wasm_mutable_globals__");
  if (HasMultivalue)
    Builder.defineMacro("__wasm_multivalue__");
  if (HasTailCall)
    Builder.defineMacro("__wasm_tail_call__");
  if (HasReferenceTypes)
    Builder.defineMacro("__wasm_reference_types__");
}

void WebAssemblyTargetInfo::setSIMDLevel(llvm::StringMap<bool> &Features,
                                         SIMDEnum Level, bool Enabled) {
  if (Enabled) {
    switch (Level) {
    case RelaxedSIMD:
      Features["relaxed-simd"] = true;
      LLVM_FALLTHROUGH;
    case SIMD128:
      Features["simd128"] = true;
      LLVM_FALLTHROUGH;
    case NoSIMD:
      break;
    }
    return;
  }

  switch (Level) {
  case NoSIMD:
  case SIMD128:
    Features["simd128"] = false;
    LLVM_FALLTHROUGH;
  case RelaxedSIMD:
    Features["relaxed-simd"] = false;
    break;
  }
}

void WebAssemblyTargetInfo::setFeatureEnabled(llvm::StringMap<bool> &Features,
                                              StringRef Name,
                                              bool Enabled) const {
  if (Name == "simd128")
    setSIMDLevel(Features, SIMD128, Enabled);
  else if (Name == "relaxed-simd")
    setSIMDLevel(Features, RelaxedSIMD, Enabled);
  else
    Features[Name] = Enabled;
}

bool WebAssemblyTargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  if (CPU == "bleeding-edge") {
    Features["nontrapping-fptoint"] = true;
    Features["sign-ext"] = true;
    Features["bulk-memory"] = true;
    Features["atomics"] = true;
    Features["mutable-globals"] = true;
    Features["tail-call"] = true;
    setSIMDLevel(Features, SIMD128, true);
  }

  return TargetInfo::initFeatureMap(Features, CPU, FeaturesVec);
}

bool WebAssemblyTargetInfo::handleTargetFeatures(
    std::vector<std::string> &Features) {
  for (const auto &Feature : Features) {
    if (Feature == "+simd128") {
      SIMDLevel = std::max(SIMDLevel, SIMD128);
      continue;
    }
    if (Feature == "-simd128") {
      SIMDLevel = std::min(SIMDLevel, SIMDEnum(SIMD128 - 1));
      continue;
    }
    if (Feature == "+relaxed-simd") {
      SIMDLevel = std::max(SIMDLevel, RelaxedSIMD);
      continue;
    }
    if (Feature == "-relaxed-simd") {
      SIMDLevel = std::min(SIMDLevel, SIMDEnum(RelaxedSIMD - 1));
      continue;
    }
    if (Feature == "+nontrapping-fptoint") {
      HasNontrappingFPToInt = true;
      continue;
    }
    if (Feature == "-nontrapping-fptoint") {
      HasNontrappingFPToInt = false;
      continue;
    }
    if (Feature == "+sign-ext") {
      HasSignExt = true;
      continue;
    }
    if (Feature == "-sign-ext") {
      HasSignExt = false;
      continue;
    }
    if (Feature == "+exception-handling") {
      HasExceptionHandling = true;
      continue;
    }
    if (Feature == "-exception-handling") {
      HasExceptionHandling = false;
      continue;
    }
    if (Feature == "+bulk-memory") {
      HasBulkMemory = true;
      continue;
    }
    if (Feature == "-bulk-memory") {
      HasBulkMemory = false;
      continue;
    }
    if (Feature == "+atomics") {
      HasAtomics = true;
      continue;
    }
    if (Feature == "-atomics") {
      HasAtomics = false;
      continue;
    }
    if (Feature == "+mutable-globals") {
      HasMutableGlobals = true;
      continue;
    }
    if (Feature == "-mutable-globals") {
      HasMutableGlobals = false;
      continue;
    }
    if (Feature == "+multivalue") {
      HasMultivalue = true;
      continue;
    }
    if (Feature == "-multivalue") {
      HasMultivalue = false;
      continue;
    }
    if (Feature == "+tail-call") {
      HasTailCall = true;
      continue;
    }
    if (Feature == "-tail-call") {
      HasTailCall = false;
      continue;
    }
    if (Feature == "+reference-types") {
      HasReferenceTypes = true;
      continue;
    }
    if (Feature == "-reference-types") {
      HasReferenceTypes = false;
      continue;
    }

    printf("Diags.Report(diag::err_opt_not_valid_with_opt) << Feature << -target-feature");
    return false;
  }
  return true;
}

ArrayRef<Builtin::Info> WebAssemblyTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, WebAssembly::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}

void WebAssemblyTargetInfo::adjust(LangOptions &Opts) {
  // If the Atomics feature isn't available, turn off POSIXThreads and
  // ThreadModel, so that we don't predefine _REENTRANT or __STDCPP_THREADS__.
  if (!HasAtomics) {
    Opts.POSIXThreads = false;
    //Opts.setThreadModel(LangOptions::ThreadModelKind::Single);
    //Opts.ThreadsafeStatics = false;
  }
}

void WebAssembly32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                               MacroBuilder &Builder) const {
  WebAssemblyTargetInfo::getTargetDefines(Opts, Builder);
  defineCPUMacros(Builder, "wasm32", /*Tuning=*/false);
}

void WebAssembly64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                               MacroBuilder &Builder) const {
  WebAssemblyTargetInfo::getTargetDefines(Opts, Builder);
  defineCPUMacros(Builder, "wasm64", /*Tuning=*/false);
}
