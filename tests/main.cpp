#define XCC_TOOLCHAIN
#define XCC_TARGET

#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"
#include <llvm/Support/Program.h>
#ifdef CC_HAS_LLD
    #include <lld/Common/Driver.h>
#endif

using namespace xcc::driver::options;

static int xcc_exit(int status = 0) {
    llvm::TimerGroup::printAll(llvm::errs());
    llvm::TimerGroup::clearAll();
    return status;
}

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

    // create a DiagnosticsEngine for diagnostics
    xcc::DiagnosticsEngine engine;

    // make our text printer that print to stderr
    xcc::TextDiagnosticPrinter printer(llvm::errs()); 

    // add the printer to engine
    engine.addConsumer(&printer);

    // create xcc_context
    xcc::xcc_context ctx;

    // create a crash report info
    XInitLLVM crashReport(engine, argc_, argv_);

    // init args
    llvm::ArrayRef<const char *> argv(argv_, size_t(argc_));
    
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

    int ret = CC_EXIT_SUCCESS;
    // parse options ...
    if (theDriver.BuildCompilation(argv, options, SM, ret))
        return ret;

    const auto &Args = theDriver.getArgs();

    if (Args.TimeTrace) {
        llvm::timeTraceProfilerInitialize(options.TimeTraceGranularity);
    }

    //auto tos = std::make_shared<xcc::TargetOptions>();

    //tos->Triple = options.triple.str();

    //auto theTarget = xcc::TargetInfo::CreateTargetInfo(tos);

    //printf("theTarget = %p\n", theTarget);
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
    
    if (Args.hasArg(OPT_dump_raw_tokens)) {
        auto &OS = llvm::errs();
        xcc::Lexer lexer(SM, nullptr, ctx, engine);
        xcc::TokenV tok;
        do {
            tok = lexer.lexAndLoc();
            tok.dump(OS);
            OS << '\n';
        } while (tok.tok != xcc::TEOF);
        return CC_EXIT_SUCCESS;
    }

    // preparing target information and ready for code generation to LLVM IR
    xcc::IRGen ig(ctx, engine, SM, *llvmcontext, options);

    // create parser
    xcc::Parser parser(SM, ig, engine, ctx);

    if (Args.hasArg(OPT_dump_tokens)) {
        auto &OS = llvm::errs();
        do {
            parser.l.cpp();
            parser.l.tok.dump(OS);
            OS << '\n';
        } while (parser.l.tok.tok != xcc::TEOF);
        return xcc_exit(CC_EXIT_SUCCESS);
    }

    // now, parsing source files ...
    size_t num_typedefs = 0, num_tags = 0;
    auto ast = parser.run(num_typedefs, num_tags);

    if (engine.getNumErrors())
        return xcc_exit(CC_EXIT_FAILURE);

    if (const auto *A = Args.getLastArg(OPT_ast_dump_EQ)) {
        llvm::StringRef dump = A->getValue();
        if (dump == "default") {
            xcc::dumpAst(ast, xcc::AST_Default);
        } else if (dump == "json") {
            xcc::dumpAst(ast, xcc::AST_JSON);
        } else {
            theDriver.error("invalid value %R in '-dump-ast'", dump);
        }
    }
    if (const auto *A = Args.getLastArg(OPT_ast_dump_all_EQ)) {
        llvm::StringRef dump = A->getValue();
        if (dump == "default") {
            xcc::dumpAst(ast, xcc::AST_Default);
        } else if (dump == "json") {
            xcc::dumpAst(ast, xcc::AST_JSON);
        } else {
            theDriver.error("invalid value %R in '-dump-ast-all'", dump);
        }
    }

    if (Args.hasArg(OPT_ast_dump) || Args.hasArg(OPT_ast_dump_all)) {
        xcc::dumpAst(ast, xcc::AST_Default);
    }

    if (Args.hasArg(OPT_fsyntax_only))
        return xcc_exit(CC_EXIT_SUCCESS);

    ig.run(ast, num_typedefs, num_tags);

    if (Args.hasArg(OPT_emit_codegen_only)) 
        return xcc_exit(CC_EXIT_SUCCESS);

    // Build ASTs and convert to LLVM, discarding output
    if (Args.hasArg(OPT_emit_llvm_only))
        return xcc_exit(CC_EXIT_SUCCESS);
    
    std::error_code EC;

    // Build ASTs then convert to LLVM, emit .bc file
    if (Args.hasArg(OPT_emit_llvm_bc)) {
        std::string outputFileName = options.mainFileName.str();
        outputFileName += ".bc";
        llvm::raw_fd_ostream OS(outputFileName, EC, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::FA_Write, llvm::sys::fs::OF_None);
        if (EC)
            goto CC_ERROR;
        dbgprint("bitcode writting to %s\n", outputFileName.c_str());
        llvm::WriteBitcodeToFile(*ig.module, OS);
        OS.close();
        return xcc_exit(CC_EXIT_SUCCESS);
    }

    // Use the LLVM representation for assembler and object files
    if (Args.hasArg(OPT_emit_llvm)) {
        std::string outputFileName = options.mainFileName.str();
        outputFileName += ".ll";
        llvm::raw_fd_ostream OS(outputFileName, EC, llvm::sys::fs::CD_CreateAlways, llvm::sys::fs::FA_Write, llvm::sys::fs::OF_None);
        if (EC)
            goto CC_ERROR;
        dbgprint("printing module to %s\n", outputFileName.data());
        ig.module->print(OS, nullptr);
        OS.close();
        return xcc_exit(CC_EXIT_SUCCESS);
    }

    {   // default - emit object file and linkning, or emit assembly
        xcc::driver::ToolChain TC(theDriver, options.triple, Args);
        bool assembly = Args.hasArg(OPT_S);
        std::string outputFileName = options.mainFileName.str();
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
            return xcc_exit(CC_EXIT_SUCCESS);

        // Only run preprocess, compile, and assemble steps
        if (Args.hasArg(OPT_C))
            return xcc_exit(CC_EXIT_SUCCESS);

        // the final phase - linking
        dbgprint("xcc_link(%s)\n", outputFileName.data());
        return xcc_exit(xcc_link(theDriver, outputFileName, options.triple));
    }

CC_ERROR:
    SM.error("cannot open output for writting: %R", EC.message());
    return xcc_exit(CC_EXIT_FAILURE);
}
