#include "common.h"

    xcc::TranslationUnit TU;
    parser.startParse();
    parser.run(TU);
    if (engine.getNumErrors()) {
        printer.finalize();
        return 1;
    }

    std::unique_ptr<llvm::Module> M(ig.run(TU, type_cache));

    xcc::StmtReleaser().Visit(TU.ast);

    xcc::IRModuleOutputFileHelper output(engine, std::move(M), "example.ll");

    output.verify();
    // output.dump();

    output.IR();
    output.finalize();

    printer.finalize();

    return 0;
}
