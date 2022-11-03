// program to test C preprocessor

#include "../src/xcc.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/TargetSelect.h>

static llvm::cl::list<std::string> InputFiles(llvm::cl::Positional, llvm::cl::desc("<input files>"), llvm::cl::ZeroOrMore);

static xcc::Options options;

int main(int argc, const char **argv)
{
    llvm::InitLLVM crashReport(argc, argv);
    
    if (!options.run(argc, argv))
        return 1;
    
    xcc::xcc_context ctx;

    xcc::SourceMgr SM(ctx);

    auto thePrinter = std::make_unique<xcc::TextDiagnosticPrinter>(SM);

    ctx.setPrinter(thePrinter.get());

    if (InputFiles.empty())
        return SM.fatal("no input files"), 1;

    for (const auto &str: InputFiles)
        SM.addFile(str.c_str());

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();

    auto llvmcontext = std::make_unique<llvm::LLVMContext>();

    llvmcontext->setDiscardValueNames(true);
    llvmcontext->setOpaquePointers(true);
    llvmcontext->setDiagnosticHandler(std::make_unique<xcc::XCCDiagnosticHandler>());

    xcc::IRGen ig(ctx, SM, *llvmcontext, options);

    xcc::Parser parser(SM, ctx, ig);

    unsigned num_typedefs, num_tags;
    auto ast = parser.run(num_typedefs, num_tags);
    if (ctx.printer->NumErrors)
        return 1;

    ig.run(ast, num_typedefs, num_tags);
    std::unique_ptr<llvm::Module> M;
    M.reset(ig.module);
    
    auto JITE = xcc::JITRunner::Create();
    if (!JITE)
        return llvm::errs() << "cannot create JIT compiler and session:\n" << JITE.takeError(), 1;
    std::unique_ptr<xcc::JITRunner> TheJIT = std::move(*JITE);
    if (auto Error = TheJIT->addModule(llvm::orc::ThreadSafeModule(std::move(M), std::move(llvmcontext))))
        return llvm::errs() << "cannot add LLVM IR module:\n" << Error, 1;
    auto E = TheJIT->runMain(argc, argv, environ);
    if (!E)
        return llvm::errs() << "cannot run main function:\n" << E.takeError(), 1;
    return *E;
}
