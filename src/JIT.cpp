namespace jit {
using namespace llvm;
using namespace llvm::orc;
struct JITRunner {
    std::unique_ptr<ExecutionSession> ES;
    DataLayout DL;
    MangleAndInterner Mangle;
    RTDyldObjectLinkingLayer ObjectLayer;
    IRCompileLayer CompileLayer;
    JITDylib &MainJD;

    JITRunner(std::unique_ptr<ExecutionSession> ES, JITTargetMachineBuilder JTMB, DataLayout DL)
        : ES(std::move(ES)), DL(std::move(DL)), Mangle(*this->ES, this->DL),
          ObjectLayer(*this->ES, []() { return std::make_unique<SectionMemoryManager>(); }),
          CompileLayer(*this->ES, ObjectLayer, std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
          MainJD(this->ES->createBareJITDylib("<main>")) {
        MainJD.addGenerator(cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix())));
    }
    ~JITRunner() {
        if (auto Err = ES->endSession())
            ES->reportError(std::move(Err));
    }
    static Expected<std::unique_ptr<JITRunner>> Create() {
        auto EPC = SelfExecutorProcessControl::Create();
        if (!EPC)
            return EPC.takeError();

        auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

        JITTargetMachineBuilder JTMB(ES->getExecutorProcessControl().getTargetTriple());

        auto DL = JTMB.getDefaultDataLayoutForTarget();
        if (!DL)
            return DL.takeError();

        return std::make_unique<JITRunner>(std::move(ES), std::move(JTMB), std::move(*DL));
    }
    class Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr) {
        if (!RT)
            RT = MainJD.getDefaultResourceTracker();
        return CompileLayer.add(RT, std::move(TSM));
    }
    Expected<JITEvaluatedSymbol> lookup(StringRef Name) { return ES->lookup({&MainJD}, Mangle(Name.str())); }
    Expected<int> runMain(int argc, const char *const *argv, const char *const *env) {
        auto mainE = lookup("main");
        if (!mainE)
            return mainE.takeError();
        auto address = reinterpret_cast<int (*)(int, const char *const *, const char *const *env)>(mainE->getAddress());
        return address(argc, argv, env);
    }
};
} // namespace
