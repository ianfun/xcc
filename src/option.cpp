#include <llvm/Support/CommandLine.h>

namespace detail {
using namespace llvm;
using namespace llvm::cl;

struct Options {
    std::string mainFileName;
    SmallString<256> CWD;
    opt<bool> g;
    opt<bool> libtrace;
    opt<std::string> triple;
    Options()
        : mainFileName("-"), g("g", desc("create debug information in output")),
          libtrace("libtrace", desc("use libtrace to obtain stack trace and some safe checks in runtime")),
          triple("triple", desc("specify target triple"), cl::init(llvm::sys::getDefaultTargetTriple())) {
        llvm::sys::fs::current_path(CWD);
    }
    bool run(int argc, const char *const *argv, StringRef Overview = "xcc: C code to LLVM IR Compiler\n") {
        return ParseCommandLineOptions(argc, argv, Overview);
    }
};

} // namespace detail

using detail::Options;
