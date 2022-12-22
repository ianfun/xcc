struct Options {
    StringRef mainFileName;
    llvm::Triple triple;
    llvm::DataLayout DL;
    llvm::TargetMachine *machine;
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
    Options(std::string tripleStr = llvm::sys::getDefaultTargetTriple()): 
        mainFileName{},
        triple{tripleStr},
        DL{""},
        machine{nullptr},
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
    llvm::CodeGenOpt::Level getCGOptLevel() const {
        switch (OptimizationLevel) {
        default: llvm_unreachable("Invalid optimization level!");
        case 0: return llvm::CodeGenOpt::None;
        case 1: return llvm::CodeGenOpt::Less;
        case 2: return llvm::CodeGenOpt::Default; // O2/Os/Oz
        case 3: return llvm::CodeGenOpt::Aggressive;
        }
    }
    void createTarget(DiagnosticsEngine &engine) {
        if (this->theTarget) return;
        std::string Error;
        this->theTarget = llvm::TargetRegistry::lookupTarget(this->triple.str(), Error);
        if (!this->theTarget) {
            DiagnosticHelper helper{engine};
            helper.error("unknown target triple %R, please use -triple or -arch", this->triple.str());
            helper.note("%R", Error);
            auto it = llvm::TargetRegistry::targets().begin();
            assert(it != llvm::TargetRegistry::targets().end() && "No target registered");
            this->theTarget = &*it;
        }
        llvm::TargetOptions opt;
        this->machine = this->theTarget->createTargetMachine(
            this->triple.str(), 
            "generic", 
            "", 
            opt,
            this->RelocationModel, 
            this->CodeModel,
            this->getCGOptLevel()
        );
        if (!this->machine)
            llvm::report_fatal_error("Failed to create TargetMachine");
        this->DL = this->machine->createDataLayout();
    }
};
