struct CompilerInstance
{
    CompilerInstance() {}
    CompilerInstance(const CompilerInstance &) = delete;
    void operator=(const CompilerInstance &) = delete;
    ~CompilerInstance() {
        // note: It's safe to delete nullptr!
        delete SM;
        delete parser;
        delete context;
        delete type_cache;
#ifdef XCC_TARGET
        delete Target;
#endif
        delete codegen;
        delete engine;
        delete llvm_context;
        delete options;
    }
    struct DiagnosticsEngine *engine = nullptr;
    struct SourceMgr *SM = nullptr;
#ifdef XCC_TARGET
    struct TargetInfo *Target = nullptr;
#endif
    struct Parser *parser = nullptr;
    struct LLVMTypeConsumer *type_cache = nullptr;
    struct xcc_context *context = nullptr;
    struct IRGen *codegen = nullptr;
    LLVMContext *llvm_context = nullptr;
    Options *options = nullptr;

    void addPrinter(struct DiagnosticConsumer *C) {
        createDiags().addConsumer(C);
    }
    struct Options &createOptions() {
        if (!options)
            options = new Options();
        return *options;
    }
    DiagnosticsEngine &createDiags() {
        if (!engine)
            engine = new DiagnosticsEngine();
        return *engine;
    }
    SourceMgr& createSourceManager() {
        if (!SM)
            SM = new SourceMgr(createDiags());
        return *SM;
    }
    xcc_context &createContext() {
        if (!context)
            context = new xcc_context();
        return *context;
    }
    LLVMContext &createLLVMContext() {
        if (!llvm_context)
            llvm_context = new LLVMContext();
        return *llvm_context;
    }
    const llvm::Target* createTarget() {
        createOptions();
        if (options->theTarget) return options->theTarget;
        std::string Error;
        options->theTarget = llvm::TargetRegistry::lookupTarget(options->triple.str(), Error);
        if (!options->theTarget) {
            DiagnosticHelper helper{createDiags()};
            helper.error("unknown target triple %R, please use -triple or -arch", options->triple.str());
            helper.note("%R", Error);
            auto it = llvm::TargetRegistry::targets().begin();
            if (it != llvm::TargetRegistry::targets().end())
                options->theTarget = &*it;
        }
        return options->theTarget;
    }
    IRGen &createCodeGen() {
        createTarget();
        if (!codegen)
            codegen = new IRGen(createContext(), createDiags(), createSourceManager(), createLLVMContext(), createOptions());
        return *codegen;
    }
    LLVMTypeConsumer &createTypeCache() {
        if (!type_cache)
            type_cache = new LLVMTypeConsumer(createContext(), createLLVMContext());
        return *type_cache;
    }
    Parser &createParser() {
        if (!parser)
            parser = new Parser(createSourceManager(), createCodeGen(), createDiags(), createContext(), createTypeCache());
        return *parser;
    }

    // * * * * * high level API * * * * *

    // Lex a raw token without expanding macros(but process directives)
    TokenV Lex() {
        return parser->l.lex();
    }
    // Lex a raw token and fill it's location fields.
    TokenV LexAndLoc() {
        return parser->l.lexAndLoc();
    }
    // Preprocessing - Return the current token of the token stream
    TokenV Cpp() {
        parser->l.cpp();
        return parser->l.tok;
    }
    // Parse a TranslationUnit
    // \pre prepareParsing()
    void Parse(TranslationUnit &TU) {
        return parser->run(TU);
    }
    // Emit LLVM IR for a TranslationUnit.
    // \pre prepareCodeGen
    std::unique_ptr<llvm::Module> CodeGen(const TranslationUnit &TU) {
        return codegen->run(TU, createTypeCache());
    }
    // Parse a TranslationUnit and emit LLVM IR.
    std::unique_ptr<llvm::Module> ParseAndCodeGen() {
        TranslationUnit TU;
        Parse(TU);
        return CodeGen(TU);
    }

    bool hasContext() const {
        return context != nullptr;
    }
    const struct xcc_context &getContext() const {
        assert(context);
        return *context;
    }
    struct xcc_context &getContext() {
        assert(context);
        return *context;
    }
    void setContext(struct xcc_context &TheContext) {
        if (context) delete context;
        context = &TheContext;
    }

    bool hasParser() const {
        return parser != nullptr;
    }
    const struct Parser &getParser() const {
        assert(parser);
        return *parser;
    }
    struct Parser &getParser() {
        assert(parser);
        return *parser;
    }
    void setParser(struct Parser &TheParser) {
        if (parser) delete parser;
        parser = &TheParser;
    }

    bool hasTypeCache() const {
        return type_cache != nullptr;
    }
    const struct LLVMTypeConsumer &getTypeCache() const {
        assert(type_cache);
        return *type_cache;
    }
    struct LLVMTypeConsumer &getTypeCache() {
        assert(type_cache);
        return *type_cache;
    }
    void setTypeCache(struct LLVMTypeConsumer &TheTypeCache) {
        if (type_cache) delete type_cache;
        type_cache = &TheTypeCache;
    }
#ifdef XCC_TARGET
    bool hasTarget() const {
        return Target != nullptr;
    }
    struct TargetInfo &getTarget() {
        assert(Target);
        return *Target;
    }
    const struct TargetInfo &getTarget() const {
        assert(Target);
        return *Target;
    }
    void setTarget(TargetInfo &theTarget) {
        if (Target) delete Target;
        Target = &theTarget;
    }
#endif
    bool hasSourceManager() const {
        return SM != nullptr;
    }
    struct SourceMgr &getSourceManager() {
        assert(SM);
        return *SM;
    }
    const struct SourceMgr &getSourceManager() const {
        assert(SM);
        return *SM;
    }
    void setSourceManager(struct SourceMgr &TheSM) {
        if (SM) delete SM;
        SM = &TheSM;
    }

    bool hasCodGen() const {
        return codegen != nullptr;
    }
    struct IRGen &getCodGen() {
        assert(codegen);
        return *codegen;
    }
    const struct IRGen &getCodGen() const {
        assert(codegen);
        return *codegen;
    }
    void setCodGen(struct IRGen &TheGen) {
        if (codegen) delete codegen;
        codegen = &TheGen;
    }

    bool hasDiags() const {
        return engine != nullptr;
    }
    struct DiagnosticsEngine &getDiags() {
        assert(engine);
        return *engine;
    }
    const struct DiagnosticsEngine &getDiags() const {
        assert(engine);
        return *engine;
    }
    void setDiags(struct DiagnosticsEngine &TheEngine) {
        if (engine) delete engine;
        engine = &TheEngine;
    }

    bool hasLLVMContext() const {
        return llvm_context != nullptr;
    }
    LLVMContext &getLLVMContext() {
        assert(llvm_context);
        return *llvm_context;
    }
    const LLVMContext &getLLVMContext() const {
        assert(llvm_context);
        return *llvm_context;
    }
    void setLLVMContext(LLVMContext &TheContext) {
        if (llvm_context) delete llvm_context;
        llvm_context = &TheContext;
    }

    bool hasOptions() const {
        return options != nullptr;
    }
    struct Options &getOptions() {
        assert(options);
        return *options;
    }
    const struct Options &getOptions() const {
        assert(options);
        return *options;
    }
    void setOptions(struct Options &TheOptions) {
        if (options) delete options;
        options = &TheOptions;
    }
};
