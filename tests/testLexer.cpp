#include "common.h"

    xcc::TokenV tok;

    do {
        tok = parser.l.lex();
        tok.dump(llvm::errs());
        llvm::errs() << '\n';
    } while (tok.tok != xcc::TEOF);

    return 0;
}
