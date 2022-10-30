#include "common.h"

    auto ast = parser.run();

    if (ctx.printer->NumErrors)
        return 1;

    ig.run(ast);

    xcc::IRModuleOutputFileHelper output(ctx, &ig.M(), "example.ll");

    //output.verify();
    //output.dump();

    output.IR();
    output.finalize();

    delete &ig.M();
    return 0;
}
