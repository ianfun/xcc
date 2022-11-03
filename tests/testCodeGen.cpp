#include "common.h"

    unsigned num_typedefs, num_tags;
    auto ast = parser.run(num_typedefs, num_tags);

    if (ctx.printer->NumErrors)
        return 1;

    ig.run(ast, num_typedefs, num_tags);

    xcc::IRModuleOutputFileHelper output(ctx, ig.module, "example.ll");

    output.verify();
    //output.dump();

    output.IR();
    output.finalize();

    return 0;
}
