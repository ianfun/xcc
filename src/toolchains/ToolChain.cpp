namespace driver {
  using namespace llvm::opt;
enum FileType {
  TY_INVALID,
  TY_Asm,
  TY_PP_Asm,
  TY_C,
  TY_PP_C,
  TY_CHeader,
  TY_LLVM_IR,
  TY_LLVM_BC,
};
FileType lookupTypeForExtension(StringRef Ext) {
  if (Ext.size() > 1) {
    return llvm::StringSwitch<FileType>(Ext)
    .Case("bc", TY_LLVM_BC)
    .Case("ll", TY_LLVM_IR)
    .Case("asm", TY_Asm)
    .Default(TY_INVALID);
  }
  switch (Ext[0]) {
    case 'c': return TY_C;
    case 'S': return TY_PP_Asm;
    case 'h': return TY_CHeader;
    case 's': return TY_Asm;
    default: return TY_INVALID;
  }
}
struct Driver;

struct Command {
  StringRef Program;
  SmallVector<StringRef> args;
  void addArg(StringRef arg) {
    args.push_back(arg);
  }
  int ExecuteAndWait(bool &Failed, std::string &ErrorMsg) {
    return llvm::sys::ExecuteAndWait(
        Program, 
        args, 
        llvm::None,
        {},
        0,
        0,
        &ErrorMsg,
        &Failed
    );
  }
  llvm::sys::ProcessInfo ExecuteNoWait(bool &Failed, std::string &ErrorMsg) {
    return llvm::sys::ExecuteNoWait(
        Program, 
        args, 
        llvm::None,
        {},
        0,
        &ErrorMsg,
        &Failed
    );
  }
};
struct ToolChain;

typedef void (*LinkerBuilder)(ToolChain &TC, Command &C);
typedef void (*AssemblerBuilder)(ToolChain &TC, Command &C);

struct ToolChain {
  llvm::Triple Triple;
  const Driver &D;
  const llvm::opt::ArgList &Args;
  AssemblerBuilder theAssembler;
  LinkerBuilder theLinker;
  ToolChain(const Driver &D, const llvm::Triple &T, const llvm::opt::ArgList &Args, AssemblerBuilder Assembler = buildAssembler, LinkerBuilder Linker = buildLinker)
  : Triple(T), D(D), Args(Args), theAssembler{Assembler},  theLinker{Linker} {}
  auto getLinker() const { return theLinker; };
  auto getAssembler() const { return theAssembler; };
  virtual void addSystemIncludes(llvm::SmallVectorImpl<const char*> paths) {};
  virtual ~ToolChain() {};
  ToolChain &getToolChain() { return *this; }
  const Driver &getDriver() const { return D; }
  static void buildLinker(ToolChain &TC, Command &C) {}
  static void buildAssembler(ToolChain &TC, Command &C) {}
  const llvm::Triple &getTriple() const { return Triple; }
  llvm::Triple::ArchType getArch() const { return Triple.getArch(); }
  StringRef getArchName() const { return Triple.getArchName(); }
  StringRef getPlatform() const { return Triple.getVendorName(); }
  StringRef getOS() const { return Triple.getOSName(); }
  std::string getTripleString() const {
    return Triple.getTriple();
  }
  virtual bool HasNativeLLVMSupport() { return false; };
  StringRef getDefaultUniversalArchName() const {
    switch (Triple.getArch()) {
    case llvm::Triple::aarch64: {
      if (getTriple().isArm64e())
        return "arm64e";
      return "arm64";
    }
    case llvm::Triple::aarch64_32:
      return "arm64_32";
    case llvm::Triple::ppc:
      return "ppc";
    case llvm::Triple::ppcle:
      return "ppcle";
    case llvm::Triple::ppc64:
      return "ppc64";
    case llvm::Triple::ppc64le:
      return "ppc64le";
    default:
      return Triple.getArchName();
    }
  }
  StringRef getOSLibName() const {
    if (Triple.isOSDarwin())
      return "darwin";
    switch (Triple.getOS()) {
    case llvm::Triple::FreeBSD:
      return "freebsd";
    case llvm::Triple::NetBSD:
      return "netbsd";
    case llvm::Triple::OpenBSD:
      return "openbsd";
    case llvm::Triple::Solaris:
      return "sunos";
    case llvm::Triple::AIX:
      return "aix";
    default:
      return getOS();
    }
  }
  bool isThreadModelSupported(const StringRef Model) const {
    if (Model == "single") {
      // FIXME: 'single' is only supported on ARM and WebAssembly so far.
      return Triple.getArch() == llvm::Triple::arm ||
             Triple.getArch() == llvm::Triple::armeb ||
             Triple.getArch() == llvm::Triple::thumb ||
             Triple.getArch() == llvm::Triple::thumbeb || Triple.isWasm();
    } else if (Model == "posix")
      return true;
  
    return false;
  }
  static std::string concat(StringRef Path, const Twine &A,
                                           const Twine &B, const Twine &C,
                                           const Twine &D) {
    SmallString<128> Result(Path);
    llvm::sys::path::append(Result, llvm::sys::path::Style::posix, A, B, C, D);
    return std::string(Result);
  } 
};
}
