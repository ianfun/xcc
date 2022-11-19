// the main program include file

#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"
#include <llvm/Support/Program.h>
#ifdef CC_HAS_LLD
    #include <lld/Common/Driver.h>
#endif

std::string xcc_getLinkerPath() {
#if WINDOWS
    auto gcc = llvm::sys::findProgramByName("gcc");
#else
    auto gcc = llvm::sys::findProgramByName("gcc.exe");
#endif
    if (gcc) {
        dbgprint("found gcc executable: %s\n", gcc->data());
        return *gcc;
    }
#if WINDOWS
    auto clang = llvm::sys::findProgramByName("clang");
#else
    auto clang = llvm::sys::findProgramByName("clang");
#endif
    if (clang) {
        dbgprint("found clang executable: %s\n", clang->data());
        return *clang;
    }
    dbgprint("no gcc or clang found! default to \"gcc\"\n");
    return "gcc"; // if not found, fall back to gcc
}

int xcc_link_gcc(xcc::DiagnosticHelper &Diags, llvm::StringRef objFileName) {
    std::string errorMsg;
    bool failed = false;
    std::string linker = xcc_getLinkerPath();
    int status = llvm::sys::ExecuteAndWait(
        linker,
        {llvm::StringRef(linker), objFileName},
        llvm::None,
        {},
        0,
        0,
        &errorMsg,
        &failed
    );
    if (failed) {
        Diags.error("failed to execute linker command: %R", errorMsg);
        return CC_EXIT_FAILURE;
    }
    else if (status) {
        Diags.error("linker command failed with exit code %i (use -v to see invocation)", status);
        return CC_EXIT_FAILURE;
    }
    return CC_EXIT_SUCCESS;
}

int xcc_link(xcc::DiagnosticHelper &Diags, llvm::StringRef objFileName, const llvm::Triple &theTriple) {
#ifdef CC_HAS_LLD
    llvm::SmallVector<const char *> args = {"ld.lld", objFileName.data()};
    bool status;
    // ld.lld (Unix), ld64.lld (macOS), lld-link (Windows), wasm-ld (WebAssembly)
    if (theTriple.isOSCygMing()) { /* isWindowsGNUEnvironment or isWindowsCygwinEnvironment */
        args[0] = "lld-link";
        dbgprint("linking: Mingw\n");
        status = lld::mingw::link(args, llvm::outs(), llvm::errs(), false, false);
    } else if (theTriple.isOSBinFormatCOFF()) {
        args[0] = "lld-link";
        dbgprint("linking: COFF\n");
        status = lld::coff::link(args, llvm::outs(), llvm::errs(), false, false);
    } else if (theTriple.isOSBinFormatELF()) {
        args[0] = "ld.lld";
        dbgprint("linking: ELF\n");
        status = lld::elf::link(args, llvm::outs(), llvm::errs(), false, false);
    } else if (theTriple.isOSBinFormatMachO()) {
        args[0] = "ld64.lld";
        dbgprint("linking: MachO\n");
        status = lld::macho::link(args, llvm::outs(), llvm::errs(), false, false);
    } else if (theTriple.isOSBinFormatWasm()) {
        args[0] = "wasm-ld";
        dbgprint("linking: WASM\n");
        status = lld::wasm::link(args, llvm::outs(), llvm::errs(), false, false);
    } else {
        dbgprint("linking: fallback to GCC\n");
        return xcc_link_gcc(Diags, objFileName);
    }
    if (status)
        return CC_EXIT_SUCCESS;
    Diags.error("lld linking failed");
    return CC_EXIT_FAILURE;
#else
    dbgprint("linking: GCC\n");
    return xcc_link_gcc(Diags, objFileName);
#endif
}

int main(int argc_, const char **argv_)
{
    assert(argc_ > 0 && "no program name!");
    assert(argv_ && "NULL argv!");
    // make our text printer that print to stderr
    xcc::TextDiagnosticPrinter printer(llvm::errs()); 

    // create xcc_context
    xcc::xcc_context ctx {&printer};

    // create a crash report info
    XInitLLVM crashReport(ctx, argc_, argv_);

    // init args
    llvm::ArrayRef<const char *> argv(argv_, size_t(argc_));
    
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

    int ret = CC_EXIT_SUCCESS;
    // parse options ...
    if (theDriver.BuildCompilation(argv, options, SM, ret))
        return ret;

    auto tos = std::make_shared<xcc::TargetOptions>();

    tos->Triple = options.triple.str();

    auto theTarget = xcc::TargetInfo::CreateTargetInfo(tos);

    printf("theTarget = %p\n", theTarget);
    //auto target_builtins = theTarget->getTargetBuiltins();
    //for (const auto &it : target_builtins) {
    //    printf("%s(%s)\n", it.Name, it.Type);
    //}

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
    size_t num_typedefs = 0, num_tags = 0;
    auto ast = parser.run(num_typedefs, num_tags);

    if (ctx.printer->NumErrors)
        return CC_EXIT_FAILURE;

    if (theDriver.getArgs().hasArg(xcc::driver::options::OPT_fsyntax_only))
        return CC_EXIT_SUCCESS;

    ig.run(ast, num_typedefs, num_tags);

    // Build ASTs and convert to LLVM, discarding output
    if (theDriver.getArgs().hasArg(xcc::driver::options::OPT_emit_llvm_only))
        return CC_EXIT_SUCCESS;
    
    std::error_code EC;

    // Build ASTs then convert to LLVM, emit .bc file
    if (theDriver.getArgs().hasArg(xcc::driver::options::OPT_emit_llvm_bc)) {
        std::string outputFileName = options.mainFileName;
        outputFileName += ".bc";
        llvm::raw_fd_ostream OS(outputFileName, EC, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::FA_Write, llvm::sys::fs::OF_None);
        if (EC)
            goto CC_ERROR;
        dbgprint("bitcode writting to %s\n", outputFileName.data());
        llvm::WriteBitcodeToFile(*ig.module, OS);
        OS.close();
        return CC_EXIT_SUCCESS;
    }

    // Use the LLVM representation for assembler and object files
    if (theDriver.getArgs().hasArg(xcc::driver::options::OPT_emit_llvm)) {
        std::string outputFileName = options.mainFileName;
        outputFileName += ".ll";
        llvm::raw_fd_ostream OS(outputFileName, EC, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::FA_Write, llvm::sys::fs::OF_None);
        if (EC)
            goto CC_ERROR;
        dbgprint("printing module to %s\n", outputFileName.data());
        ig.module->print(OS, nullptr);
        OS.close();
        return CC_EXIT_SUCCESS;
    }

    {   // default - emit object file and linkning, or emit assembly
        xcc::driver::ToolChain TC(theDriver, options.triple, theDriver.getArgs());
        bool assembly = theDriver.getArgs().hasArg(xcc::driver::options::OPT_S);
        std::string outputFileName = options.mainFileName;
        outputFileName += assembly ?  ".s" : ".o";
        llvm::raw_fd_ostream OS(outputFileName, EC, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::FA_Write, llvm::sys::fs::OF_None);
        dbgprint("creating object file: %s\n", outputFileName.data());
        if (EC)
            goto CC_ERROR;

        llvm::legacy::PassManager pass;
        ig.machine->addPassesToEmitFile(pass, OS, nullptr, assembly ? llvm::CGFT_AssemblyFile : llvm::CGFT_ObjectFile);
        pass.run(*ig.module);

        OS.close();

        if (assembly)
            return CC_EXIT_SUCCESS;

        // Only run preprocess, compile, and assemble steps
        if (theDriver.getArgs().hasArg(xcc::driver::options::OPT_C))
            return CC_EXIT_SUCCESS;

        // the final phase - linking
        dbgprint("xcc_link(%s)\n", outputFileName.data());
        return xcc_link(theDriver, outputFileName, options.triple);
    }

CC_ERROR:
    SM.error("cannot open output for writting: %R", EC.message());
    return CC_EXIT_FAILURE;
}
