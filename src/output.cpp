struct IRModuleOutputFileHelper : public DiagnosticHelper {
    llvm::Module *M;
    StringRef outputPath;
    std::error_code EC;
    raw_fd_ostream OS;
    // the outputPath may be '-', will print to stdout
    IRModuleOutputFileHelper(xcc_context &context, llvm::Module *M, StringRef outputPath)
        : DiagnosticHelper{context}, M{M}, outputPath{outputPath}, EC{}, OS{outputPath, EC} {
        if (EC) {
            error("open output(%R): %R", outputPath, StringRef(EC.message()));
        }
    }
    void maybe_print_error() {
        if (OS.has_error()) {
#pragma push_macro("error")
#undef error
            EC = OS.error();
#pragma pop_macro("error")
            error("error writting file(%R): %R", outputPath, StringRef(EC.message()));
        }
    }
    // the function will delete the module when finished linking(take the ownership)!
    bool link(llvm::Module &dst) {
        std::unique_ptr<llvm::Module> ptr;
        ptr.reset(M);
        return llvm::Linker::linkModules(dst, std::move(ptr));
    }
    bool verify() { return llvm::verifyModule(*M, &llvm::errs()); }
    // output functions
    void dump() { M->print(llvm::errs(), nullptr); }
    void IR() {
        if (!EC)
            M->print(OS, nullptr);
        maybe_print_error();
    }
    void BitCode() { WriteBitcodeToFile(*M, OS); }
    void _target(llvm::TargetMachine &M, llvm::CodeGenFileType type) {
        llvm::legacy::PassManager pass;
        M.addPassesToEmitFile(pass, OS, nullptr, type);
        pass.run(*(this->M));
    }
    void assembly(llvm::TargetMachine &M) { return _target(M, llvm::CGFT_AssemblyFile); }
    void objectfile(llvm::TargetMachine &M) { return _target(M, llvm::CGFT_ObjectFile); }
    void finalize() { OS.close(); }
};
