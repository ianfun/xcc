#include "common.h"

    xcc::TokenV tok;

    do {
        tok = parser.l.lexAndLoc();
        tok.dump(llvm::errs(), SM);
        llvm::errs() << '\n';
    } while (tok.tok != xcc::TEOF);

    return 0;
}
