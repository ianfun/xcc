#include "common.h"
    size_t num_typedefs, num_tags;
    auto ast = parser.run(num_typedefs, num_tags);
    if (ctx.printer->NumErrors)
        return 1;
    return 0;
}
