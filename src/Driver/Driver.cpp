namespace xcc {
namespace driver {

using namespace llvm::opt;

#include "XCCOptions.cpp"
using namespace options;

struct Driver: public DiagnosticHelper
{
std::string TargetTriple;
Driver(xcc_context &context, StringRef TargetTriple = llvm::sys::getDefaultTargetTriple()): 
  DiagnosticHelper(context),
  TargetTriple{TargetTriple} {}
std::unique_ptr<InputArgList> inputArgs;
static const llvm::opt::OptTable &getOpts() {
    return options::getDriverOptTable();
}
InputArgList &getArgs() {
  return *inputArgs;
}
static void PrintVersion(llvm::raw_ostream &OS) {
  OS << CC_VERSION_FULL << '\n';
}
static void PrintHelp(bool ShowHidden) {
  unsigned IncludedFlagsBitmask = 0;
  unsigned ExcludedFlagsBitmask = options::FlangOnlyOption | options::NoDriverOption;
  if (!ShowHidden)
    ExcludedFlagsBitmask |= HelpHidden;
  getOpts().printHelp(llvm::outs(), "xcc [options] file...", "XCC C compiler",
                      IncludedFlagsBitmask, ExcludedFlagsBitmask,
                      /*ShowAllAliases=*/false);
}
static llvm::Triple::ArchType getArchTypeForMachOArchName(StringRef Str) {
  // See arch(3) and llvm-gcc's driver-driver.c. We don't implement support for
  // archs which Darwin doesn't use.

  // The matching this routine does is fairly pointless, since it is neither the
  // complete architecture list, nor a reasonable subset. The problem is that
  // historically the driver driver accepts this and also ties its -march=
  // handling to the architecture name, so we need to be careful before removing
  // support for it.

  // This code must be kept in sync with Clang's Darwin specific argument
  // translation.

  return llvm::StringSwitch<llvm::Triple::ArchType>(Str)
      .Cases("ppc", "ppc601", "ppc603", "ppc604", "ppc604e", llvm::Triple::ppc)
      .Cases("ppc750", "ppc7400", "ppc7450", "ppc970", llvm::Triple::ppc)
      .Case("ppc64", llvm::Triple::ppc64)
      .Cases("i386", "i486", "i486SX", "i586", "i686", llvm::Triple::x86)
      .Cases("pentium", "pentpro", "pentIIm3", "pentIIm5", "pentium4",
             llvm::Triple::x86)
      .Cases("x86_64", "x86_64h", llvm::Triple::x86_64)
      // This is derived from the driver driver.
      .Cases("arm", "armv4t", "armv5", "armv6", "armv6m", llvm::Triple::arm)
      .Cases("armv7", "armv7em", "armv7k", "armv7m", llvm::Triple::arm)
      .Cases("armv7s", "xscale", llvm::Triple::arm)
      .Cases("arm64", "arm64e", llvm::Triple::aarch64)
      .Case("arm64_32", llvm::Triple::aarch64_32)
      .Case("r600", llvm::Triple::r600)
      .Case("amdgcn", llvm::Triple::amdgcn)
      .Case("nvptx", llvm::Triple::nvptx)
      .Case("nvptx64", llvm::Triple::nvptx64)
      .Case("amdil", llvm::Triple::amdil)
      .Case("spir", llvm::Triple::spir)
      .Default(llvm::Triple::UnknownArch);
}
static void setTripleTypeForMachOArchName(llvm::Triple &T, StringRef Str) {
  const llvm::Triple::ArchType Arch = getArchTypeForMachOArchName(Str);
  llvm::ARM::ArchKind ArchKind = llvm::ARM::parseArch(Str);
  T.setArch(Arch);
  if (Arch != llvm::Triple::UnknownArch)
    T.setArchName(Str);

  if (ArchKind == llvm::ARM::ArchKind::ARMV6M ||
      ArchKind == llvm::ARM::ArchKind::ARMV7M ||
      ArchKind == llvm::ARM::ArchKind::ARMV7EM) {
    T.setOS(llvm::Triple::UnknownOS);
    T.setObjectFormat(llvm::Triple::MachO);
  }
}

llvm::Triple computeTargetTriple(const Driver &D,
                                        StringRef TargetTriple,
                                        const ArgList &Args,
                                        StringRef DarwinArchName = "") {
  // FIXME: Already done in Compilation *Driver::BuildCompilation
  if (const Arg *A = Args.getLastArg(options::OPT_target))
    TargetTriple = A->getValue();

  llvm::Triple Target(llvm::Triple::normalize(TargetTriple));

  // GNU/Hurd's triples should have been -hurd-gnu*, but were historically made
  // -gnu* only, and we can not change this, so we have to detect that case as
  // being the Hurd OS.
  if (TargetTriple.contains("-unknown-gnu") || TargetTriple.contains("-pc-gnu"))
    Target.setOSName("hurd");

  // Handle Apple-specific options available here.
  if (Target.isOSBinFormatMachO()) {
    // If an explicit Darwin arch name is given, that trumps all.
    if (!DarwinArchName.empty()) {
      setTripleTypeForMachOArchName(Target, DarwinArchName);
      return Target;
    }

    // Handle the Darwin '-arch' flag.
    if (Arg *A = Args.getLastArg(options::OPT_arch)) {
      StringRef ArchName = A->getValue();
      setTripleTypeForMachOArchName(Target, ArchName);
    }
  }

  // Handle pseudo-target flags '-mlittle-endian'/'-EL' and
  // '-mbig-endian'/'-EB'.
  if (Arg *A = Args.getLastArg(options::OPT_mlittle_endian,
                               options::OPT_mbig_endian)) {
    if (A->getOption().matches(options::OPT_mlittle_endian)) {
      llvm::Triple LE = Target.getLittleEndianArchVariant();
      if (LE.getArch() != llvm::Triple::UnknownArch)
        Target = std::move(LE);
    } else {
      llvm::Triple BE = Target.getBigEndianArchVariant();
      if (BE.getArch() != llvm::Triple::UnknownArch)
        Target = std::move(BE);
    }
  }

  // Skip further flag support on OSes which don't support '-m32' or '-m64'.
  if (Target.getArch() == llvm::Triple::tce ||
      Target.getOS() == llvm::Triple::Minix)
    return Target;

  // On AIX, the env OBJECT_MODE may affect the resulting arch variant.
  if (Target.isOSAIX()) {
    if (llvm::Optional<std::string> ObjectModeValue =
            llvm::sys::Process::GetEnv("OBJECT_MODE")) {
      StringRef ObjectMode = *ObjectModeValue;
      llvm::Triple::ArchType AT = llvm::Triple::UnknownArch;

      if (ObjectMode.equals("64")) {
        AT = Target.get64BitArchVariant().getArch();
      } else if (ObjectMode.equals("32")) {
        AT = Target.get32BitArchVariant().getArch();
      } else {
        error("OBJECT_MODE setting %R is not recognized and is not a valid setting", ObjectMode);
      }

      if (AT != llvm::Triple::UnknownArch && AT != Target.getArch())
        Target.setArch(AT);
    }
  }

  // Handle pseudo-target flags '-m64', '-mx32', '-m32' and '-m16'.
  Arg *A = Args.getLastArg(options::OPT_m64, options::OPT_mx32,
                           options::OPT_m32, options::OPT_m16);
  if (A) {
    llvm::Triple::ArchType AT = llvm::Triple::UnknownArch;

    if (A->getOption().matches(options::OPT_m64)) {
      AT = Target.get64BitArchVariant().getArch();
      if (Target.getEnvironment() == llvm::Triple::GNUX32)
        Target.setEnvironment(llvm::Triple::GNU);
      else if (Target.getEnvironment() == llvm::Triple::MuslX32)
        Target.setEnvironment(llvm::Triple::Musl);
    } else if (A->getOption().matches(options::OPT_mx32) &&
               Target.get64BitArchVariant().getArch() == llvm::Triple::x86_64) {
      AT = llvm::Triple::x86_64;
      if (Target.getEnvironment() == llvm::Triple::Musl)
        Target.setEnvironment(llvm::Triple::MuslX32);
      else
        Target.setEnvironment(llvm::Triple::GNUX32);
    } else if (A->getOption().matches(options::OPT_m32)) {
      AT = Target.get32BitArchVariant().getArch();
      if (Target.getEnvironment() == llvm::Triple::GNUX32)
        Target.setEnvironment(llvm::Triple::GNU);
      else if (Target.getEnvironment() == llvm::Triple::MuslX32)
        Target.setEnvironment(llvm::Triple::Musl);
    } else if (A->getOption().matches(options::OPT_m16) &&
               Target.get32BitArchVariant().getArch() == llvm::Triple::x86) {
      AT = llvm::Triple::x86;
      Target.setEnvironment(llvm::Triple::CODE16);
    }

    if (AT != llvm::Triple::UnknownArch && AT != Target.getArch()) {
      Target.setArch(AT);
      if (Target.isWindowsGNUEnvironment())
        //toolchains::MinGW::fixTripleArch(D, Target, Args);
        ; /* XCC: TODO */
    }
  }

  // Handle -miamcu flag.
  if (Args.hasFlag(options::OPT_miamcu, options::OPT_mno_iamcu, false)) {
    if (Target.get32BitArchVariant().getArch() != llvm::Triple::x86)
      error("unsupported option '%s' for target '%R'",  "-miamcu", Target.str());
    if (A && !A->getOption().matches(options::OPT_m32))
      error("invalid argument '%s' not allowed with '%s'", "-miamcu", A->getBaseArg().getAsString(Args));
    Target.setArch(llvm::Triple::x86);
    Target.setArchName("i586");
    Target.setEnvironment(llvm::Triple::UnknownEnvironment);
    Target.setEnvironmentName("");
    Target.setOS(llvm::Triple::ELFIAMCU);
    Target.setVendor(llvm::Triple::UnknownVendor);
    Target.setVendorName("intel");
  }

  // If target is MIPS adjust the target triple
  // accordingly to provided ABI name.
  A = Args.getLastArg(options::OPT_mabi_EQ);
  if (A && Target.isMIPS()) {
    StringRef ABIName = A->getValue();
    if (ABIName == "32") {
      Target = Target.get32BitArchVariant();
      if (Target.getEnvironment() == llvm::Triple::GNUABI64 ||
          Target.getEnvironment() == llvm::Triple::GNUABIN32)
        Target.setEnvironment(llvm::Triple::GNU);
    } else if (ABIName == "n32") {
      Target = Target.get64BitArchVariant();
      if (Target.getEnvironment() == llvm::Triple::GNU ||
          Target.getEnvironment() == llvm::Triple::GNUABI64)
        Target.setEnvironment(llvm::Triple::GNUABIN32);
    } else if (ABIName == "64") {
      Target = Target.get64BitArchVariant();
      if (Target.getEnvironment() == llvm::Triple::GNU ||
          Target.getEnvironment() == llvm::Triple::GNUABIN32)
        Target.setEnvironment(llvm::Triple::GNUABI64);
    }
  }

  // If target is RISC-V adjust the target triple according to
  // provided architecture name
  A = Args.getLastArg(options::OPT_march_EQ);
  if (A && Target.isRISCV()) {
    StringRef ArchName = A->getValue();
    if (ArchName.startswith_insensitive("rv32"))
      Target.setArch(llvm::Triple::riscv32);
    else if (ArchName.startswith_insensitive("rv64"))
      Target.setArch(llvm::Triple::riscv64);
  }

  return Target;
}
bool HandleImmediateArgs() {
  if (getArgs().hasArg(options::OPT_dumpmachine)) {
    llvm::outs() << TargetTriple << '\n';
    return false;
  }
  if (getArgs().hasArg(options::OPT_dumpversion)) {
    llvm::outs() << CC_VERSION_FULL << '\n';
    return false;
  }

  if (getArgs().hasArg(options::OPT_help) ||
      getArgs().hasArg(options::OPT__help_hidden)) {
    PrintHelp(getArgs().hasArg(options::OPT__help_hidden));
    return false;
  }

  if (getArgs().hasArg(options::OPT__version)) {
    // Follow gcc behavior and use stdout for --version and stderr for -v.
    PrintVersion(llvm::outs());
    return false;
  }

  if (getArgs().hasArg(options::OPT_v) ||
      getArgs().hasArg(options::OPT__HASH_HASH_HASH) ||
      getArgs().hasArg(options::OPT_print_supported_cpus)) {
    PrintVersion(llvm::errs());
    return false;
  }

  if (getArgs().hasArg(options::OPT_print_search_dirs)) {
    llvm::outs() << "programs: =\nlibraries: =\n";
    return false;
  }

  if (getArgs().hasArg(options::OPT_print_target_triple)) {
    llvm::outs() << TargetTriple << "\n";
    return false;
  }

  if (getArgs().hasArg(options::OPT_print_targets)) {
    llvm::TargetRegistry::printRegisteredTargetsForVersion(llvm::outs());
    return false;
  }

  return true;
}
bool BuildInputs(SourceMgr &SM, Options &opts) {
  for (Arg *A : getArgs()) {
    if (A->getOption().getKind() == Option::InputClass) { 
      const char *Value = A->getValue();
      SM.addFile(Value);
    }
  }
  if (SM.empty())
    return fatal("no input files"), true;
  opts.mainFileName = SM.streams[SM.includeStack[0]].name;
  return false;
}
bool BuildCompilation(ArrayRef<const char *> Args, Options &opts, SourceMgr &SM, int &ret) {
  bool should_exit = false;
  ParseArgStrings(Args.slice(1), should_exit);
  if (Arg *WD = getArgs().getLastArg(options::OPT_working_directory))
    if (llvm::sys::fs::set_current_path(WD->getValue()))
      error("unable to set working directory: %s", WD->getValue());
  if (!HandleImmediateArgs())
    return true;
  opts.triple = computeTargetTriple(*this, TargetTriple, getArgs());
  std::string Error;
  opts.theTarget = llvm::TargetRegistry::lookupTarget(opts.triple.str(), Error);;
  if (!opts.theTarget) {
   error("unknown target triple %R, please use -triple or -arch", opts.triple.str());
   should_exit |= true;
  }
  opts.g = getArgs().hasArg(OPT_g_Flag);
  should_exit |= BuildInputs(SM, opts);
  ret = should_exit ? 1 : 0;
  return should_exit;
}
void ParseArgStrings(llvm::ArrayRef<const char *> ArgStrings, bool &ContainsError) {
  // clang/include/clang/Basic/DiagnosticDriverKinds.td
  ContainsError = false;
  unsigned IncludedFlagsBitmask = 0 /*options::CLOption | options::CoreOption*/;
  unsigned ExcludedFlagsBitmask = options::NoDriverOption;
  unsigned MissingArgIndex, MissingArgCount;
  inputArgs = std::make_unique<InputArgList>(getOpts().ParseArgs(ArgStrings, MissingArgIndex, MissingArgCount, IncludedFlagsBitmask, ExcludedFlagsBitmask));
  if (MissingArgCount) {
    error("argument to '%s' is missing (expected %u value)", 
        inputArgs->getArgString(MissingArgIndex),
        MissingArgCount
    );
    ContainsError |= true;
  }
  for (const Arg *A : *inputArgs) {
    if (A->getOption().hasFlag(options::Unsupported)) {
      auto ArgString = A->getAsString(*inputArgs);
      std::string Nearest;
      if (getOpts().findNearest(
            ArgString, Nearest, IncludedFlagsBitmask,
            ExcludedFlagsBitmask | options::Unsupported) > 1) {
        error("unsupported option %R\n", ArgString);
      } else {
        error("unsupported option %R; did you mean %R?", ArgString, Nearest);
      }
      ContainsError |= true;
      continue;
    }
    if (A->getOption().matches(options::OPT_mcpu_EQ) && A->containsValue("")) {
        error("joined argument expects additional value: %R", A->getAsString(*inputArgs));
      ContainsError |= false;
    }
  }
  for (const Arg *A : inputArgs->filtered(options::OPT_UNKNOWN)) {
    auto ArgString = A->getAsString(*inputArgs);
    std::string Nearest;
    if (getOpts().findNearest(
          ArgString, Nearest, IncludedFlagsBitmask, ExcludedFlagsBitmask) > 1) {
      error("unknown argument %R", ArgString);
    } else {
      error("unknown argument %R; did you mean %R?", ArgString, Nearest);
    }
    ContainsError |= false;
  }
}

};

}
}

