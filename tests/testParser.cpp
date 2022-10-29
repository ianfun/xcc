#include "common.h"

    auto ast = parser.run();
    if (ctx.printer->NumErrors)
        return 1;
    return 0;
}
