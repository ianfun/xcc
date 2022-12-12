#define CC_STATICS 0
#define CC_PRINT_CDECL 1
#define CC_PRINT_CDECL_FUNCTION 1

#include <string>
#include <iostream>
#include <sstream>
#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"

int main(int argc_, const char **argv_)
{
    llvm::InitializeNativeTarget();
    
    xcc::CompilerInstance XCC;

    // make our text printer that print to stderr
    xcc::TextDiagnosticPrinter printer(llvm::errs()); 

    XInitLLVM crashReport(XCC.createDiags(), argc_, argv_);

    xcc::DiagnosticsStore DiagStore;
    XCC.addPrinter(&DiagStore);

    printer.setSourceMgr(&XCC.createSourceManager());

/*
    xcc::driver::Driver theDriver(XCC.getDiags());

    // init args
    llvm::ArrayRef<const char*> argv(argv_, argv_ + argc_);

    int ret = 0;
    // parse options ...
    if (theDriver.BuildCompilation(argv, XCC.createOptions(), XCC.getSourceManager(), ret))
        return ret;
*/

    xcc::intepreter::InteractiveInterpreter interpreter(XCC);

    std::string line;

    std::cerr
       << "###### XCC interpreter demo ######" << std::endl 
       << "Press EOF(Ctrl+D) to exit." << std::endl;

    for (unsigned InputCount = 0;!std::cin.eof();InputCount++) {
        std::cerr << "xcc [" << InputCount << "]: ";
        std::getline(std::cin, line);
        bool ret = interpreter.ParseAndExecute(line);
        if (!ret) {
            DiagStore.FlushDiagnostics(printer);
            DiagStore.clear();
        }
        line.clear();
    }

    std::cerr << "EOF reached.Exit." << std::endl;
    printer.finalize();
    return 0;
}
