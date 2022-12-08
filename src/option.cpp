struct Options {
    StringRef mainFileName;
    llvm::Triple triple;
    SmallString<256> CWD;
    const llvm::Target *theTarget;
    bool g : 1;
    bool trigraphs : 1;
    bool UnrollLoops : 1;
    bool RerollLoops : 1;
    bool MergeFunctions : 1;
    bool VectorizeLoop : 1;
    bool VectorizeSLP : 1;
    bool DisableIntegratedAS : 1;
    bool VerifyModule : 1;
    unsigned OptimizationLevel;
    unsigned OptimizeSize;
    std::vector<std::string> PassPlugins;
    llvm::Reloc::Model RelocationModel;
    Optional<llvm::CodeModel::Model> CodeModel;
    Options() { llvm::sys::fs::current_path(CWD); }
    void resetToDefault() {
        g = false;
        trigraphs = false;
        UnrollLoops = false;
        RerollLoops = false;
        MergeFunctions = false;
        VectorizeLoop = false;
        VectorizeSLP = false;
        DisableIntegratedAS = false;
        VerifyModule = false;

        OptimizationLevel = 0;
        OptimizeSize = 0;
        RelocationModel = llvm::Reloc::PIC_;
        theTarget = nullptr;
        triple = llvm::Triple();
        MergeFunctions = false;
        PassPlugins.clear();
        CWD.clear();
        llvm::sys::fs::current_path(CWD);
        mainFileName = StringRef();
    }
};
