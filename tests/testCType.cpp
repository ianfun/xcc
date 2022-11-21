#include "../src/xcc.h"
#include <llvm/Support/InitLLVM.h>

int main(int argc_, const char **argv_) {
    llvm::InitLLVM trace(argc_, argv_);

    xcc::OpaqueCType::TEST_MAIN();

    return 0;
}
