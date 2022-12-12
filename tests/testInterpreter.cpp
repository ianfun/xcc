#define CC_STATICS 0
#define CC_PRINT_CDECL 1
#define CC_PRINT_CDECL_FUNCTION 1

#include <string>
#include <iostream>
#include "../src/xcc.h"
#include "../src/xInitLLVM.cpp"
#include "../src/Driver/Driver.cpp"

namespace xcc {
#include "../src/terminal/terminal.h"
}

int main(int argc_, const char **argv_)
{
    if (!xcc::Terminal::isRealTermianl()) {
        llvm::errs() << "fatal error: output is not a termianl.";
        return 1;
    }
    xcc::Terminal terminal;

    terminal.installSignalHandlers();

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

    xcc::intepreter::Interpreter interpreter(XCC);

    terminal.write(
"\n"
"~                                                                      ~\n"
"~                                                                      ~\n"
"~                                                                      ~\n"
"~                       # XCC interpreter demo #                       ~\n"
"~                                                                      ~\n"
"~                       Press EOF(Ctrl+D) to exit.                     ~\n"
"~                                                                      ~\n"
"~                                                                      ~\n"
"~                                                                      ~\n"
"\n"
    );
    
    std::string line;
    
    for (unsigned InputCount = 0;!std::cin.eof();InputCount++) {
        char buffer[15];
        int N = snprintf(buffer, sizeof(buffer), "%u", InputCount++);
        terminal.write("in [");
        terminal.changeColor(xcc::GREEN);
        terminal.write(buffer);
        terminal.resetColor();
        terminal.write("]: ");
        std::getline(std::cin, line);
        bool ret = interpreter.ParseAndExecute(line);
        if (!ret) {
            DiagStore.FlushDiagnostics(printer);
            DiagStore.clear();
        }
        line.clear();
    }
    terminal.write("Leave XCC interpreter.\n");
    printer.finalize();
    return 0;
}
