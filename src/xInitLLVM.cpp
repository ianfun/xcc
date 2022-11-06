struct XInitLLVM : public xcc::DiagnosticHelper {
    static void handle_llvm_error(void *user_data, const char *reason, bool gen_crash_diag) {
        reinterpret_cast<XInitLLVM *>(user_data)->fatal("LLVM ERROR: %s\n", reason);
    }
    llvm::InitLLVM LLVM;
    XInitLLVM(xcc::xcc_context &context, int &Argc, const char **&Argv, bool InstallPipeSignalExitHandler = true)
        : DiagnosticHelper{context}, LLVM(Argc, Argv, InstallPipeSignalExitHandler) {
        llvm::install_fatal_error_handler(handle_llvm_error, this);
        llvm::install_bad_alloc_error_handler(handle_llvm_error, this);
        llvm::setBugReportMsg("This is " CC_VERSION_FULL "\nPLEASE submit a bug report to " CC_URL
                              " and include the crash backtrace.\n");
    }
};
