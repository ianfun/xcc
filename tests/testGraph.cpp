// the main program include file

#include "../src/xcc.h"
#include <llvm/Support/InitLLVM.h>
#include "../src/Driver/Driver.cpp"

int main(int argc_, const char **argv_)
{
    llvm::InitLLVM(argc_, argv_);

    xcc::TranslationUnit TU;
{
    // create a DiagnosticsEngine for diagnostics
    xcc::DiagnosticsEngine engine;

    // make our text printer that print to stderr
    xcc::TextDiagnosticPrinter printer(llvm::errs()); 

    // add the printer to engine
    engine.addConsumer(&printer);

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

    SM.setTrigraphsEnabled(options.trigraphs);

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

    // create xcc_context
    xcc::xcc_context ctx;

    // preparing target information and ready for code generation to LLVM IR
    xcc::IRGen ig(ctx, engine, SM, llvmcontext, options);

    // create xcc's type cache for LLVM
    xcc::LLVMTypeConsumer type_cache(ctx, llvmcontext, options.g);

    // configure IRGen to use it
    ig.setTypeConsumer(type_cache);

    // create parser
    xcc::Parser parser(SM, ig, engine, ctx, type_cache);

    // now, parsing source files ...

    parser.run(TU);

    printer.finalize();

    unsigned errs = engine.getNumErrors();

    if (errs)
        return 1;


    std::error_code EC;

    llvm::raw_fd_ostream OS("output.dot", EC, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::FA_Write, llvm::sys::fs::OF_None);

    TU.ast->writeGraph(OS, "AST");

    xcc::StmtReleaser().Visit(TU.ast);

    OS.close();

    llvm::errs() << "Output written to output.dot.\n";

    return 0;
}
}
