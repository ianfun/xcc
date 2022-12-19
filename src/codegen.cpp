/*
 * codegen.cpp
 *
 * Code generation to LLVM IR, and Debug Info
 * implements getAlignof() and getsizeof() method(used by Sema and constant folding).
 */


struct IRGen : public DiagnosticHelper {
    static constexpr llvm::GlobalValue::LinkageTypes ExternalLinkage = llvm::GlobalValue::ExternalLinkage,
                          PrivateLinkage = llvm::GlobalValue::PrivateLinkage,
                          CommonLinkage = llvm::GlobalValue::CommonLinkage,
                          AppendingLinkage = llvm::GlobalValue::AppendingLinkage,
                          InternalLinkage = llvm::GlobalValue::InternalLinkage;
    static constexpr llvm::GlobalValue::DLLStorageClassTypes DefaultStorageClass = llvm::GlobalValue::DefaultStorageClass,
                          DLLImportStorageClass = llvm::GlobalValue::DLLImportStorageClass,
                          DLLExportStorageClass = llvm::GlobalValue::DLLExportStorageClass;

    DenseMap<StringRef, llvm::GlobalVariable *> str8Map;
    DenseMap<ArrayRef<uint16_t>, llvm::GlobalVariable *> str16Map;
    DenseMap<ArrayRef<uint32_t>, llvm::GlobalVariable *> str32Map;

    LLVMTypeConsumer &type_cache;
    xcc_context &context;
    std::unique_ptr<llvm::Module> module;
    SourceMgr &SM;
    const Options &options;
    llvm::Triple triple;
    llvm::Function *currentfunction = nullptr;
    llvm::BasicBlock::iterator allocaInsertPt;
    llvm::DIBasicType* di_basic_types[LLVMTypeConsumer::TypeIndexHigh];
    unsigned lastFileID = -1;
    llvm::DIFile *lastFile = nullptr;
    llvm::SmallVector<llvm::BasicBlock *, 5> labels{};
    DenseMap<CType, llvm::DISubroutineType*> di_subroutine_type_cache;
    DenseMap<Expr, llvm::DILocalVariable*> di_vla_expr_cache;
    DenseMap<CType, llvm::DICompositeType*> di_composite_cache;
    SmallVector<llvm::DIScope*> di_lex_scopes;
    llvm::Value **vars = nullptr; // llvm::GlobalValue* or llvm::AllocaInst*
    Stmt currentfunctionAST = nullptr;
    llvm::BasicBlock *insertBB = nullptr;
    
    location_t last_location_t = -1;
    unsigned current_line = 0;
    unsigned current_column = 0;
    unsigned vla_expr_count = 0;
    const IncludeFile *current_file = nullptr;
    llvm::DIFile* di_cur_file = nullptr;
    llvm::DIBuilder *di = nullptr;
    llvm::DICompileUnit *CU = nullptr;
    llvm::DILocation *di_cur_location = nullptr;
    bool g = false;
    LLVMContext &getLLVMContext() {
        return type_cache.getLLVMContext();
    }

private:
    auto &getMapFor(const StringRef &) { return str8Map; }
    auto &getMapFor(const ArrayRef<uint16_t> &) { return str16Map; }
    auto &getMapFor(const ArrayRef<uint32_t> &) { return str32Map; }
    template <typename T>
    llvm::GlobalVariable *createString(llvm::Constant *init, T str) {
        auto it = getMapFor(str).insert({str, nullptr});
        if (it.second) {
            llvm::GlobalVariable *GV = new llvm::GlobalVariable(*module, init->getType(), true, llvm::GlobalValue::PrivateLinkage, init, ".cstr");
            GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            // GV->setAlignment(options.DL.getPreferredAlign(GV));
            GV->setAlignment(options.DL.getABITypeAlign(init->getType()));
            it.first->second = GV;
        }
        return it.first->second;
    }
    llvm::GlobalVariable * createString(llvm::Constant *C) {
        llvm::IntegerType *T = cast<llvm::IntegerType>(cast<llvm::ArrayType>(C->getType())->getElementType());
        switch (T->getBitWidth()) {
        case 8: 
        {
            if (auto CA = dyn_cast<llvm::ConstantDataArray>(C)) {
                return createString(C, CA->getRawDataValues());
            }
            assert(isa<llvm::ConstantAggregateZero>(C));
            return createString(C, StringRef());
        }
        case 16:
        {
            if (auto CA = dyn_cast<llvm::ConstantDataArray>(C)) {
                const uint16_t *Data = reinterpret_cast<const uint16_t*>(CA->getRawDataValues().data());
                unsigned numElements = CA->getNumElements();
                return createString(
                    C,
                    llvm::makeArrayRef(
                        Data + numElements,
                        numElements
                    )
                );
            }
            assert(isa<llvm::ConstantAggregateZero>(C));
            return createString(C, ArrayRef<uint16_t>());
        }
        case 32:
        {
            if (auto CA = dyn_cast<llvm::ConstantDataArray>(C)) {
                const uint32_t *Data = reinterpret_cast<const uint32_t*>(CA->getRawDataValues().data());
                unsigned numElements = CA->getNumElements();
                return createString(
                    C,
                    llvm::makeArrayRef(
                        Data + numElements,
                        numElements
                    )
                );
            }
            assert(isa<llvm::ConstantAggregateZero>(C));
            return createString(C, ArrayRef<uint32_t>());
        }
        default: llvm_unreachable("unhandled character size");
        }
    }
    llvm::GlobalVariable *createString(Expr e) {
        return createString(e->string);
    }
    void setDebugLoc(location_t loc) {
        assert(options.g && "Not in debug mode");
        if (loc && loc != last_location_t) {
            const IncludeFile *file;
            std::pair<unsigned, unsigned> pair = SM.getLineAndColumn(loc, file);
            if (pair.first != 0) {
                current_line = pair.first;
                current_column = pair.second;
                if (file != current_file) {
                    current_file = file;
                    di_cur_file = di->createFile(file->cache->getName(), "");
                }
                di_cur_location = llvm::DILocation::get(getLLVMContext(), current_line, current_column, getLexScope());
            }
        }
    }
    llvm::DIScope *getLexScope() {
        assert(di_lex_scopes.size() && "empty scope!");
        return di_lex_scopes.back();
    }
    llvm::DIFile *getFile() {
        return di_cur_file;
    }
    llvm::DILocation* getCurrentDebugLocation() const {
        return di_cur_location;
    }
    unsigned getLine() {
        return current_line;
    }
    unsigned getColumn() {
        return current_column;
    }
    void AddMetadataToInst(llvm::Instruction *I) {
        if (options.g) {
            I->setDebugLoc(llvm::DebugLoc(di_cur_location));
        }
    }
    template <typename T> 
    T *Insert(T *I) {
        AddMetadataToInst(I);
        return I;
    }
    llvm::ReturnInst *ret(llvm::Value *Val = nullptr) {
        return Insert(llvm::ReturnInst::Create(getLLVMContext(), Val, insertBB));
    }
    llvm::CastInst *createCast(llvm::Instruction::CastOps Op, llvm::Value *V, llvm::Type *DestTy) {
        return Insert(llvm::CastInst::Create(Op, V, DestTy, "", insertBB));
    }
    llvm::CastInst *ptrtoint(llvm::Value *V, llvm::Type *DestTy) {
        return createCast(llvm::Instruction::PtrToInt, V, DestTy);
    }
    llvm::BinaryOperator *createNot(llvm::Value *V) {
        return binop(llvm::Instruction::Xor, V, llvm::Constant::getAllOnesValue(V->getType()));
    }
    llvm::BinaryOperator *neg(llvm::Value *V) {
        return binop(llvm::Instruction::Sub, llvm::Constant::getNullValue(V->getType()), V);
    }
    llvm::UnaryOperator *fneg(llvm::Value *V) {
        return Insert(llvm::UnaryOperator::Create(llvm::Instruction::FNeg, V, "", insertBB));
    }
    llvm::AllocaInst *createAlloca(llvm::Type *Ty) {
        unsigned AddrSpace = options.DL.getAllocaAddrSpace();
        llvm::Align Align = options.DL.getPrefTypeAlign(Ty);
        return Insert(new llvm::AllocaInst(Ty, AddrSpace, nullptr, Align, "", insertBB));
    }
    llvm::AllocaInst *createAlloca(llvm::Type *Ty, llvm::Align Align) {
        unsigned AddrSpace = options.DL.getAllocaAddrSpace();
        return Insert(new llvm::AllocaInst(Ty, AddrSpace, nullptr, Align, "", insertBB));
    }
    llvm::BinaryOperator *udiv(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::UDiv, LHS, RHS);
    }
    llvm::BinaryOperator *sdiv(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::SDiv, LHS, RHS);
    }
    llvm::UnreachableInst *createUnreachable() {
        return Insert(new llvm::UnreachableInst(getLLVMContext(), insertBB));
    }
    llvm::ICmpInst *createLogicalNot(llvm::Value *Val) {
        llvm::Constant *C;
        llvm::Type *Ty = Val->getType();
        if (llvm::IntegerType *IT = dyn_cast<llvm::IntegerType>(Ty)) {
            C = ConstantInt::get(getLLVMContext(), APInt::getZero(IT->getBitWidth()));
        } else if (isa<llvm::PointerType>(Ty)) {
            C = type_cache.null_ptr;
        } else {
            llvm_unreachable("bad argument to createLogicalNot");
        }
        return icmp(llvm::ICmpInst::ICMP_EQ, Val, C);
    }
    llvm::GetElementPtrInst *gep(llvm::Type *PointeeType, llvm::Value *Ptr, ArrayRef<llvm::Value*> IdxList) {
        return Insert(llvm::GetElementPtrInst::CreateInBounds(PointeeType, Ptr, IdxList, "", insertBB));
    }
    llvm::Value *structGEP(llvm::Type *StructTy, llvm::Value *Ptr, ArrayRef<unsigned> idxs) {
        while (idxs.size() && !idxs.back())
            idxs = idxs.drop_back();
        if (idxs.empty()) return Ptr;
        llvm::Value **vals = type_cache.alloc.Allocate<llvm::Value*>(idxs.size() + 1);
        vals[0] = ConstantInt::get(type_cache.intptrTy, 0);
        llvm::IntegerType *i32Ty = cast<llvm::IntegerType>(type_cache.integer_types[5]);
        for (size_t i = 0;i < idxs.size();++i)
            vals[i+1] = llvm::ConstantInt::get(i32Ty, idxs[i]);
        return gep(StructTy, Ptr, llvm::makeArrayRef(vals, idxs.size() + 1));
    }
    llvm::PHINode *phi(llvm::Type *Ty, unsigned NumReservedValues) {
        return Insert(llvm::PHINode::Create(Ty, NumReservedValues, "", insertBB));
    }
    llvm::SelectInst *select(llvm::Value *Cond, llvm::Value *T, llvm::Value *F) {
        return Insert(llvm::SelectInst::Create(Cond, T, F, "", insertBB));
    }
    llvm::ICmpInst *icmp(llvm::CmpInst::Predicate Pred, llvm::Value *LHS, llvm::Value *RHS) {
        return Insert(new llvm::ICmpInst(*insertBB, Pred, LHS, RHS));
    }
    llvm::BinaryOperator *binop(llvm::Instruction::BinaryOps Op, llvm::Value *LHS, llvm::Value *RHS) {
        return Insert(llvm::BinaryOperator::Create(Op, LHS, RHS, "", insertBB));
    }
    llvm::BinaryOperator *createOr(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::Or, LHS, RHS);
    }
    llvm::BinaryOperator *createAnd(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::And, LHS, RHS);
    }
    llvm::BinaryOperator *add(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::Add, LHS, RHS);
    }
    llvm::BinaryOperator *sub(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::Sub, LHS, RHS);
    }
    llvm::BinaryOperator *fadd(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::Add, LHS, RHS);
    }
    llvm::BinaryOperator *fsub(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::FSub, LHS, RHS);
    }
    llvm::BinaryOperator *fmul(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::FMul, LHS, RHS);
    }
    llvm::BinaryOperator *mul(llvm::Value *LHS, llvm::Value *RHS) {
        return binop(llvm::Instruction::Mul, LHS, RHS);
    }
    llvm::CmpInst *fcmp(llvm::FCmpInst::Predicate Pred, llvm::Value *LHS, llvm::Value *RHS) {
        return Insert(new llvm::FCmpInst(*insertBB, Pred, LHS, RHS));
    }
    llvm::InsertValueInst *insertValue(llvm::Value *Agg, llvm::Value *Val, ArrayRef<unsigned> idxs) {
        return Insert(llvm::InsertValueInst::Create(Agg, Val, idxs, "", insertBB));
    }
    llvm::AtomicRMWInst *atomicRMW(llvm::AtomicRMWInst::BinOp Op, llvm::Value *Ptr, llvm::Value *Val, llvm::Align Align,
        llvm::AtomicOrdering Ordering = llvm::AtomicOrdering::SequentiallyConsistent,
        llvm::SyncScope::ID SSID = llvm::SyncScope::System) {
        return Insert(new llvm::AtomicRMWInst(Op, Ptr, Val, Align, Ordering, SSID, insertBB));
    }
    llvm::BranchInst *br(llvm::BasicBlock *BB) {
        return Insert(llvm::BranchInst::Create(BB, insertBB));
    }
    llvm::BranchInst *condbr(llvm::Value *V, llvm::BasicBlock *T, llvm::BasicBlock *F) {
        return Insert(llvm::BranchInst::Create(T, F, V, insertBB));
    }
    llvm::CallInst *call(llvm::FunctionType *FTY, llvm::Value *FN, ArrayRef<llvm::Value*> Args = {}) {
        return Insert(llvm::CallInst::Create(llvm::FunctionCallee(FTY, FN), Args, {}, "", insertBB));
    }
    llvm::CallInst *call(llvm::FunctionCallee Func, ArrayRef<llvm::Value*> Args = {}) {
        return Insert(llvm::CallInst::Create(Func, Args, {}, "", insertBB));
    }
    llvm::ExtractValueInst* extractValue(llvm::Value *Agg, ArrayRef<unsigned> idxs) {
        return Insert(llvm::ExtractValueInst::Create(Agg, idxs, "", insertBB));
    }
    llvm::StoreInst* store(llvm::Value *p, llvm::Value *v) { 
        assert(p->getType()->isPointerTy() && "Store operand must be a pointer");
        return Insert(new llvm::StoreInst(v, p, insertBB));
    }
    llvm::StoreInst* store(llvm::Value *p, llvm::Value *v, llvm::Align align) {
        llvm::StoreInst *s = store(p, v);
        s->setAlignment(align);
        return s;
    }
    llvm::StoreInst* store(llvm::Value *p, llvm::Value *v, llvm::MaybeAlign align) {
        llvm::StoreInst *s = store(p, v);
        if (align.hasValue())
            s->setAlignment(*align);
        return s;
    }
    llvm::LoadInst *load(llvm::Value *p, llvm::Type *Ty) {
        assert(p);
        assert(Ty);
        return Insert(new llvm::LoadInst(Ty, p, "", insertBB));
    }
    llvm::LoadInst *load(llvm::Value *p, llvm::Type *Ty, llvm::Align align) {
        llvm::LoadInst *i = load(p, Ty);
        i->setAlignment(align);
        return i;
    }
    llvm::LoadInst *load(llvm::Value *p, llvm::Type *Ty, llvm::MaybeAlign align) {
        llvm::LoadInst *i = load(p, Ty);
        if (align.hasValue())
            i->setAlignment(*align);
        return i;
    }
    const llvm::Instruction *getTerminator() { return insertBB->getTerminator(); }
    llvm::ValueAsMetadata *mdNum(uint64_t num) {
        llvm::Value *v = llvm::ConstantInt::get(llvm::Type::getInt32Ty(getLLVMContext()), num);
        return llvm::ValueAsMetadata::get(v);
    }
    void append(llvm::BasicBlock *TheBB) {
        currentfunction->getBasicBlockList().push_back(TheBB);
        after(TheBB);
    }
    void after(llvm::BasicBlock *TheBB) { insertBB = TheBB; }
    llvm::Function *addFunction(llvm::FunctionType *ty, const Twine &N) {
        return llvm::Function::Create(ty, ExternalLinkage, N, module.get());
    }
    llvm::BasicBlock *newBB() {
        return llvm::BasicBlock::Create(getLLVMContext());
    }
    llvm::BasicBlock *addBB(StringRef Name = "") {
        return llvm::BasicBlock::Create(getLLVMContext(), "", currentfunction);
    }
    void handle_asm(StringRef s) {
        if (currentfunction)
            // module level asm
            module->appendModuleInlineAsm(s);
        else {
            auto ty = llvm::FunctionType::get(type_cache.void_type, false);
            auto f = llvm::InlineAsm::get(ty, s, StringRef(), true);
            call(ty, f);
        }
    }
    uint64_t getSizeInBits(llvm::Type *ty) { return options.DL.getTypeSizeInBits(ty); }
    uint64_t getSizeInBits(CType ty) { return getSizeInBits(wrap(ty)); }
    uint64_t getSizeInBits(Expr e) { return getSizeInBits(e->ty); }
    llvm::MDString *mdStr(StringRef str) { return llvm::MDString::get(getLLVMContext(), str); }
    llvm::MDNode *mdNode(llvm::Metadata *m) { return llvm::MDNode::get(getLLVMContext(), {m}); }
    llvm::MDNode *mdNode(ArrayRef<llvm::Metadata *> m) { return llvm::MDNode::get(getLLVMContext(), m); }
    void createModule() {
        module.reset(new llvm::Module(options.mainFileName, getLLVMContext()));
        module->setSourceFileName(options.mainFileName);
        module->setDataLayout(options.DL);
        module->setTargetTriple(options.triple.str());

        auto llvm_ident = module->getOrInsertNamedMetadata("llvm.ident");
        auto llvm_flags = module->getOrInsertNamedMetadata("llvm.module.flags");
        llvm_ident->addOperand(mdNode(mdStr(CC_VERSION_FULL)));

        llvm_flags->addOperand(mdNode({mdNum(1), mdStr("short_enum"), mdNum(1)}));
        llvm_flags->addOperand(mdNode({mdNum(1), mdStr("wchar_size"), mdNum(1)}));
        llvm_flags->addOperand(mdNode({mdNum(1), mdStr("short_wchar"), mdNum(1)}));
    }
    llvm::OptimizationLevel mapToLevel() const {
        switch (options.OptimizationLevel) {
        default: llvm_unreachable("Invalid optimization level!");

        case 0: return llvm::OptimizationLevel::O0;

        case 1: return llvm::OptimizationLevel::O1;

        case 2:
            switch (options.OptimizeSize) {
            default: llvm_unreachable("Invalid optimization level for size!");

            case 0: return llvm::OptimizationLevel::O2;

            case 1: return llvm::OptimizationLevel::Os;

            case 2: return llvm::OptimizationLevel::Oz;
            }

        case 3: return llvm::OptimizationLevel::O3;
        }
    }
    // clang::EmitAssemblyHelper::RunOptimizationPipeline
    void RunOptimizationPipeline() {
        llvm::Optional<llvm::PGOOptions> PGOOpt;
        options.machine->setPGOOption(PGOOpt);

        llvm::PipelineTuningOptions PTO;
        PTO.LoopUnrolling = options.UnrollLoops;
        // For historical reasons, loop interleaving is set to mirror setting for loop
        // unrolling.
        PTO.LoopInterleaving = options.UnrollLoops;
        PTO.LoopVectorization = options.VectorizeLoop;
        PTO.SLPVectorization = options.VectorizeSLP;
        PTO.MergeFunctions = options.MergeFunctions;
        // Only enable CGProfilePass when using integrated assembler, since
        // non-integrated assemblers don't recognize .cgprofile section.
        PTO.CallGraphProfile = !options.DisableIntegratedAS;

        llvm::LoopAnalysisManager LAM;
        llvm::FunctionAnalysisManager FAM;
        llvm::CGSCCAnalysisManager CGAM;
        llvm::ModuleAnalysisManager MAM;

        llvm::PassInstrumentationCallbacks PIC;
        llvm::PrintPassOptions PrintPassOpts;
        llvm::StandardInstrumentations SI(false,
                                          /*VerifyEach*/ false, PrintPassOpts);
        SI.registerCallbacks(PIC, &FAM);
        llvm::PassBuilder PB(options.machine, PTO, PGOOpt, &PIC);

        // Enable verify-debuginfo-preserve-each for new PM.
        DebugInfoPerPass DebugInfoBeforePass;
        // Attempt to load pass plugins and register their callbacks with PB.
        for (auto &PluginFN : options.PassPlugins) {
            auto PassPlugin = llvm::PassPlugin::Load(PluginFN);
            if (PassPlugin) {
                PassPlugin->registerPassBuilderCallbacks(PB);
            } else {
                StringRef str(PluginFN);
                std::string errStr(llvm::toString(PassPlugin.takeError()));
                StringRef errStr2(errStr);
                error("unable to load plugin %R: %R") << str << errStr2;
            }
        }
#define HANDLE_EXTENSION(Ext) get##Ext##PluginInfo().RegisterPassBuilderCallbacks(PB);
#include "llvm/Support/Extension.def"

        // Register all the basic analyses with the managers.
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        llvm::ModulePassManager MPM;
        // Map our optimization levels into one of the distinct levels used to
        // configure the pipeline.
        llvm::OptimizationLevel Level = mapToLevel();
        if (options.OptimizationLevel == 0) {
            MPM = PB.buildO0DefaultPipeline(Level, false);
        } else {
            MPM = PB.buildPerModuleDefaultPipeline(Level);
        }
        if (options.VerifyModule)
            MPM.addPass(llvm::VerifierPass());
        {
            llvm::PrettyStackTraceString CrashInfo("Optimizer");
            llvm::TimeTraceScope TimeScope("Optimizer");
            MPM.run(*module, MAM);
        }
    }
    void runCodeGenTranslationUnit(Stmt s) {
        for (Stmt ptr = s->next; ptr; ptr = ptr->next)
            gen(ptr);
    }
    llvm::Value *gen_complex_real(llvm::Value *v) { return extractValue(v, {0}); }
    llvm::Value *gen_complex_imag(llvm::Value *v) { return extractValue(v, {1}); }
    llvm::Value *make_complex_pair(llvm::Type *ty, llvm::Value *a, llvm::Value *b) {
        auto T = cast<llvm::StructType>(ty);
        if (!currentfunction)
            return llvm::ConstantStruct::get(T, {cast<llvm::Constant>(a), cast<llvm::Constant>(b)});
        return insertValue(insertValue(llvm::PoisonValue::get(T), a, {0}), b, {1});
    }
    llvm::Value *make_complex_pair(CType ty, llvm::Value *a, llvm::Value *b) {
        return make_complex_pair(wrapComplex(ty), a, b);
    }
    llvm::Value *gen_cond(Expr e) {
        assert(e);
        llvm::Value *V = gen(e);
        if (e->ty->isBool())
            return V;
        if (e->ty->isComplex()) {
            llvm::Value *a = gen_complex_real(V);
            llvm::Value *b = gen_complex_imag(V);
            if (e->ty->isFloating()) {
                auto zero = ConstantFP::getZero(a->getType());
                a = fcmp(llvm::FCmpInst::FCMP_UNO, a, zero);
                b = fcmp(llvm::FCmpInst::FCMP_UNO, b, zero);
            } else {
                auto zero = ConstantInt::getNullValue(a->getType());
                a = icmp(llvm::ICmpInst::ICMP_NE, a, zero);
                b = icmp(llvm::ICmpInst::ICMP_NE, b, zero);
            }
            return createOr(a, b);
        }
        auto T = V->getType();
        if (auto I = dyn_cast<llvm::IntegerType>(T))
            return icmp(llvm::ICmpInst::ICMP_NE, V, ConstantInt::get(I, 0));
        return fcmp(llvm::FCmpInst::FCMP_UNE, V, ConstantFP::getZero(T, false));
    }
    llvm::PHINode *gen_logical(Expr lhs, Expr rhs, bool isand = true) {
        llvm::BasicBlock *rightBB = newBB();
        llvm::BasicBlock *phiBB = newBB();
        auto R = gen_cond(lhs);
        llvm::BasicBlock *leftBB = insertBB;

        isand ? condbr(R, rightBB, phiBB) : condbr(R, phiBB, rightBB);

        append(rightBB);
        auto L = gen_cond(rhs);
        // gen_cond may change the basic block
        rightBB = insertBB;
        br(phiBB);

        append(phiBB);
        llvm::PHINode *Phi = phi(type_cache.integer_types[0], 2);
        Phi->addIncoming(R, leftBB);
        Phi->addIncoming(L, rightBB);
        return Phi;
    }
    llvm::Function *newFunction(llvm::FunctionType *fty, IdentRef name, type_tag_t tags, size_t idx,
                                bool isDefinition = false) {
        auto old = module->getFunction(name->getKey());
        if (old)
            return old;
        auto F = addFunction(fty, name->getKey());
        F->setDSOLocal(true);
        F->setDoesNotThrow();
        if (options.OptimizationLevel == 0) {
            F->addFnAttr(llvm::Attribute::NoInline);
            F->addFnAttr(llvm::Attribute::OptimizeNone);
        } else if (isDefinition) {
            F->addFnAttr(llvm::Attribute::OptimizeForSize);
        }
        if (tags & TYSTATIC)
            F->setLinkage(InternalLinkage);
        if (tags & TYNORETURN)
            F->setDoesNotReturn();
        if (tags & TYINLINE)
            F->addFnAttr(llvm::Attribute::InlineHint);
        vars[idx] = F;
        return F;
    }
    StringRef getLinkageName(type_tag_t tags) {
        if (tags & TYSTATIC)
            return "static";
        if (tags & TYEXTERN)
            return "extern";
        if (tags & TYREGISTER)
            return "register";
        if (tags & TYTHREAD_LOCAL)
            return "_Thread_local";
        return "";
    }
    void gen(Stmt s) {
        assert(s);
        switch (s->k) {
        case SSwitch: {
            auto cond = gen(s->itest);
            llvm::SwitchInst *sw = llvm::SwitchInst::Create(cond, labels[s->sw_default], s->switchs.size());
            if (s->gnu_switchs.size()) {
                for (const auto &it : s->gnu_switchs) {
                    // Range is small enough to add multiple switch instruction cases.
                    if (it.range.ult(CC_SITCH_RANGE_UNROLL_MAX)) {
                        const uint64_t range_end = it.CaseStart->getZExtValue() + it.range.getZExtValue();
                        for (uint64_t i = it.CaseStart->getZExtValue(); i <= range_end;) {
                            sw->addCase(ConstantInt::get(getLLVMContext(), APInt(it.CaseStart->getBitWidth(), i++)),
                                        labels[it.label]);
                        }
                        continue;
                    }
                    auto diff = sub(cond, ConstantInt::get(getLLVMContext(), *it.CaseStart));
                    auto c = icmp(llvm::ICmpInst::ICMP_UGE, diff, ConstantInt::get(getLLVMContext(), it.range));
                    auto BB = addBB();
                    condbr(c, labels[it.label], BB);
                    after(BB);
                }
            }
            if (s->switchs.empty()) {
                br(labels[s->sw_default]);
                return;
            }
            for (const auto &it : s->switchs)
                sw->addCase(ConstantInt::get(getLLVMContext(), *it.CaseStart), labels[it.label]);
            Insert(sw);
        } break;
        case SIndirectBr:
        {
            const label_t *start = currentfunctionAST->indirectBrs;
            assert(start && "indirect goto in function with no address-of-label expressions");
            unsigned num = *start++;
            llvm::Value *V = getAddress(s->jump_addr);
            llvm::IndirectBrInst *Inst = llvm::IndirectBrInst::Create(V, num);
            for (unsigned i = 0;i < num;++i)
                Inst->addDestination(labels[start[i]]);
            Insert(Inst);
        } break;
        case SHead: llvm_unreachable("");
        case SCompound: {
            if (options.g)
                di_lex_scopes.push_back(di->createLexicalBlock(getLexScope(), getFile(), getLine(), getColumn()));
            for (Stmt ptr = s->inner; ptr; ptr = ptr->next)
                gen(ptr);
            if (options.g)
                di_lex_scopes.pop_back();
        } break;
        case SDecl: {
            type_cache.handleDecl(s);
        } break;
        case SNoReturnCall:
            (void)gen(s->call_expr);
            createUnreachable();
            break;
        case SExpr: (void)gen(s->exprbody); break;
        case SFunction: {
            llvm::DISubprogram *SP;
            assert(this->labels.empty());
            auto ty = cast<llvm::FunctionType>(wrap(s->functy));
            currentfunctionAST = s;
            currentfunction =
                newFunction(ty, s->funcname, s->functy->getFunctionAttrTy()->getTags(), s->func_idx, true);
            if (options.g) {
                llvm::DISubroutineType *SubTy = cast<llvm::DISubroutineType>(wrapDIType(s->functy));
                SP = di->createFunction(getLexScope(), s->funcname->getKey(), getLinkageName(s->functy->getFunctionAttrTy()->getTags()), getFile(), getLine(), SubTy, getLine(), llvm::DINode::FlagZero, llvm::DISubprogram::SPFlagDefinition);
                di_lex_scopes.push_back(SP);
                setDebugLoc(s->funcDefLoc);
                currentfunction->setSubprogram(SP);
            }
            llvm::BasicBlock *entry = llvm::BasicBlock::Create(getLLVMContext(), "entry", currentfunction);
            after(entry);
            statics("# %u labels in function\n", s->numLabels);
            for (unsigned i = 0; i < s->numLabels; i++)
                this->labels.push_back(newBB());
            unsigned arg_no = 0;
            for (auto arg = currentfunction->arg_begin(); arg != currentfunction->arg_end(); ++arg) {
                llvm::Type *pty = ty->getParamType(arg_no);
                llvm::AllocaInst *p = createAlloca(pty);
                if (options.g) {
                    const Param &it = s->functy->params[arg_no];
                    setDebugLoc(it.loc);
                    llvm::DILocalVariable *var = di->createParameterVariable(SP, it.name->getKey(), arg_no, getFile(), getLine(), wrapDIType(it.ty));
                    di->insertDeclare(p, var, di->createExpression(), di_cur_location, insertBB);
                }
                store(p, arg);
                vars[s->localStart + arg_no++] = p;
            }
            allocaInsertPt = entry->begin();
            for (Stmt ptr = s->funcbody->next; ptr; ptr = ptr->next)
                gen(ptr);
            if (!getTerminator()) {
                auto retTy = ty->getReturnType();
                if (retTy->isVoidTy()) {
                    ret();
                } else {
                    ret(llvm::UndefValue::get(retTy));
                }
            }
            if (options.g) {
                di->finalizeSubprogram(SP);
                di_lex_scopes.pop_back();
            }
            this->labels.clear();
            this->currentfunction = nullptr;
        } break;
        case SReturn: ret(s->ret ? gen(s->ret) : nullptr); break;
        case SNamedLabel:
        case SLabel: {
            auto BB = labels[s->label];
            if (!getTerminator())
                br(BB);
            after(BB);
            BB->insertInto(currentfunction);
            if (options.g && s->k == SNamedLabel) {
                BB->setName(s->labelName->getKey());
            }
        } break;
        case SGotoWithLoc:
        case SGotoWithLocName:
        case SGoto:
            if (!getTerminator())
                br(labels[s->location]);
            break;
        case SCondJump: {
            auto cond = gen_cond(s->test);
            if (!getTerminator())
                condbr(cond, labels[s->T], labels[s->F]);
        } break;
        case SAsm:
            if (currentfunction)
                module->appendModuleInlineAsm(s->asms);
            else {
                auto ty = llvm::FunctionType::get(type_cache.void_type, false);
                auto f = llvm::InlineAsm::get(ty, s->asms, StringRef(), true);
                call(ty, f);
            }
            break;
        case SVarDecl: {
            for (const auto &it : s->vars) {
                IdentRef name = it.name;
                CType varty = it.ty;
                Expr init = it.init;
                unsigned idx = it.idx;
                if (options.g) setDebugLoc(it.loc);
                auto align = varty->getAlignAsMaybeAlign();
                if (varty->hasTag(TYTYPEDEF)) {
                    /* nothing */
                } else if (varty->getKind() == TYFUNCTION) {
                    newFunction(cast<llvm::FunctionType>(wrap(varty)), name, varty->getFunctionAttrTy()->getTags(),
                                idx);
                } else if (!currentfunction || varty->isGlobalStorage()) {
                    auto GV = module->getGlobalVariable(name->getKey(), true);
                    if (GV) {
                        auto L = GV->getLinkage();
                        if (!varty->hasTag(TYEXTERN)) {
                            if (L == ExternalLinkage) {
                                if (varty->hasTag(TYSTATIC))
                                    GV->setLinkage(InternalLinkage); // make it from extern declaration to static
                                else
                                    GV->setLinkage(ExternalLinkage); // a definition!
                            }
                        }
                        if (init) {
                            GV->setInitializer(cast<llvm::Constant>(gen(init))); // now update initializer
                            if (!varty->hasTag(TYSTATIC))
                                GV->setLinkage(ExternalLinkage); // a definition!
                        }
                        break;
                    }
                    const type_tag_t tags = varty->getTags();
                    llvm::Type *ty = wrap(varty);
                    // translate "extern void" to "extern char"
                    if (LLVM_UNLIKELY(ty->isVoidTy())) {
                        ty = type_cache.integer_types[3];
                    }
                    GV =
                        new llvm::GlobalVariable(*module, ty, tags & TYCONST, ExternalLinkage, nullptr, name->getKey());
                    GV->setAlignment(align);
                    GV->setAlignment(options.DL.getPreferredAlign(GV));
                    if (!(tags & TYEXTERN)) {
                        llvm::Constant *ginit = 
                           init ? 
                           (init->k == EInitList) ? 
                                buildAggGlobalInit(init, ty) : 
                                cast<llvm::Constant>(gen(init)) : 
                            llvm::Constant::getNullValue(ty);
                        GV->setInitializer(ginit), GV->setDSOLocal(true);
                    }
                    if (tags & TYTHREAD_LOCAL)
                        GV->setThreadLocal(true);
                    if (tags & TYSTATIC)
                        GV->setLinkage(InternalLinkage);
                    else if (tags & TYEXTERN)
                        GV->setLinkage(ExternalLinkage);
                    else if (!init && !(tags & TYCONST))
                        GV->setLinkage(CommonLinkage);
                    if (options.g) {
                        llvm::DIGlobalVariableExpression *GVE = di->createGlobalVariableExpression(getLexScope(), name->getKey(), getLinkageName(varty->getTags()), getFile(), getLine(), wrapDIType(varty), false, GV->isDeclaration());
                        GV->addDebugInfo(GVE);
                    }
                    vars[idx] = GV;
                } else {
                    if (LLVM_UNLIKELY(scope_index_is_unnamed_alloca(idx))) {
                        if (init->k == EInitList) {
                            scope_index_restore_unnamed_alloca(idx);
                        } else {
                            gen(init);
                            break;
                        }
                    }
                    bool isVLA = false;
                    llvm::Value *alloca_size;
                    llvm::Type *ty;
                    llvm::MaybeAlign Align;
                    Align = varty->getAlignAsMaybeAlign();
                    if (LLVM_UNLIKELY(varty->isVLA())) {
                        // llvm.stackrestore()
                        // llvm.stacksave()
                        isVLA = true;
                        const std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(varty);
                        alloca_size = pair.first;
                        ty = pair.second;
                    } else {
                        ty = wrap(varty);
                        alloca_size = nullptr;
                    }
                    if (!Align)
                        Align = options.DL.getPrefTypeAlign(ty);
                    llvm::AllocaInst *val = new llvm::AllocaInst(ty, options.DL.getAllocaAddrSpace(), alloca_size,
                                                                 *Align, name->getKey());
                    if (LLVM_UNLIKELY(isVLA)) {
                        insertBB->getInstList().push_back(val);
                        Insert(val);
                    } else {
                        auto &instList = currentfunction->getEntryBlock().getInstList();
                        instList.push_front(val);
                    }
                    if (options.g) {
                        llvm::DILocalVariable *var = di->createAutoVariable(getLexScope(), name->getKey(), getFile(), getLine(), wrapDIType(varty));
                        di->insertDeclare(val, var, di->createExpression(), di_cur_location, insertBB);
                    }
                    vars[idx] = val;
                    if (align.hasValue())
                        val->setAlignment(align.valueOrOne());
                    if (init) {
                        if (init->k == EInitList) {
                            buildAggLocalInit(ty, init, val);
                        } else {
                            auto initv = gen(init);
                            store(val, initv, align);
                        }
                    }
                }
            }
        } break;
        }
    }
    llvm::Constant *buildAggGlobalInit(Expr e, llvm::Type *T) {
        llvm_unreachable("un-implemented");
        return nullptr;
    }
    void buildAggLocalInit(llvm::Type *T, Expr e, llvm::Value *val) {
        SmallVector<unsigned> idxs;
        for (const Initializer &it : e->inits) {
            ArrayRef<Designator> designators = it.getDesignatorsOneOrMore();
            idxs.resize_for_overwrite(designators.size());
            for (size_t i = 0;i < designators.size();++i)
                idxs[i] = designators[i].getStart();
            llvm::Value *p = structGEP(T, val, idxs);
            if (it.value->k == EInitList) {
                llvm::Type *elemTy = wrap(it.value->ty);
                buildAggLocalInit(elemTy, it.value, p);
            } else {
                llvm::Value *v = gen(it.value);
                store(p, v);
            }
            idxs.clear();
        }
    }
    llvm::GlobalVariable *CreateGlobalString(StringRef bstr, llvm::Type *&ty) {
        auto str = llvm::ConstantDataArray::getString(getLLVMContext(), bstr, true);
        ty = str->getType();
        auto GV = new llvm::GlobalVariable(*module, ty, true, PrivateLinkage, str);
        GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        GV->setAlignment(llvm::Align(1));
        return GV;
    }
    llvm::Value *subscript(Expr e, llvm::Type *&ty) {
        ty = wrap(e->left->ty->p);
        llvm::Value *v = gen(e->left);
        llvm::Value *r = gen(e->right);
        return gep(ty, v, {r});
    }
    llvm::Type* wrap(CType ty) {
        return type_cache.wrap(ty);
    }
    llvm::StructType *wrapComplex(const_CType ty) {
        return type_cache.wrapComplex(ty);
    }
    llvm::StructType *wrapComplexForInteger(const_CType ty) {
        return type_cache.wrapComplexForInteger(ty);
    }
    llvm::Value *getAddress(Expr e) {
        assert(e);
        if (options.g) setDebugLoc(e->getBeginLoc());
        switch (e->k) {
        case EVar: return vars[e->sval];
        case EMemberAccess: {
            llvm::Value *agg = getAddress(e->obj);
            llvm::Type *ty = wrap(e->obj->ty);
            return structGEP(ty, agg, e->idxs);
        }
        case EUnary:
            switch (e->uop) {
            case AddressOf: return getAddress(e->uoperand);
            case Dereference: return gen(e->uoperand);
            case C__real__:
            case C__imag__: {
                llvm::Type *ty = wrap(e->uoperand->ty);
                return gep(ty, getAddress(e->uoperand), {type_cache.i32_0, e->uop == C__real__ ? type_cache.i32_0 : type_cache.i32_1});
            }
            default: llvm_unreachable("");
            }
        case ESubscript: {
            llvm::Type *ty;
            return subscript(e, ty);
        }
        case EArrToAddress: return getAddress(e->voidexpr);
        case EString:
            return createString(e);
        default:
            if (e->ty->hasTag(TYREPLACED_CONSTANT)) {
                auto e2 = reinterpret_cast<ReplacedExpr *>(e);
                return vars[e2->id];
            }
#if CC_DEBUG
            llvm::errs() << "unable to generate address for expression " << e << " type = " << e->ty;
#endif
            llvm_unreachable("");
        }
    }
    llvm::Value *genVLANumelements(Expr e) {
        auto it = type_cache.vla_size_map.insert(std::make_pair(e, nullptr));
        if (it.second)
            it.first->second = gen(e);
        return it.first->second;
    }
    llvm::Value *genVLANumelements(CType ty) {
        assert(ty->isVLA());
        return genVLANumelements(ty->vla_expr);
    }
    std::pair<llvm::Value *, llvm::Type *> genVLASizeof(Expr e, CType elementType) {
        llvm::Value *numElements = genVLANumelements(e);
        if (elementType->getKind() == TYVLA) {
            const auto pair = genVLASizeof(elementType->vla_expr, elementType->vla_arraytype);
            llvm::Instruction *Val = mul(numElements, pair.first);
            Val->setHasNoSignedWrap(true);
            return std::make_pair(Val, pair.second);
        }
        return std::make_pair(numElements, wrap(elementType));
    }
    std::pair<llvm::Value *, llvm::Type *> genVLASizeof(CType ty) {
        assert(ty->isVLA());
        return genVLASizeof(ty->vla_expr, ty->vla_arraytype);
    }
    llvm::Value *gen(Expr e) {
        assert(e);
        if (options.g) setDebugLoc(e->getBeginLoc());
        switch (e->k) {
        case EConstant: return e->C;
        case EInitList:
        {
            llvm_unreachable("un-implemented");
            return nullptr;
        } break;
        case EBuiltinCall: {
            StringRef Name = e->builtin_func_name->getKey();
            StringRef Prefix = llvm::Triple::getArchTypePrefix(options.triple.getArch());
            unsigned ID = llvm::Intrinsic::getIntrinsicForClangBuiltin(Prefix.data(), Name);
            if (ID == llvm::Intrinsic::not_intrinsic)
                ID = llvm::Intrinsic::getIntrinsicForMSBuiltin(Prefix.data(), Name);
            llvm::Function *F = llvm::Intrinsic::getDeclaration(module.get(), ID);
            return F;
        }
        case EMemberAccess: 
        {

            llvm::Type *ty = wrap(e->obj->ty);
            if (e->obj->k == EUnary && e->obj->uop == Dereference) {
                return structGEP(ty, getAddress(e->obj), e->idxs);
            }
            return extractValue(gen(e->obj), e->idxs);
        }
        case EConstantArraySubstript:
            return llvm::ConstantExpr::getInBoundsGetElementPtr(wrap(e->ty->p), e->array,
                                                                ConstantInt::get(type_cache.intptrTy, e->cidx));
        case EPostFix: {
            auto p = getAddress(e->poperand);
            auto ty = wrap(e->ty);
            auto a = e->poperand->ty->getAlignAsMaybeAlign();
            auto r = load(p, ty, a);
            llvm::Value *v;
            if (e->ty->getKind() == TYPOINTER) {
                ty = wrap(e->poperand->ty->p);
                v = gep(ty, r, {e->pop == PostfixIncrement ? type_cache.i32_1 : type_cache.i32_n1});
            } else if (e->ty->getKind() == TYPRIM && e->ty->isFloating()) {
                v = (e->pop == PostfixIncrement) ? fadd(r, llvm::ConstantFP::get(ty, 1.0))
                                                 : fsub(r, llvm::ConstantFP::get(ty, 1.0));
            } else {
                v = (e->pop == PostfixIncrement) ? add(r, llvm::ConstantInt::get(ty, 1))
                                                 : sub(r, llvm::ConstantInt::get(ty, 1));
            }
            store(p, v, a);
            return r;
        }
        case EArrToAddress: return getAddress(e->voidexpr);
        case ESubscript: {
            llvm::Type *ty;
            auto v = subscript(e, ty);
            return load(v, ty);
        }
        case EString: return createString(e);
        case EBin: {
            unsigned pop = 0;
            switch (e->bop) {
            default: break;
            case LogicalAnd:
                return gen_logical(e->lhs, e->rhs, true);
            case LogicalOr:
                return gen_logical(e->rhs, e->lhs, false);
            case Assign:
            {
                llvm::Value *basep = getAddress(e->lhs),
                            *rhs = gen(e->rhs);
                llvm::StoreInst *s = store(basep, rhs, e->lhs->ty->getAlignAsMaybeAlign());
                if (e->lhs->ty->hasTag(TYVOLATILE))
                    s->setVolatile(true);
                if (e->lhs->ty->hasTag(TYATOMIC))
                    s->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
                return rhs;
            }
            case AtomicrmwAdd: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Add); goto BINOP_ATOMIC_RMW;
            case AtomicrmwSub: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Sub); goto BINOP_ATOMIC_RMW;
            case AtomicrmwXor: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Xor); goto BINOP_ATOMIC_RMW;
            case AtomicrmwOr: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Or); goto BINOP_ATOMIC_RMW;
            case AtomicrmwAnd:
                pop = static_cast<unsigned>(llvm::AtomicRMWInst::And);
                goto BINOP_ATOMIC_RMW;
BINOP_ATOMIC_RMW:
{
                llvm::Value *addr = getAddress(e->lhs), *rhs = gen(e->rhs);
                llvm::MaybeAlign Align = e->rhs->ty->getAlignAsMaybeAlign();
                if (!Align) {
                    Align = llvm::Align(options.DL.getTypeStoreSize(rhs->getType()));
                }
                return atomicRMW(static_cast<llvm::AtomicRMWInst::BinOp>(pop), addr, rhs, *Align,
                                         llvm::AtomicOrdering::SequentiallyConsistent);

}
            }
            llvm::Value *lhs = gen(e->lhs);
            llvm::Value *rhs = gen(e->rhs);
            switch (e->bop) {
            case LogicalOr:
            case LogicalAnd:
            case Assign:
            case AtomicrmwAdd:
            case AtomicrmwSub:
            case AtomicrmwXor:
            case AtomicrmwOr: 
            case AtomicrmwAnd:
                llvm_unreachable("invalid control follow");
            case SAdd:
            { 
                llvm::BinaryOperator *it = add(lhs, rhs);
                it->setHasNoSignedWrap(true);
                return it;
            }
            // clang::ComplexExprEmitter::EmitBinAdd
            case CAdd: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = add(a, c), *const r = add(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            case CFAdd: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = fadd(a, c), *const r = fadd(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            // clang::ComplexExprEmitter::EmitBinSub
            case CFSub: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = fsub(a, c), *const r = fsub(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            case CSub: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = sub(a, c), *const r = sub(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            // clang::ComplexExprEmitter::EmitBinMul
            case CFMul: {
                llvm::Value *LibCallI, *LibCallR;
                llvm::MDBuilder MDHelper(getLLVMContext());
                llvm::StructType *ty = cast<llvm::StructType>(wrap(e->ty));
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const AC = fmul(a, c), *const BD = fmul(b, d),
                                   *const AD = fmul(a, d), *const BC = fmul(b, c),
                                   *const ResR = fsub(AC, BD), *const ResI = fadd(AD, BC),
                                   *const IsRNaN = fcmp(llvm::FCmpInst::FCMP_UNO, ResR, ResR);
                llvm::BasicBlock *ContBB = newBB(), *INaNBB = newBB();
                llvm::BranchInst *Branch = condbr(IsRNaN, INaNBB, ContBB);
                llvm::MDNode *BrWeight = MDHelper.createBranchWeights(1, (1U << 20) - 1);
                llvm::BasicBlock *OrigBB = Branch->getParent();
                Branch->setMetadata(LLVMContext::MD_prof, BrWeight);
                append(INaNBB);
                llvm::Value *const IsINaN = fcmp(llvm::FCmpInst::FCMP_UNO, ResI, ResI);
                llvm::BasicBlock *LibCallBB = newBB();
                Branch = condbr(IsINaN, LibCallBB, ContBB);
                Branch->setMetadata(LLVMContext::MD_prof, BrWeight);
                append(LibCallBB);
                {
                    llvm::Type *real_ty = ty->getTypeAtIndex((unsigned)0);
                    StringRef LibCallName;
                    switch (real_ty->getTypeID()) {
                    default: llvm_unreachable("Unsupported floating point type!");
                    case llvm::Type::HalfTyID: LibCallName = "__mulhc3"; break;
                    case llvm::Type::FloatTyID: LibCallName = "__mulsc3"; break;
                    case llvm::Type::DoubleTyID: LibCallName = "__muldc3"; break;
                    case llvm::Type::PPC_FP128TyID: LibCallName = "__multc3"; break;
                    case llvm::Type::X86_FP80TyID: LibCallName = "__mulxc3"; break;
                    case llvm::Type::FP128TyID: LibCallName = "__multc3"; break;
                    }
                    ArrayRef<llvm::Type *> Params = {real_ty, real_ty, real_ty, real_ty};
                    ArrayRef<llvm::Value *> Args = {a, b, c, d};
                    llvm::FunctionType *FTY = llvm::FunctionType::get(ty, Params, false);
                    llvm::AttributeList attrs = llvm::AttributeList::get(
                        getLLVMContext(), llvm::AttributeList::FunctionIndex,
                        {llvm::Attribute::NoUnwind, llvm::Attribute::ReadNone, llvm::Attribute::MustProgress,
                         llvm::Attribute::WillReturn, llvm::Attribute::NoRecurse, llvm::Attribute::NoSync,
                         llvm::Attribute::NoFree});
                    llvm::FunctionCallee F_C = module->getOrInsertFunction(LibCallName, FTY, attrs);
                    llvm::CallInst *LibCallRes = call(F_C, Args);
                    LibCallR = gen_complex_real(LibCallRes);
                    LibCallI = gen_complex_imag(LibCallRes);
                }
                br(ContBB);
                append(ContBB);
                llvm::PHINode *RealPHI = phi(ResR->getType(), 3);
                RealPHI->addIncoming(ResR, OrigBB);
                RealPHI->addIncoming(ResR, INaNBB);
                RealPHI->addIncoming(LibCallR, LibCallBB);
                llvm::PHINode *ImagPHI = phi(ResI->getType(), 3);
                ImagPHI->addIncoming(ResI, OrigBB);
                ImagPHI->addIncoming(ResI, INaNBB);
                ImagPHI->addIncoming(LibCallI, LibCallBB);
                return make_complex_pair(ty, RealPHI, ImagPHI);
            }
            case CMul: {
                // (a + ib) * (c + id) = (a * c - b * d) + i(a * d + b * c)
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs);
                return make_complex_pair(wrapComplexForInteger(e->ty),
                                         sub(mul(a, c), mul(b, d)),
                                         sub(mul(a, d), mul(b, c)));
            }
            // clang::ComplexExprEmitter::EmitBinDiv
            case CSDiv: {
                // (a+ib) / (c+id) = ((ac+bd)/(cc+dd)) + i((bc-ad)/(cc+dd))
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const cc = mul(c, c), *const dd = mul(d, d),
                                   *const cc_plus_dd = add(cc, dd), *const ac = add(a, c),
                                   *const bd = add(b, d), *const bc = add(b, c),
                                   *const ad = add(a, d),
                                   *const L = sdiv(add(ac, bd), cc_plus_dd),
                                   *const R = sdiv(mul(bc, ad), cc_plus_dd);
                return make_complex_pair(wrapComplexForInteger(e->ty), L, R);
            }
            case CUDiv: {
                // (a+ib) / (c+id) = ((ac+bd)/(cc+dd)) + i((bc-ad)/(cc+dd))
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const cc = mul(c, c), *const dd = mul(d, d),
                                   *const cc_plus_dd = add(cc, dd), *const ac = add(a, c),
                                   *const bd = add(b, d), *const bc = add(b, c),
                                   *const ad = add(a, d),
                                   *const L = udiv(sub(ac, bd), cc_plus_dd),
                                   *const R = udiv(sub(bc, ad), cc_plus_dd);
                return make_complex_pair(wrapComplexForInteger(e->ty), L, R);
            }
            case CFDiv: {
                llvm::StructType *ty = cast<llvm::StructType>(wrapComplex(e->ty));
                llvm::Type *real_ty = ty->getTypeAtIndex((unsigned)0);
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs);
                StringRef LibCallName;
                switch (real_ty->getTypeID()) {
                default: llvm_unreachable("Unsupported floating point type!");
                case llvm::Type::HalfTyID: LibCallName = "__divhc3"; break;
                case llvm::Type::FloatTyID: LibCallName = "__divsc3"; break;
                case llvm::Type::DoubleTyID: LibCallName = "__divdc3"; break;
                case llvm::Type::PPC_FP128TyID: LibCallName = "__divtc3"; break;
                case llvm::Type::X86_FP80TyID: LibCallName = "__divxc3"; break;
                case llvm::Type::FP128TyID: LibCallName = "__divtc3"; break;
                }
                ArrayRef<llvm::Type *> Params = {real_ty, real_ty, real_ty, real_ty};
                ArrayRef<llvm::Value *> Args = {a, b, c, d};
                llvm::FunctionType *FTY = llvm::FunctionType::get(ty, Params, false);
                llvm::AttributeList attrs = llvm::AttributeList::get(
                    getLLVMContext(), llvm::AttributeList::FunctionIndex,
                    {llvm::Attribute::NoUnwind, llvm::Attribute::ReadNone, llvm::Attribute::MustProgress,
                     llvm::Attribute::WillReturn, llvm::Attribute::NoRecurse, llvm::Attribute::NoSync,
                     llvm::Attribute::NoFree});
                llvm::FunctionCallee F_C = module->getOrInsertFunction(LibCallName, FTY, attrs);
                return call(F_C, Args);
            }
            case CEQ: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs);
                llvm::Value *L, *R;
                if (e->lhs->ty->isFloating()) {
                    L = fcmp(llvm::FCmpInst::FCMP_OEQ, a, c);
                    R = fcmp(llvm::FCmpInst::FCMP_OEQ, b, d);
                } else {
                    L = icmp(llvm::ICmpInst::ICMP_EQ, a, c);
                    R = icmp(llvm::ICmpInst::ICMP_EQ, b, d);
                }
                return createAnd(L, R);
            }
            case CNE: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs);
                llvm::Value *L, *R;
                if (e->lhs->ty->isFloating()) {
                    L = fcmp(llvm::FCmpInst::FCMP_ONE, a, c);
                    R = fcmp(llvm::FCmpInst::FCMP_ONE, b, d);
                } else {
                    L = icmp(llvm::ICmpInst::ICMP_NE, a, c);
                    R = icmp(llvm::ICmpInst::ICMP_NE, b, d);
                }
                return createOr(L, R);
            }
            case SSub: {
                llvm::BinaryOperator *it = sub(lhs, rhs);
                it->setHasNoSignedWrap(true);
                return it;
            }
            case SMul: {
                llvm::BinaryOperator *it = mul(lhs, rhs);
                it->setHasNoSignedWrap(true);
                return it;
            }
            case PtrDiff: {
                CType target = e->lhs->ty->p;
                llvm::Value *dividend;

                lhs = ptrtoint(lhs, type_cache.intptrTy);
                rhs = ptrtoint(rhs, type_cache.intptrTy);
                llvm::BinaryOperator *diff = sub(lhs, rhs);

                if (LLVM_UNLIKELY(target->isVLA())) {
                    std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(target);
                    uint64_t Size = type_cache.getsizeof(pair.second);
                    if (Size == 1) {
                        dividend = mul(pair.first, llvm::ConstantInt::get(type_cache.intptrTy, Size));
                        cast<llvm::Instruction>(dividend)->setHasNoSignedWrap(true);
                    } else {
                        dividend = pair.first;
                    }
                } else {
                    uint64_t Size = type_cache.getsizeof(target);
                    if (Size == 1)
                        return diff;
                    dividend = llvm::ConstantInt::get(type_cache.intptrTy, Size);
                }
                llvm::BinaryOperator *it = sdiv(diff, dividend);
                it->setIsExact(true);
                return it;
            }
            case SAddP: {
                if (const ConstantInt *CI = dyn_cast<ConstantInt>(rhs))
                    if (CI->isZero())
                        return lhs;
                CType target = e->ty->p;
                llvm::Type *T;
                if (LLVM_UNLIKELY(target->isVLA())) {
                    const std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(target);
                    T = pair.second;
                    if (const ConstantInt *CI = dyn_cast<ConstantInt>(rhs)) {
                        if (CI->getLimitedValue() == 1) {
                            rhs = pair.first;
                            goto VLA_NEXT;
                        }
                    }
                    rhs = mul(pair.first, rhs);
                    cast<llvm::Instruction>(rhs)->setHasNoSignedWrap(true);
                } else {
                    T = wrap(target);
                }
VLA_NEXT:;
                return gep(T, lhs, {rhs});
            }
            case Complex_CMPLX: return make_complex_pair(e->ty, lhs, rhs);
            case EQ: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_EQ); goto BINOP_ICMP;
            case NE: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_NE); goto BINOP_ICMP;
            case UGT: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_UGT); goto BINOP_ICMP;
            case UGE: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_UGE); goto BINOP_ICMP;
            case ULT: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_ULT); goto BINOP_ICMP;
            case ULE: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_ULE); goto BINOP_ICMP;
            case SGT: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SGT); goto BINOP_ICMP;
            case SGE: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SGE); goto BINOP_ICMP;
            case SLT: pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SLT); goto BINOP_ICMP;
            case SLE:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SLE);
                goto BINOP_ICMP;
BINOP_ICMP:
                return icmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
            case FEQ: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OEQ); goto BINOP_FCMP;
            case FNE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_ONE); goto BINOP_FCMP;
            case FGT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGT); goto BINOP_FCMP;
            case FGE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGE); goto BINOP_FCMP;
            case FLT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLT); goto BINOP_FCMP;
            case FLE:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLE);
                goto BINOP_FCMP;
BINOP_FCMP:
                return fcmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
            case Comma: return rhs;
            case UAdd: pop = llvm::Instruction::Add; goto BINOP_ARITH;
            case FAdd: pop = llvm::Instruction::FAdd; goto BINOP_ARITH;
            case USub: pop = llvm::Instruction::Sub; goto BINOP_ARITH;
            case FSub: pop = llvm::Instruction::FSub; goto BINOP_ARITH;
            case UMul: pop = llvm::Instruction::Mul; goto BINOP_ARITH;
            case FMul: pop = llvm::Instruction::FMul; goto BINOP_ARITH;
            case UDiv: pop = llvm::Instruction::UDiv; goto BINOP_ARITH;
            case SDiv: pop = llvm::Instruction::SDiv; goto BINOP_ARITH;
            case FDiv: pop = llvm::Instruction::FDiv; goto BINOP_ARITH;
            case URem: pop = llvm::Instruction::URem; goto BINOP_ARITH;
            case SRem: pop = llvm::Instruction::SRem; goto BINOP_ARITH;
            case FRem: pop = llvm::Instruction::FRem; goto BINOP_ARITH;

            case Shr: pop = llvm::Instruction::LShr; goto BINOP_SHIFT;
            case AShr: pop = llvm::Instruction::AShr; goto BINOP_SHIFT;
            case Shl: pop = llvm::Instruction::Shl; goto BINOP_SHIFT;

            case And: pop = llvm::Instruction::And; goto BINOP_BITWISE;
            case Xor: pop = llvm::Instruction::Xor; goto BINOP_BITWISE;
            case Or:
                pop = llvm::Instruction::Or;
                goto BINOP_BITWISE;
BINOP_ARITH:
BINOP_BITWISE:
BINOP_SHIFT:
                return binop(static_cast<llvm::Instruction::BinaryOps>(pop), lhs, rhs);
            }
            llvm_unreachable("invalid BinOp");
        }
        case EVoid: return (void)gen(e->voidexpr), nullptr;
        case EBlockAddress:
            assert(labels[e->addr] && "block address is nullptr");
            return llvm::BlockAddress::get(currentfunction, labels[e->addr]);
        case EUnary: {
            if (e->uop == AddressOf)
                return getAddress(e->uoperand);
            if (e->uop == ToBool)
                return gen_cond(e->uoperand);
            llvm::Value *Val = gen(e->uoperand);
            switch (e->uop) {
            case SNeg:
            {
                llvm::BinaryOperator *it = neg(Val);
                it->Instruction::setHasNoSignedWrap(true);
                return it;
            }
            case UNeg: return neg(Val);
            case FNeg: return fneg(Val);
            case Not: return createNot(Val);
            case Dereference: {
                llvm::Type *ty = wrap(e->ty);
                llvm::LoadInst *r = load(Val, ty);
                type_tag_t tags = e->uoperand->ty->p->getTags();
                if (tags & TYVOLATILE)
                    r->setVolatile(true);
                if (tags & TYATOMIC)
                    r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
                return r;
            }
            case LogicalNot: return createLogicalNot(Val);
            // clang::ComplexExprEmitter::VisitUnaryMinus
            case CNeg: {
                llvm::Value *l, *r;
                if (e->ty->isFloating()) {
                    l = fneg(gen_complex_real(Val));
                    r = fneg(gen_complex_imag(Val));
                } else {
                    l = neg(gen_complex_real(Val));
                    r = neg(gen_complex_imag(Val));
                }
                return make_complex_pair(e->ty, l, r);
            }
            // clang::ComplexExprEmitter::VisitUnaryNot
            case CConj: {
                llvm::Value *r = gen_complex_imag(Val);
                return make_complex_pair(e->ty, gen_complex_real(Val),
                                         e->ty->isFloating() ? static_cast<llvm::Instruction*>(fneg(r)) : static_cast<llvm::Instruction*>(neg(r)));
            }
            case C__real__: return gen_complex_real(Val);
            case C__imag__: return gen_complex_imag(Val);
            case ToBool:
            case AddressOf: llvm_unreachable("");
            }
            llvm_unreachable("invalid UnaryOp");
        }
        case EVar: {
            llvm::Value *pvar = vars[e->sval];
            llvm::LoadInst *r = load(pvar, wrap(e->ty), e->ty->getAlignAsMaybeAlign());
            if (e->ty->hasTag(TYVOLATILE))
                r->setVolatile(true);
            if (e->ty->hasTag(TYATOMIC))
                r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
            return r;
        }
        case ECondition: {
            if (e->ty->hasTag(TYVOID)) {
                llvm::BasicBlock *iftrue = addBB(),
                                 *iffalse = newBB(), *ifend = newBB();
                condbr(gen_cond(e->cond), iftrue, iffalse);
                after(iftrue);
                (void)gen(e->cleft);
                br(ifend);
                append(iffalse);
                (void)gen(e->cright);
                append(ifend);
                return nullptr;
            }
            if (e->cleft->k == EConstant && e->cright->k == EConstant)
                return select(gen_cond(e->cond), gen(e->cleft), gen(e->cright));
            llvm::Type *ty = wrap(e->cleft->ty);
            llvm::BasicBlock *iftrue = addBB(),
                             *iffalse = newBB(), *ifend = newBB();
            condbr(gen_cond(e->cond), iftrue, iffalse);

            after(iftrue);
            llvm::Value *left = gen(e->cleft);
            br(ifend);

            append(iffalse);
            llvm::Value *right = gen(e->cright);
            br(ifend);

            append(ifend);
            llvm::PHINode *Phi = phi(ty, 2);

            Phi->addIncoming(left, iftrue);
            Phi->addIncoming(right, iffalse);
            return Phi;
        }
        case ESizeof: {
            std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(e->theType);
            if (pair.second->isIntegerTy(1))
                return pair.first;
            llvm::BinaryOperator *it = mul(pair.first, /*llvm::ConstantExpr::getSizeOf(pair.second)*/ llvm::ConstantInt::get(
                                      type_cache.intptrTy, type_cache.getsizeof(pair.second)));
            it->setHasNoSignedWrap(true);
            return it;
        }
        case ECast: return createCast(getCastOp(e->castop), gen(e->castval), wrap(e->ty));
        case ECall: {
            llvm::Value *f;
            llvm::FunctionType *ty = cast<llvm::FunctionType>(wrap(e->callfunc->ty->p));
            if (e->callfunc->k == EUnary) {
                assert(e->callfunc->uop == AddressOf);
                f = getAddress(e->callfunc);
            } else {
                f = gen(e->callfunc);
            }
            size_t l = e->callargs.size();
            llvm::Value **buf = type_cache.alloc.Allocate<llvm::Value *>(l);
            for (size_t i = 0; i < l; ++i)
                buf[i] = gen(e->callargs[i]); // eval argument from left to right
            llvm::CallInst *r = call(ty, f, ArrayRef<llvm::Value *>(buf, l));
            return r;
        }
        default: llvm_unreachable("bad enum kind!");
        }
    }
    void initDebugInfo() {
        vla_expr_count = 0;
        assert(options.g && "Not in debug mode");
        di = new llvm::DIBuilder(*module);
        module->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
        module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
        di_basic_types[LLVMTypeConsumer::voidty] = nullptr;
        di_basic_types[LLVMTypeConsumer::i1ty] = di->createBasicType("_Bool", 1, llvm::dwarf::DW_ATE_boolean);
        di_basic_types[LLVMTypeConsumer::i8ty] = di->createBasicType("char", 8, llvm::dwarf::DW_ATE_signed_char);
        di_basic_types[LLVMTypeConsumer::u8ty] = di->createBasicType("unsigned char", 8, llvm::dwarf::DW_ATE_unsigned_char);
        di_basic_types[LLVMTypeConsumer::i16ty] = di->createBasicType("short", 16, llvm::dwarf::DW_ATE_signed);
        di_basic_types[LLVMTypeConsumer::u16ty] = di->createBasicType("unsigned short", 16, llvm::dwarf::DW_ATE_unsigned);
        di_basic_types[LLVMTypeConsumer::i32ty] = di->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
        di_basic_types[LLVMTypeConsumer::u32ty] = di->createBasicType("unsigned", 32, llvm::dwarf::DW_ATE_unsigned);
        di_basic_types[LLVMTypeConsumer::i64ty] = di->createBasicType("long long", 64, llvm::dwarf::DW_ATE_signed);
        di_basic_types[LLVMTypeConsumer::u64ty] = di->createBasicType("unsigned long long", 64, llvm::dwarf::DW_ATE_unsigned);
        di_basic_types[LLVMTypeConsumer::i128ty] = di->createBasicType("__int128", 128, llvm::dwarf::DW_ATE_unsigned);
        di_basic_types[LLVMTypeConsumer::u128ty] = di->createBasicType("unsigned __int128", 128, llvm::dwarf::DW_ATE_unsigned);
        di_basic_types[LLVMTypeConsumer::bfloatty] = di->createBasicType("__bf16", 16, llvm::dwarf::DW_ATE_float);
        di_basic_types[LLVMTypeConsumer::halfty] = di->createBasicType("Half", 16, llvm::dwarf::DW_ATE_float);
        di_basic_types[LLVMTypeConsumer::floatty] = di->createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
        di_basic_types[LLVMTypeConsumer::doublety] = di->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
        di_basic_types[LLVMTypeConsumer::fdecimal32ty] = di->createBasicType("_Decimal32", 32, llvm::dwarf::DW_ATE_decimal_float);
        di_basic_types[LLVMTypeConsumer::fdecimal64ty] = di->createBasicType("_Decimal64", 64, llvm::dwarf::DW_ATE_decimal_float);
        di_basic_types[LLVMTypeConsumer::fdecimal128ty] = di->createBasicType("_Decimal128", 128, llvm::dwarf::DW_ATE_decimal_float);
        di_basic_types[LLVMTypeConsumer::ptrty] = di->createNullPtrType();

        di_cur_file = di->createFile(SM.getMainFileName(), "");

        CU = di->createCompileUnit(
            llvm::dwarf::DW_LANG_C99,
            di_cur_file,
            "xcc",
            options.OptimizationLevel != 0,
            "",
            0
        );
        // the file scope
        di_lex_scopes.push_back(di_cur_file);
    }
    llvm::DIType* wrapDIType(CType ty) {
        assert(ty);
        switch (ty->getKind()) {
            case TYPRIM:
                return di_basic_types[type_cache.getTypeIndex(ty)];
            case TYPOINTER:
                return di_basic_types[LLVMTypeConsumer::ptrty];
            case TYTAG:
                switch (ty->tag) {
                    case TagType_Enum:
                    {
                        auto it = di_composite_cache.insert(std::make_pair(ty, nullptr));
                        if (it.second) {
                            const EnumDecl &enumDecl = *ty->getEnum();
                            size_t Size = enumDecl.enums.size();
                            llvm::Metadata **Elements = type_cache.alloc.Allocate<llvm::Metadata*>(Size);
                            for (size_t i = 0;i < Size;++i)
                                Elements[i] = di->createEnumerator(enumDecl.enums[i].name->getKey(), enumDecl.enums[i].val);
                            it.first->second = di->createEnumerationType(
                                getLexScope(), 
                                ty->tag_name ? ty->tag_name->getKey() : StringRef(),
                                getFile(),
                                getLine(),
                                context.getEnumSizeInBits(),
                                0,
                                di->getOrCreateArray(ArrayRef<llvm::Metadata *>(Elements, Size)),
                                wrapDIType(context.getEnumRealType())
                            );
                        }
                        return it.first->second;
                    }
                    case TagType_Struct:
                    {
                        llvm_unreachable("TODO");
                    }
                    case TagType_Union:
                    {
                        llvm_unreachable("TODO");
                    }
                }
                llvm_unreachable("invalid Tag kind");
            case TYBITFIELD:
                return wrapDIType(ty->bittype);
            case TYARRAY:
            {
                uint64_t size = type_cache.getsizeof(ty);
                llvm::DINodeArray subscripts = di->getOrCreateArray(
                    {
                        di->getOrCreateSubrange(0, ty->hassize ? (int64_t)ty->arrsize : (int64_t)-1)
                    }
                );
                return di->createArrayType(size, type_cache.getAlignof(ty) * 8, wrapDIType(ty->arrtype), subscripts);
            }
            case TYFUNCTION:
            {
                auto it = di_subroutine_type_cache.insert(std::make_pair(ty, nullptr));
                if (it.second) {
                    size_t Size = ty->params.size() + 1;
                    llvm::Metadata **tys = type_cache.alloc.Allocate<llvm::Metadata*>(Size);
                    tys[0] = wrapDIType(ty->ret);
                    for (size_t i = 0;i < ty->params.size();i++) 
                        tys[i + 1] = wrapDIType(ty->params[i].ty);
                    it.first->second = di->createSubroutineType(di->getOrCreateTypeArray(makeArrayRef(tys, Size)));
                }
                return it.first->second;
            }
            case TYVLA:
            {
                assert(module && "VLA is not allowed in file scope");
                llvm::DIType *T = wrapDIType(ty->vla_arraytype);
                auto it = di_vla_expr_cache.insert(std::make_pair(ty->vla_expr, nullptr));
                if (it.second) {
                    char Name[20];
                    int N = snprintf(Name, sizeof(Name), "__vla_expr%u", vla_expr_count++);
                    it.first->second = di->createAutoVariable(
                        getLexScope(), StringRef(Name, N), getFile(), 
                        current_line, wrapDIType(ty->vla_expr->ty), false,  llvm::DINode::FlagArtificial);
                }
                llvm::DINodeArray subscripts = di->getOrCreateArray({di->getOrCreateSubrange(0, it.first->second)});
                return di->createArrayType(0, 0, T, subscripts);
            }
            case TYBITINT:
                return wrapDIType(ty->bitint_base);
        }
        llvm_unreachable("invalid CType");
    }
  public:
    [[nodiscard]] std::unique_ptr<llvm::Module> takeModule() {
        return std::move(module);
    }
    [[nodiscard]] llvm::Module *getModule() const {
        return module.get();
    }
    llvm::Module *releaseModule() {
        return module.release();
    }
    [[nodiscard]] bool hasModule() const {
        return bool(module);
    }
    IRGen(SourceMgr &SM, xcc_context &context, const Options &options, LLVMTypeConsumer &type_cache)
     : DiagnosticHelper{SM}, type_cache{type_cache}, context{context}, SM{SM}, options{options} {}
    [[nodiscard]] std::unique_ptr<llvm::Module> run(const TranslationUnit &TU) {
        createModule();
        
        if (options.g)
            initDebugInfo();

        type_cache.reset(TU.max_tags_scope);

        vars = new llvm::Value *[TU.max_typedef_scope];

        runCodeGenTranslationUnit(TU.ast);

        RunOptimizationPipeline();

        delete [] vars;

        if (di) {
            di->finalize();
            delete di;
        }
        return std::move(module);
    }
};
