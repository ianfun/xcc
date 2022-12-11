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

    XCC.addPrinter(&printer);

    printer.setSourceMgr(&XCC.createSourceManager());

    xcc::driver::Driver theDriver(XCC.getDiags());

    // init args
    llvm::ArrayRef<const char*> argv(argv_, argv_ + argc_);

    int ret = 0;
    // parse options ...
    if (theDriver.BuildCompilation(argv, XCC.createOptions(), XCC.getSourceManager(), ret))
        return ret;

    xcc::intepreter::InteractiveInterpreter interpreter(XCC);

    std::string line;

    std::cerr
       << "###### XCC interpreter demo ######" << std::endl 
       << "Press EOF(Ctrl+D) to exit." << std::endl;

    for (unsigned InputCount = 0;;InputCount++) {
        std::cerr << "xcc [" << InputCount << "]: ";
        std::getline(std::cin, line);
        line += '\n';
        interpreter.ParseAndExecute(line);
    }

    std::cerr << "EOF reached.Exit." << std::endl;
    return 0;
}
