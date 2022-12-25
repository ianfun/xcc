// the main program include file

#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"

int main(int argc_, const char **argv_)
{
    // create a DiagnosticsEngine for diagnostics
    xcc::DiagnosticsEngine engine;

    // make our text printer that print to stderr
    xcc::TextDiagnosticPrinter printer(llvm::errs()); 

    // add the printer to engine
    engine.addConsumer(&printer);

    // create a crash report info
    XInitLLVM crashReport(engine, argc_, argv_);

    // init args
    llvm::ArrayRef<const char*> argv(argv_, argv_ + argc_);
    
    // register targets
    llvm::InitializeAllTargets();

    // create the Driver
    xcc::driver::Driver theDriver(engine);

    // XCC options
    xcc::Options options;

    // create SourceMgr for mangement source files
    xcc::SourceMgr SM(engine);

    // set SourceMgr to the printer for printing source lines
    printer.setSourceMgr(&SM);

    int ret = 0;
    // parse options ...
    if (theDriver.BuildCompilation(argv, options, SM, ret))
        return ret;

    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllDisassemblers();

    options.createTarget(engine);

    SM.setTrigraphsEnabled(options.trigraphs);

    // create LLVMContext - this will delete all modules when it deleted(dtor)
    llvm::LLVMContext llvmcontext;

    llvmcontext.setDiscardValueNames(true);
    llvmcontext.setOpaquePointers(true);

    // set our DiagnosticHandler
    llvmcontext.setDiagnosticHandler(std::make_unique<xcc::XCCDiagnosticHandler>()); 

    // create xcc_context
    xcc::xcc_context ctx(options.DL, options.triple);

    // create xcc's type cache for LLVM
    xcc::LLVMTypeConsumer type_cache(llvmcontext, options);

    xcc::IRGen ig(SM, ctx, options, type_cache);

    // create parser
    xcc::Parser parser(SM, ctx, type_cache, options);

    // now, parsing source files ...
