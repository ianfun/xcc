struct LinkerRunner: public DiagnosticHelper {
    StringRef linkerPath, filePath, outputPath;
    ProcessInfo info{};
    std::string ErrMsg{};
    LinkerRunner(xcc_context &context, StringRef linkerPath, StringRef filePath, StringRef outputPath): 
        DiagnosticHelper{context}, 
        linkerPath{linkerPath}, 
        filePath{filePath}, 
        outputPath{outputPath} {}
    void run() {
        bool failed = false;
        info = llvm::sys::ExecuteNoWait(
            linkerPath, 
            {filePath, "-o", outputPath}, 
            llvm::None, 
            llvm::None,
            0,
            &ErrMsg,
            &failed);
        if (failed)
            error("execute linker failed: %R", StringRef(ErrMsg));
    }
    void wait() {
        (void)Wait(info, 0, true, &ErrMsg);
    }
    void runAndWait() {
        bool failed = false;
        int status = llvm::sys::ExecuteAndWait(
                linkerPath, 
                {filePath, "-o", outputPath}, 
                llvm::None,
                llvm::None,
                0,
                0,
                &ErrMsg, 
                &failed
        );
        if (status || failed)
            error("execute linker failed: %R", StringRef(ErrMsg));
    }
};
