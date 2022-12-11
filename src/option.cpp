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
    bool TimeTrace: 1;
    unsigned OptimizationLevel;
    unsigned OptimizeSize;
    unsigned TimeTraceGranularity;
    std::vector<std::string> PassPlugins;
    llvm::Reloc::Model RelocationModel;
    Optional<llvm::CodeModel::Model> CodeModel;
    Options(): 
        mainFileName{},
        triple{},
        CWD{},
        theTarget{nullptr},
        g{false}, 
        trigraphs{false}, 
        UnrollLoops{false},
        RerollLoops{false},
        MergeFunctions{false},
        VectorizeLoop{false},
        VectorizeSLP{false},
        DisableIntegratedAS{false},
        VerifyModule{false},
        TimeTrace {false},
        OptimizationLevel{0},
        OptimizeSize{0},
        TimeTraceGranularity{500},
        PassPlugins{},
        RelocationModel{llvm::Reloc::PIC_},
        CodeModel{}
         {
            llvm::sys::fs::current_path(CWD);
        }
};
