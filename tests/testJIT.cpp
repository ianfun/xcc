// the main program include file

#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"
#include <llvm/Support/Allocator.h>
#include <llvm/Support/StringSaver.h>

void myGetLine(std::string &line) {
    char c;
    ssize_t L;

    std::string str = "Enter command line arguments for main():\n> ";
    str += line;

    write(STDERR_FILENO, str.data(), str.size());

    for (;;) {
        L = read(STDIN_FILENO, &c, sizeof(char));
        if (L <= 0 || c == '\n' || c == '\r') break;
        line += c;
    }
}

int main(int argc_, const char **argv_)
{
    // make our text printer that print to stderr
    xcc::TextDiagnosticPrinter printer(llvm::errs()); 

    // create xcc_context
    xcc::xcc_context ctx {&printer};

    // create a crash report info
    XInitLLVM crashReport(ctx, argc_, argv_);

    // init args
    llvm::SmallVector<const char *, 8> argv(argv_, argv_ + argc_);
    
    // register targets
    llvm::InitializeAllTargets();

    // create the Driver
    xcc::driver::Driver theDriver(ctx);

    // XCC options
    xcc::Options options;

    // create SourceMgr for mangement source files
    xcc::SourceMgr SM(ctx);

    // set SourceMgr to the printer for printing source lines
    ctx.printer->setSourceMgr(&SM);

    // parse options ...
    if (theDriver.BuildCompilation(argv, options, SM))
        return 1;

    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllDisassemblers();

    // create LLVMContext - this will delete all modules when it deleted(dtor)
    auto llvmcontext = std::make_unique<llvm::LLVMContext>();
    llvmcontext->setDiscardValueNames(true);
    llvmcontext->setOpaquePointers(true);
    // set our DiagnosticHandler
    llvmcontext->setDiagnosticHandler(std::make_unique<xcc::XCCDiagnosticHandler>()); 

    // preparing target information and ready for code generation to LLVM IR
    xcc::IRGen ig(ctx, SM, *llvmcontext, options);

    // create parser
    xcc::Parser parser(SM, ctx, ig);

    // now, parsing source files ...
    size_t num_typedefs, num_tags;
    auto ast = parser.run(num_typedefs, num_tags);
    printer.finalize();
    if (ctx.printer->NumErrors)
        return 1;

    ig.run(ast, num_typedefs, num_tags);
    std::unique_ptr<llvm::Module> M;
    M.reset(ig.module);

    std::string cmdline = options.mainFileName;
    cmdline += ' ';

    myGetLine(cmdline);

    llvm::BumpPtrAllocator bumpAlloc;
    llvm::SmallVector<const char*, 4> newArgvs;
    llvm::StringSaver string_Saver(bumpAlloc);

#if WINDOWS
    llvm::cl::TokenizeGNUCommandLine(cmdline, string_Saver, newArgvs);
#else
    llvm::cl::TokenizeWindowsCommandLineFull(cmdline, string_Saver, newArgvs);
#endif

    auto JITE = xcc::JITRunner::Create();
    if (!JITE)
        return llvm::errs() << "cannot create JIT compiler and session:\n" << JITE.takeError(), 1;
    std::unique_ptr<xcc::JITRunner> TheJIT = std::move(*JITE);
    if (auto Error = TheJIT->addModule(llvm::orc::ThreadSafeModule(std::move(M), std::move(llvmcontext))))
        return llvm::errs() << "cannot add LLVM IR module:\n" << Error, 1;
    auto E = TheJIT->runMain(newArgvs.size(), newArgvs.data(), environ);
    if (!E)
        return llvm::errs() << "cannot run main function:\n" << E.takeError(), 1;
    return *E;
}
