// the main program include file

#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"

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
    llvm::LLVMContext llvmcontext;
    llvmcontext.setDiscardValueNames(true);
    llvmcontext.setOpaquePointers(true);
    // set our DiagnosticHandler
    llvmcontext.setDiagnosticHandler(std::make_unique<xcc::XCCDiagnosticHandler>()); 

    // preparing target information and ready for code generation to LLVM IR
    xcc::IRGen ig(ctx, SM, llvmcontext, options);

    // create parser
    xcc::Parser parser(SM, ctx, ig);

    // now, parsing source files ...
