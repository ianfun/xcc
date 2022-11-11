void getDarwinDefines(MacroBuilder &Builder, const LangOptions &Opts,
                      const llvm::Triple &Triple, StringRef &PlatformName,
                      VersionTuple &PlatformMinVersion) {
  Builder.defineMacro("__APPLE_CC__", "6000");
  Builder.defineMacro("__APPLE__");
  Builder.defineMacro("__STDC_NO_THREADS__");

  if (Opts.Static)
    Builder.defineMacro("__STATIC__");
  else
    Builder.defineMacro("__DYNAMIC__");

  if (Opts.POSIXThreads)
    Builder.defineMacro("_REENTRANT");

  // Get the platform type and version number from the triple.
  VersionTuple OsVersion;
  if (Triple.isMacOSX()) {
    Triple.getMacOSXVersion(OsVersion);
    PlatformName = "macos";
  } else {
    OsVersion = Triple.getOSVersion();
    PlatformName = llvm::Triple::getOSTypeName(Triple.getOS());
    if (PlatformName == "ios" && Triple.isMacCatalystEnvironment())
      PlatformName = "maccatalyst";
  }

  // If -target arch-pc-win32-macho option specified, we're
  // generating code for Win32 ABI. No need to emit
  // __ENVIRONMENT_XX_OS_VERSION_MIN_REQUIRED__.
  if (PlatformName == "win32") {
    PlatformMinVersion = OsVersion;
    return;
  }

  // Set the appropriate OS version define.
  if (Triple.isiOS()) {
    assert(OsVersion < VersionTuple(100) && "Invalid version!");
    char Str[7];
    if (OsVersion.getMajor() < 10) {
      Str[0] = '0' + OsVersion.getMajor();
      Str[1] = '0' + (OsVersion.getMinor().value_or(0) / 10);
      Str[2] = '0' + (OsVersion.getMinor().value_or(0) % 10);
      Str[3] = '0' + (OsVersion.getSubminor().value_or(0) / 10);
      Str[4] = '0' + (OsVersion.getSubminor().value_or(0) % 10);
      Str[5] = '\0';
    } else {
      // Handle versions >= 10.
      Str[0] = '0' + (OsVersion.getMajor() / 10);
      Str[1] = '0' + (OsVersion.getMajor() % 10);
      Str[2] = '0' + (OsVersion.getMinor().value_or(0) / 10);
      Str[3] = '0' + (OsVersion.getMinor().value_or(0) % 10);
      Str[4] = '0' + (OsVersion.getSubminor().value_or(0) / 10);
      Str[5] = '0' + (OsVersion.getSubminor().value_or(0) % 10);
      Str[6] = '\0';
    }
    if (Triple.isTvOS())
      Builder.defineMacro("__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__", Str);
    else
      Builder.defineMacro("__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__",
                          Str);

  } else if (Triple.isWatchOS()) {
    assert(OsVersion < VersionTuple(10) && "Invalid version!");
    char Str[6];
    Str[0] = '0' + OsVersion.getMajor();
    Str[1] = '0' + (OsVersion.getMinor().value_or(0) / 10);
    Str[2] = '0' + (OsVersion.getMinor().value_or(0) % 10);
    Str[3] = '0' + (OsVersion.getSubminor().value_or(0) / 10);
    Str[4] = '0' + (OsVersion.getSubminor().value_or(0) % 10);
    Str[5] = '\0';
    Builder.defineMacro("__ENVIRONMENT_WATCH_OS_VERSION_MIN_REQUIRED__", Str);
  } else if (Triple.isMacOSX()) {
    // Note that the Driver allows versions which aren't representable in the
    // define (because we only get a single digit for the minor and micro
    // revision numbers). So, we limit them to the maximum representable
    // version.
    assert(OsVersion < VersionTuple(100) && "Invalid version!");
    char Str[7];
    if (OsVersion < VersionTuple(10, 10)) {
      Str[0] = '0' + (OsVersion.getMajor() / 10);
      Str[1] = '0' + (OsVersion.getMajor() % 10);
      Str[2] = '0' + std::min(OsVersion.getMinor().value_or(0), 9U);
      Str[3] = '0' + std::min(OsVersion.getSubminor().value_or(0), 9U);
      Str[4] = '\0';
    } else {
      // Handle versions > 10.9.
      Str[0] = '0' + (OsVersion.getMajor() / 10);
      Str[1] = '0' + (OsVersion.getMajor() % 10);
      Str[2] = '0' + (OsVersion.getMinor().value_or(0) / 10);
      Str[3] = '0' + (OsVersion.getMinor().value_or(0) % 10);
      Str[4] = '0' + (OsVersion.getSubminor().value_or(0) / 10);
      Str[5] = '0' + (OsVersion.getSubminor().value_or(0) % 10);
      Str[6] = '\0';
    }
    Builder.defineMacro("__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__", Str);
  }

  // Tell users about the kernel if there is one.
  if (Triple.isOSDarwin())
    Builder.defineMacro("__MACH__");

  PlatformMinVersion = OsVersion;
}

static void addMinGWDefines(const llvm::Triple &Triple, const LangOptions &Opts,
                            MacroBuilder &Builder) {
  DefineStd(Builder, "WIN32", Opts);
  DefineStd(Builder, "WINNT", Opts);
  if (Triple.isArch64Bit()) {
    DefineStd(Builder, "WIN64", Opts);
    Builder.defineMacro("__MINGW64__");
  }
  Builder.defineMacro("__MSVCRT__");
  Builder.defineMacro("__MINGW32__");
  addCygMingDefines(Opts, Builder);
}

static void addVisualCDefines(const LangOptions &Opts, MacroBuilder &Builder) {
  /*if (Opts.CPlusPlus) {
    if (Opts.RTTIData)
      Builder.defineMacro("_CPPRTTI");

    if (Opts.CXXExceptions)
      Builder.defineMacro("_CPPUNWIND");
  }*/

  if (Opts.Bool)
    Builder.defineMacro("__BOOL_DEFINED");

  // FIXME: POSIXThreads isn't exactly the option this should be defined for,
  //        but it works for now.
  if (Opts.POSIXThreads)
    Builder.defineMacro("_MT");

  if (Opts.MicrosoftExt) {
    Builder.defineMacro("_MSC_EXTENSIONS");

    /*if (Opts.CPlusPlus11) {
      Builder.defineMacro("_RVALUE_REFERENCES_V2_SUPPORTED");
      Builder.defineMacro("_RVALUE_REFERENCES_SUPPORTED");
      Builder.defineMacro("_NATIVE_NULLPTR_SUPPORTED");
    }*/
  }

  Builder.defineMacro("_INTEGRAL_MAX_BITS", "64");
  Builder.defineMacro("__STDC_NO_THREADS__");

  // Starting with VS 2022 17.1, MSVC predefines the below macro to inform
  // users of the execution character set defined at compile time.
  // The value given is the Windows Code Page Identifier:
  // https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
  //
  // Clang currently only supports UTF-8, so we'll use 65001
  Builder.defineMacro("_MSVC_EXECUTION_CHARACTER_SET", "65001");
}

void addWindowsDefines(const llvm::Triple &Triple, const LangOptions &Opts,
                       MacroBuilder &Builder) {
  Builder.defineMacro("_WIN32");
  if (Triple.isArch64Bit())
    Builder.defineMacro("_WIN64");
  if (Triple.isWindowsGNUEnvironment())
    addMinGWDefines(Triple, Opts, Builder);
  else if (Triple.isKnownWindowsMSVCEnvironment() ||
           (Triple.isWindowsItaniumEnvironment() && Opts.MSVCCompat))
    addVisualCDefines(Opts, Builder);
}
