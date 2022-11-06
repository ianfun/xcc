#include "common.h"
    size_t num_typedefs, num_tags;
    auto ast = parser.run(num_typedefs, num_tags);
    thePrinter->finalize();
    return (int)(bool)ctx.printer->NumErrors;
}
