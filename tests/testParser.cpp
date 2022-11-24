#include "common.h"

    size_t num_typedefs, num_tags;

    auto ast = parser.run(num_typedefs, num_tags);

    (void)ast;

    printer.finalize();

    return engine.getNumErrors() != 0;
}
