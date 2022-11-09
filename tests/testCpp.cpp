#include "common.h"

    do {
        parser.l.cpp();
        parser.l.tok.dump(llvm::errs());
        llvm::errs() << '\n';
    } while (parser.l.tok.tok != xcc::TEOF);

    return 0;
}
