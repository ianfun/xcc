// build: clang++/g++ testXXX.cpp `llvm-config --cxxflags --ldflags --libs`

#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
//#include "../src/Driver/Driver.cpp"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/TargetSelect.h>

static llvm::cl::list<std::string> InputFiles(llvm::cl::Positional, llvm::cl::desc("<input files>"), llvm::cl::ZeroOrMore);

int main(int argc, const char **argv)
{
    llvm::SmallVector<const char *, 8> argv(argv_, argv_ + argc_);
    xcc::xcc_context ctx;

    XInitLLVM crashReport(ctx, argc, argv);
    llvm::InitializeAllTargets();

    xcc::Options options;
    Driver theDriver(ctx);

    if (!thePrinter.BuildCompilation(argv, options))
        return 1;

    xcc::SourceMgr SM(ctx);

    auto thePrinter = std::make_unique<xcc::TextDiagnosticPrinter>(SM);

    ctx.setPrinter(thePrinter.get());

    if (InputFiles.empty())
        return SM.fatal("no input files"), 1;

    for (const auto &str: InputFiles) {
        if (str.length() == 1 && str.front() == '-')
            SM.addStdin();
        else
            SM.addFile(str.c_str());
    }

    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllDisassemblers();

    llvm::LLVMContext llvmcontext;
    llvmcontext.setDiscardValueNames(true);
    llvmcontext.setOpaquePointers(true);
    llvmcontext.setDiagnosticHandler(std::make_unique<xcc::XCCDiagnosticHandler>());

    xcc::IRGen ig(ctx, SM, llvmcontext, options);

    xcc::Parser parser(SM, ctx, ig);

