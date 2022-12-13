#include "common.h"

    xcc::TranslationUnit TU;

    parser.run(TU);

    printer.finalize();

    return engine.getNumErrors() != 0;
}
