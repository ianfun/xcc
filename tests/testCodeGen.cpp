#include "common.h"

    size_t num_typedefs, num_tags;
    auto ast = parser.run(num_typedefs, num_tags);
    printer.finalize();

    if (printer.NumErrors)
        return 1;

    ig.run(ast, num_typedefs, num_tags);

    xcc::IRModuleOutputFileHelper output(engine, ig.module, "example.ll");

    output.verify();

    // output.dump();

    output.IR();
    output.finalize();

    // system("vim example.ll");

    return 0;
}
