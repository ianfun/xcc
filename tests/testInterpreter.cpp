#include "common.h"

    size_t num_typedefs, num_tags;

    [[maybe_unused]] xcc::Stmt ast = parser.run(num_typedefs, num_tags);

    printer.finalize();

    return engine.getNumErrors() != 0;
}
