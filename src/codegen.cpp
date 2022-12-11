/*
 * codegen.cpp
 *
 * Code generation to LLVM IR, and Debug Info
 * implements getAlignof() and getsizeof() method(used by Sema and constant folding).
 */

struct IRGen : public DiagnosticHelper {
    static constexpr auto ExternalLinkage = llvm::GlobalValue::ExternalLinkage,
                          PrivateLinkage = llvm::GlobalValue::PrivateLinkage,
                          CommonLinkage = llvm::GlobalValue::CommonLinkage,
                          AppendingLinkage = llvm::GlobalValue::AppendingLinkage,
                          InternalLinkage = llvm::GlobalValue::InternalLinkage;
    static constexpr auto DefaultStorageClass = llvm::GlobalValue::DefaultStorageClass,
                          DLLImportStorageClass = llvm::GlobalValue::DLLImportStorageClass,
                          DLLExportStorageClass = llvm::GlobalValue::DLLExportStorageClass;
    LLVMTypeConsumer *type_cache;
    xcc_context &context;
    std::unique_ptr<llvm::Module> module;
    SourceMgr &SM;
    LLVMContext &ctx;
    llvm::IRBuilder<> B;
    llvm::TargetMachine *machine = nullptr;
    llvm::DataLayout *layout = nullptr;
    const Options &options;
    llvm::Triple triple;
    llvm::Function *currentfunction = nullptr;
    llvm::BasicBlock::iterator allocaInsertPt;
    // `i32 1/0/-1` constant
    llvm::ConstantInt *i32_1 = nullptr, *i32_0 = nullptr, *i32_n1 = nullptr;
    llvm::IntegerType *intptrTy = nullptr;
    unsigned pointerSizeInBits = 0;
    // LLVM false/true constant
    llvm::ConstantInt *i1_0 = nullptr, *i1_1 = nullptr;
    unsigned lastFileID = -1;
    llvm::DIFile *lastFile = nullptr;
    // jump labels
    llvm::SmallVector<llvm::BasicBlock *, 5> labels{};
    // struct/union
    llvm::Value **vars = nullptr;
    Stmt currentfunctionAST;
private:
    void store(llvm::Value *p, llvm::Value *v) { (void)B.CreateStore(v, p); }
    llvm::LoadInst *load(llvm::Value *p, llvm::Type *t) {
        assert(p);
        assert(t);
        llvm::LoadInst *i = B.CreateLoad(t, p, false);
        return i;
    }
    void store(llvm::Value *p, llvm::Value *v, llvm::Align align) {
        llvm::StoreInst *s = B.CreateStore(v, p);
        s->setAlignment(align);
    }
    llvm::LoadInst *load(llvm::Value *p, llvm::Type *t, llvm::Align align) {
        assert(p);
        assert(t);
        llvm::LoadInst *i = B.CreateLoad(t, p, false);
        i->setAlignment(align);
        return i;
    }
    void store(llvm::Value *p, llvm::Value *v, llvm::MaybeAlign align) {
        llvm::StoreInst *s = B.CreateStore(v, p);
        if (align.hasValue())
            s->setAlignment(*align);
    }
    llvm::LoadInst *load(llvm::Value *p, llvm::Type *t, llvm::MaybeAlign align) {
        assert(p);
        assert(t);
        llvm::LoadInst *i = B.CreateLoad(t, p, false);
        if (align.hasValue())
            i->setAlignment(*align);
        return i;
    }
    const llvm::Instruction *getTerminator() { return B.GetInsertBlock()->getTerminator(); }
    llvm::ValueAsMetadata *mdNum(uint64_t num) {
        llvm::Value *v = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), num);
        return llvm::ValueAsMetadata::get(v);
    }
    void append(llvm::BasicBlock *theBB) {
        currentfunction->getBasicBlockList().push_back(theBB);
        B.SetInsertPoint(theBB);
    }
    void after(llvm::BasicBlock *loc) { B.SetInsertPoint(loc); }
    llvm::Function *addFunction(llvm::FunctionType *ty, const Twine &N) {
        return llvm::Function::Create(ty, ExternalLinkage, N, module.get());
    }
    void handle_asm(StringRef s) {
        if (currentfunction)
            // module level asm
            module->appendModuleInlineAsm(s);
        else {
            auto ty = llvm::FunctionType::get(type_cache->void_type, false);
            auto f = llvm::InlineAsm::get(ty, s, StringRef(), true);
            B.CreateCall(ty, f);
        }
    }
    uint64_t getSizeInBits(llvm::Type *ty) { return layout->getTypeSizeInBits(ty); }
    uint64_t getSizeInBits(CType ty) { return getSizeInBits(wrap(ty)); }
    uint64_t getSizeInBits(Expr e) { return getSizeInBits(e->ty); }
    llvm::MDString *mdStr(StringRef str) { return llvm::MDString::get(ctx, str); }
    llvm::MDNode *mdNode(llvm::Metadata *m) { return llvm::MDNode::get(ctx, {m}); }
    llvm::MDNode *mdNode(ArrayRef<llvm::Metadata *> m) { return llvm::MDNode::get(ctx, m); }
    void createModule() {
        module.reset(new llvm::Module(options.mainFileName, ctx));
        module->setSourceFileName(options.mainFileName);
        module->setDataLayout(*layout);
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
    llvm::CodeGenOpt::Level getCGOptLevel() const {
        switch (options.OptimizationLevel) {
        default: llvm_unreachable("Invalid optimization level!");
        case 0: return llvm::CodeGenOpt::None;
        case 1: return llvm::CodeGenOpt::Less;
        case 2: return llvm::CodeGenOpt::Default; // O2/Os/Oz
        case 3: return llvm::CodeGenOpt::Aggressive;
        }
    }
    // clang::EmitAssemblyHelper::RunOptimizationPipeline
    void RunOptimizationPipeline() {
        llvm::Optional<llvm::PGOOptions> PGOOpt;
        machine->setPGOOption(PGOOpt);

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
        llvm::PassBuilder PB(machine, PTO, PGOOpt, &PIC);

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
    llvm::Value *gen_complex_real(llvm::Value *v) { return B.CreateExtractValue(v, {0}); }
    llvm::Value *gen_complex_imag(llvm::Value *v) { return B.CreateExtractValue(v, {1}); }
    llvm::Value *make_complex_pair(llvm::Type *ty, llvm::Value *a, llvm::Value *b) {
        auto T = cast<llvm::StructType>(ty);
        if (!currentfunction)
            return llvm::ConstantStruct::get(T, {cast<llvm::Constant>(a), cast<llvm::Constant>(b)});
        return B.CreateInsertValue(B.CreateInsertValue(llvm::PoisonValue::get(T), a, {0}), b, {1});
    }
    llvm::Value *make_complex_pair(CType ty, llvm::Value *a, llvm::Value *b) {
        return make_complex_pair(wrapComplex(ty), a, b);
    }
    llvm::Value *gen_cond(Expr e) {
        llvm::Value *V = gen(e);
        if (e->ty->isBool())
            return V;
        if (e->ty->isComplex()) {
            llvm::Value *a = gen_complex_real(V);
            llvm::Value *b = gen_complex_imag(V);
            if (e->ty->isFloating()) {
                auto zero = ConstantFP::getZero(a->getType());
                a = B.CreateFCmpUNO(a, zero);
                b = B.CreateFCmpUNO(b, zero);
            } else {
                auto zero = ConstantInt::getNullValue(a->getType());
                a = B.CreateICmpNE(a, zero);
                b = B.CreateICmpNE(b, zero);
            }
            return B.CreateOr(a, b);
        }
        auto T = V->getType();
        if (auto I = dyn_cast<llvm::IntegerType>(T))
            return B.CreateICmpNE(V, ConstantInt::get(I, 0));
        return B.CreateFCmpONE(V, ConstantFP::getZero(T, false));
    }
    llvm::PHINode *gen_logical(Expr lhs, Expr rhs, bool isand = true) {
        llvm::BasicBlock *rightBB = llvm::BasicBlock::Create(ctx);
        llvm::BasicBlock *phiBB = llvm::BasicBlock::Create(ctx);
        auto R = gen_cond(lhs);
        llvm::BasicBlock *leftBB = B.GetInsertBlock();

        isand ? B.CreateCondBr(R, rightBB, phiBB) : B.CreateCondBr(R, phiBB, rightBB);

        append(rightBB);
        auto L = gen_cond(rhs);
        // gen_cond may change the basic block
        rightBB = B.GetInsertBlock();
        B.CreateBr(phiBB);

        append(phiBB);
        auto phi = B.CreatePHI(type_cache->integer_types[0], 2);
        phi->addIncoming(R, leftBB);
        phi->addIncoming(L, rightBB);
        return phi;
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
                            sw->addCase(ConstantInt::get(ctx, APInt(it.CaseStart->getBitWidth(), i++)),
                                        labels[it.label]);
                        }
                        continue;
                    }
                    auto diff = B.CreateSub(cond, ConstantInt::get(ctx, *it.CaseStart));
                    auto c = B.CreateICmpULE(diff, ConstantInt::get(ctx, it.range));
                    auto BB = llvm::BasicBlock::Create(ctx, "", currentfunction);
                    B.CreateCondBr(c, labels[it.label], BB);
                    after(BB);
                }
            }
            if (s->switchs.empty()) {
                B.CreateBr(labels[s->sw_default]);
                return;
            }
            for (const auto &it : s->switchs)
                sw->addCase(ConstantInt::get(ctx, *it.CaseStart), labels[it.label]);
            B.Insert(sw);
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
            B.Insert(Inst);
        } break;
        case SHead: llvm_unreachable("");
        case SCompound: {
            for (Stmt ptr = s->inner; ptr; ptr = ptr->next)
                gen(ptr);
        } break;
        case SDecl: {
            type_cache->handleDecl(s);
        } break;
        case SNoReturnCall:
            (void)gen(s->call_expr);
            B.CreateUnreachable();
            break;
        case SExpr: (void)gen(s->exprbody); break;
        case SFunction: {
            assert(this->labels.empty());
            auto ty = cast<llvm::FunctionType>(wrap(s->functy));
            currentfunctionAST = s;
            currentfunction =
                newFunction(ty, s->funcname, s->functy->getFunctionAttrTy()->getTags(), s->func_idx, true);
            llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx, "entry");
            append(entry);
            statics("%u labels in function\n", s->numLabels);
            for (unsigned i = 0; i < s->numLabels; i++)
                this->labels.push_back(llvm::BasicBlock::Create(ctx));
            unsigned arg_no = 0;
            for (auto arg = currentfunction->arg_begin(); arg != currentfunction->arg_end(); ++arg) {
                auto pty = ty->getParamType(arg_no);
                auto p = B.CreateAlloca(pty, layout->getAllocaAddrSpace());
                store(p, arg);
                vars[s->args[arg_no++]] = p;
            }
            allocaInsertPt = entry->begin();
            for (Stmt ptr = s->funcbody->next; ptr; ptr = ptr->next)
                gen(ptr);
            if (!getTerminator()) {
                auto retTy = ty->getReturnType();
                if (retTy->isVoidTy()) {
                    B.CreateRetVoid();
                } else {
                    B.CreateRet(llvm::UndefValue::get(retTy));
                }
            }
            this->labels.clear();
            this->currentfunction = nullptr;
        } break;
        case SReturn: B.CreateRet(s->ret ? gen(s->ret) : nullptr); break;
        case SDeclOnly:
            break;
        case SNamedLabel:
        case SLabel: {
            auto BB = labels[s->label];
            if (!getTerminator())
                B.CreateBr(BB);
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
                B.CreateBr(labels[s->location]);
            break;
        case SCondJump: {
            auto cond = gen_cond(s->test);
            if (!getTerminator())
                B.CreateCondBr(cond, labels[s->T], labels[s->F]);
        } break;
        case SAsm:
            if (currentfunction)
                module->appendModuleInlineAsm(s->asms);
            else {
                auto ty = llvm::FunctionType::get(type_cache->void_type, false);
                auto f = llvm::InlineAsm::get(ty, s->asms, StringRef(), true);
                B.CreateCall(ty, f);
            }
            break;
        case SVarDecl: {
            for (const auto &it : s->vars) {
                auto name = it.name;
                auto varty = it.ty;
                auto init = it.init;
                auto idx = it.idx;
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
                    auto tags = varty->getTags();
                    llvm::Type *ty = wrap(varty);
                    llvm::Constant *ginit = init ? cast<llvm::Constant>(gen(init)) : llvm::Constant::getNullValue(ty);
                    GV =
                        new llvm::GlobalVariable(*module, ty, tags & TYCONST, ExternalLinkage, nullptr, name->getKey());
                    GV->setAlignment(align);
                    GV->setAlignment(layout->getPreferredAlign(GV));
                    if (!(tags & TYEXTERN))
                        GV->setInitializer(ginit), GV->setDSOLocal(true);
                    if (tags & TYTHREAD_LOCAL)
                        GV->setThreadLocal(true);
                    if (tags & TYSTATIC)
                        GV->setLinkage(InternalLinkage);
                    else if (tags & TYEXTERN)
                        GV->setLinkage(ExternalLinkage);
                    else if (!init && !(tags & TYCONST))
                        GV->setLinkage(CommonLinkage);
                    vars[idx] = GV;
                } else {
                    if (LLVM_UNLIKELY(scope_index_is_unnamed_alloca(idx))) {
                        assert(init);
                        gen(init);
                        break;
                    }
                    bool isVLA = false;
                    llvm::Value *alloca_size;
                    llvm::Type *ty;
                    if (LLVM_UNLIKELY(varty->isVLA())) {
                        // llvm.stackrestore()
                        // llvm.stacksave()
                        isVLA = true;
                        const std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(varty);
                        alloca_size = pair.first;
                        ty = pair.second;
                    } else {
                        alloca_size = nullptr;
                        ty = wrap(varty);
                    }
                    llvm::AllocaInst *val = new llvm::AllocaInst(ty, layout->getAllocaAddrSpace(), alloca_size,
                                                                 layout->getPrefTypeAlign(ty), name->getKey());
                    if (LLVM_UNLIKELY(isVLA)) {
                        B.Insert(val);
                    } else {
                        auto &instList = currentfunction->getEntryBlock().getInstList();
                        instList.push_front(val);
                    }
                    vars[idx] = val;
                    if (align.hasValue())
                        val->setAlignment(align.valueOrOne());
                    if (init) {
                        auto initv = gen(init);
                        store(val, initv, align);
                    }
                }
            }
        } break;
        }
    }
    llvm::Value *getStruct(Expr e) { return nullptr; }
    llvm::Value *getArray(Expr e) { return nullptr; }
    llvm::GlobalVariable *CreateGlobalString(StringRef bstr, llvm::Type *&ty) {
        auto str = llvm::ConstantDataArray::getString(ctx, bstr, true);
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
        return B.CreateInBoundsGEP(ty, v, {r});
    }
    llvm::Type* wrap(CType ty) {
        assert(type_cache);
        return type_cache->wrap(ty);
    }
    llvm::StructType *wrapComplex(const_CType ty) {
        assert(type_cache);
        return type_cache->wrapComplex(ty);
    }
    llvm::StructType *wrapComplexForInteger(const_CType ty) {
        assert(type_cache);
        return type_cache->wrapComplexForInteger(ty);
    }
    llvm::Value *getAddress(Expr e) {
        switch (e->k) {
        case EVar: return vars[e->sval];
        case EMemberAccess: {
            auto basep = e->k == EMemberAccess ? getAddress(e->obj) : gen(e->obj);
            auto ty = wrap(e->obj->ty);
            return B.CreateStructGEP(ty, basep, e->idx);
        }
        case EUnary:
            switch (e->uop) {
            case AddressOf: return getAddress(e->uoperand);
            case Dereference: return gen(e->uoperand);
            case C__real__:
            case C__imag__: {
                auto ty = wrap(e->uoperand->ty);
                return B.CreateInBoundsGEP(ty, getAddress(e->uoperand), {i32_0, e->uop == C__real__ ? i32_0 : i32_1});
            }
            default: llvm_unreachable("");
            }
        case ESubscript: {
            llvm::Type *ty;
            return subscript(e, ty);
        }
        case EArrToAddress: return getAddress(e->voidexpr);
        case EConstantArray: return e->array;
        case EArray:
        case EStruct: {
            auto v = e->k == EArray ? getArray(e) : getStruct(e);
            if (!currentfunction) {
                auto g = new llvm::GlobalVariable(*module, v->getType(), false, InternalLinkage, nullptr, "");
                // g->setInitializer(v);
                return g;
            }
            auto local = B.CreateAlloca(v->getType());
            B.CreateStore(v, local);
            return local;
        }
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
        auto it = type_cache->vla_size_map.insert(std::make_pair(e, nullptr));
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
            return std::make_pair(B.CreateNSWMul(numElements, pair.first), pair.second);
        }
        return std::make_pair(numElements, wrap(elementType));
    }
    std::pair<llvm::Value *, llvm::Type *> genVLASizeof(CType ty) {
        assert(ty->isVLA());
        return genVLASizeof(ty->vla_expr, ty->vla_arraytype);
    }
    llvm::Value *gen(Expr e) {
        switch (e->k) {
        case EConstant: return e->C;
        case EConstantArraySubstript:
            return llvm::ConstantExpr::getInBoundsGetElementPtr(wrap(e->ty->p), e->carray,
                                                                ConstantInt::get(ctx, e->cidx));
        case EPostFix: {
            auto p = getAddress(e->poperand);
            auto ty = wrap(e->ty);
            auto a = e->poperand->ty->getAlignAsMaybeAlign();
            auto r = load(p, ty, a);
            llvm::Value *v;
            if (e->ty->getKind() == TYPOINTER) {
                ty = wrap(e->poperand->ty->p);
                v = B.CreateInBoundsGEP(ty, r, {e->pop == PostfixIncrement ? i32_1 : i32_n1});
            } else if (e->ty->getKind() == TYPRIM && e->ty->isFloating()) {
                v = (e->pop == PostfixIncrement) ? B.CreateFAdd(r, llvm::ConstantFP::get(ty, 1.0))
                                                 : B.CreateFSub(r, llvm::ConstantFP::get(ty, 1.0));
            } else {
                v = (e->pop == PostfixIncrement) ? B.CreateAdd(r, llvm::ConstantInt::get(ty, 1))
                                                 : B.CreateSub(r, llvm::ConstantInt::get(ty, 1));
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
        case EMemberAccess: {
            bool isVar = e->k == EVar;
            auto ty = wrap(e->obj->ty);
            auto base = isVar ? getAddress(e->obj) : gen(e->obj);
            if (isVar || e->obj->ty->getKind() == TYPOINTER) {
                auto pMember = B.CreateInBoundsGEP(
                    ty, base, {llvm::ConstantInt::get(type_cache->integer_types[context.getIntLog2()], e->idx)});
                return load(pMember, wrap(e->ty), e->obj->ty->getAlignAsMaybeAlign());
            }
            return B.CreateExtractValue(load(base, ty, e->obj->ty->getAlignAsMaybeAlign()), e->idx);
        }
        case EConstantArray: return e->array;
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
                llvm::StoreInst *s = B.CreateAlignedStore(rhs, basep, e->lhs->ty->getAlignAsMaybeAlign());
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
                return B.CreateAtomicRMW(static_cast<llvm::AtomicRMWInst::BinOp>(pop), addr, rhs, llvm::None,
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
            case SAdd: return B.CreateNSWAdd(lhs, rhs);
            // clang::ComplexExprEmitter::EmitBinAdd
            case CAdd: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = B.CreateAdd(a, c), *const r = B.CreateAdd(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            case CFAdd: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = B.CreateFAdd(a, c), *const r = B.CreateFAdd(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            // clang::ComplexExprEmitter::EmitBinSub
            case CFSub: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = B.CreateFSub(a, c), *const r = B.CreateFSub(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            case CSub: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const l = B.CreateSub(a, c), *const r = B.CreateSub(b, d);
                return make_complex_pair(e->ty, l, r);
            }
            // clang::ComplexExprEmitter::EmitBinMul
            case CFMul: {
                llvm::Value *LibCallI, *LibCallR;
                llvm::MDBuilder MDHelper(ctx);
                auto ty = cast<llvm::StructType>(wrap(e->ty));
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const AC = B.CreateFMul(a, c), *const BD = B.CreateFMul(b, d),
                                   *const AD = B.CreateFMul(a, d), *const BC = B.CreateFMul(b, c),
                                   *const ResR = B.CreateFSub(AC, BD), *const ResI = B.CreateFAdd(AD, BC),
                                   *const IsRNaN = B.CreateFCmpUNO(ResR, ResR);
                const auto ContBB = llvm::BasicBlock::Create(ctx), INaNBB = llvm::BasicBlock::Create(ctx);
                auto Branch = B.CreateCondBr(IsRNaN, INaNBB, ContBB);
                llvm::MDNode *BrWeight = MDHelper.createBranchWeights(1, (1U << 20) - 1);
                auto OrigBB = Branch->getParent();
                Branch->setMetadata(LLVMContext::MD_prof, BrWeight);
                append(INaNBB);
                llvm::Value *const IsINaN = B.CreateFCmpUNO(ResI, ResI);
                auto LibCallBB = llvm::BasicBlock::Create(ctx);
                Branch = B.CreateCondBr(IsINaN, LibCallBB, ContBB);
                Branch->setMetadata(LLVMContext::MD_prof, BrWeight);
                append(LibCallBB);
                {
                    auto real_ty = ty->getTypeAtIndex((unsigned)0);
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
                    auto FTY = llvm::FunctionType::get(ty, Params, false);
                    llvm::AttributeList attrs = llvm::AttributeList::get(
                        ctx, llvm::AttributeList::FunctionIndex,
                        {llvm::Attribute::NoUnwind, llvm::Attribute::ReadNone, llvm::Attribute::MustProgress,
                         llvm::Attribute::WillReturn, llvm::Attribute::NoRecurse, llvm::Attribute::NoSync,
                         llvm::Attribute::NoFree});
                    llvm::FunctionCallee F_C = module->getOrInsertFunction(LibCallName, FTY, attrs);
                    auto LibCallRes = B.CreateCall(F_C, Args);
                    LibCallR = gen_complex_real(LibCallRes);
                    LibCallI = gen_complex_imag(LibCallRes);
                }
                B.CreateBr(ContBB);
                append(ContBB);
                auto RealPHI = B.CreatePHI(ResR->getType(), 3);
                RealPHI->addIncoming(ResR, OrigBB);
                RealPHI->addIncoming(ResR, INaNBB);
                RealPHI->addIncoming(LibCallR, LibCallBB);
                auto ImagPHI = B.CreatePHI(ResI->getType(), 3);
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
                                         B.CreateSub(B.CreateMul(a, c), B.CreateMul(b, d)),
                                         B.CreateSub(B.CreateMul(a, d), B.CreateMul(b, c)));
            }
            // clang::ComplexExprEmitter::EmitBinDiv
            case CSDiv: {
                // (a+ib) / (c+id) = ((ac+bd)/(cc+dd)) + i((bc-ad)/(cc+dd))
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const cc = B.CreateMul(c, c), *const dd = B.CreateMul(d, d),
                                   *const cc_plus_dd = B.CreateAdd(cc, dd), *const ac = B.CreateAdd(a, c),
                                   *const bd = B.CreateAdd(b, d), *const bc = B.CreateAdd(b, c),
                                   *const ad = B.CreateAdd(a, d),
                                   *const L = B.CreateSDiv(B.CreateAdd(ac, bd), cc_plus_dd),
                                   *const R = B.CreateSDiv(B.CreateSub(bc, ad), cc_plus_dd);
                return make_complex_pair(wrapComplexForInteger(e->ty), L, R);
            }
            case CUDiv: {
                // (a+ib) / (c+id) = ((ac+bd)/(cc+dd)) + i((bc-ad)/(cc+dd))
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs),
                                   *const cc = B.CreateMul(c, c), *const dd = B.CreateMul(d, d),
                                   *const cc_plus_dd = B.CreateAdd(cc, dd), *const ac = B.CreateAdd(a, c),
                                   *const bd = B.CreateAdd(b, d), *const bc = B.CreateAdd(b, c),
                                   *const ad = B.CreateAdd(a, d),
                                   *const L = B.CreateUDiv(B.CreateAdd(ac, bd), cc_plus_dd),
                                   *const R = B.CreateUDiv(B.CreateSub(bc, ad), cc_plus_dd);
                return make_complex_pair(wrapComplexForInteger(e->ty), L, R);
            }
            case CFDiv: {
                auto ty = cast<llvm::StructType>(wrapComplex(e->ty));
                auto real_ty = ty->getTypeAtIndex((unsigned)0);
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
                auto FTY = llvm::FunctionType::get(ty, Params, false);
                llvm::AttributeList attrs = llvm::AttributeList::get(
                    ctx, llvm::AttributeList::FunctionIndex,
                    {llvm::Attribute::NoUnwind, llvm::Attribute::ReadNone, llvm::Attribute::MustProgress,
                     llvm::Attribute::WillReturn, llvm::Attribute::NoRecurse, llvm::Attribute::NoSync,
                     llvm::Attribute::NoFree});
                llvm::FunctionCallee F_C = module->getOrInsertFunction(LibCallName, FTY, attrs);
                return B.CreateCall(F_C, Args);
            }
            case CEQ: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs);
                llvm::Value *L, *R;
                if (e->lhs->ty->isFloating()) {
                    L = B.CreateFCmpOEQ(a, c);
                    R = B.CreateFCmpOEQ(b, d);
                } else {
                    L = B.CreateICmpEQ(a, c);
                    R = B.CreateICmpEQ(b, d);
                }
                return B.CreateAnd(L, R);
            }
            case CNE: {
                llvm::Value *const a = gen_complex_real(lhs), *const b = gen_complex_imag(lhs),
                                   *const c = gen_complex_real(rhs), *const d = gen_complex_imag(rhs);
                llvm::Value *L, *R;
                if (e->lhs->ty->isFloating()) {
                    L = B.CreateFCmpONE(a, c);
                    R = B.CreateFCmpONE(b, d);
                } else {
                    L = B.CreateICmpNE(a, c);
                    R = B.CreateICmpNE(b, d);
                }
                return B.CreateOr(L, R);
            }
            case SSub: return B.CreateNSWSub(lhs, rhs);
            case SMul: return B.CreateNSWMul(lhs, rhs);
            case PtrDiff: {
                CType target = e->lhs->ty->p;
                llvm::Value *dividend;

                lhs = B.CreatePtrToInt(lhs, intptrTy);
                rhs = B.CreatePtrToInt(rhs, intptrTy);
                auto sub = B.CreateSub(lhs, rhs);

                if (LLVM_UNLIKELY(target->isVLA())) {
                    std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(target);
                    uint64_t Size = getsizeof(pair.second);
                    dividend =
                        Size == 1 ? pair.first : B.CreateNSWMul(pair.first, llvm::ConstantInt::get(intptrTy, Size));
                } else {
                    uint64_t Size = getsizeof(target);
                    if (Size == 1)
                        return sub;
                    dividend = llvm::ConstantInt::get(intptrTy, Size);
                }
                return B.CreateExactSDiv(sub, dividend);
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
                    rhs = B.CreateNSWMul(pair.first, rhs);
                } else {
                    T = wrap(target);
                }
VLA_NEXT:;
                return B.CreateInBoundsGEP(T, lhs, {rhs});
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
                return B.CreateICmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
            case FEQ: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OEQ); goto BINOP_FCMP;
            case FNE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_ONE); goto BINOP_FCMP;
            case FGT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGT); goto BINOP_FCMP;
            case FGE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGE); goto BINOP_FCMP;
            case FLT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLT); goto BINOP_FCMP;
            case FLE:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLE);
                goto BINOP_FCMP;
BINOP_FCMP:
                return B.CreateFCmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
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
                return B.CreateBinOp(static_cast<llvm::Instruction::BinaryOps>(pop), lhs, rhs);
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
            auto Val = gen(e->uoperand);
            switch (e->uop) {
            case SNeg: return B.CreateNSWNeg(Val);
            case UNeg: return B.CreateNeg(Val);
            case FNeg: return B.CreateFNeg(Val);
            case Not: return B.CreateNot(Val);
            case Dereference: {
                auto ty = wrap(e->ty);
                auto r = load(Val, ty);
                auto tags = e->uoperand->ty->p->getTags();
                if (tags & TYVOLATILE)
                    r->setVolatile(true);
                if (tags & TYATOMIC)
                    r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
                return r;
            }
            case LogicalNot: return B.CreateIsNull(Val);
            // clang::ComplexExprEmitter::VisitUnaryMinus
            case CNeg: {
                llvm::Value *l, *r;
                if (e->ty->isFloating()) {
                    l = B.CreateFNeg(gen_complex_real(Val));
                    r = B.CreateFNeg(gen_complex_imag(Val));
                } else {
                    l = B.CreateNeg(gen_complex_real(Val));
                    r = B.CreateNeg(gen_complex_imag(Val));
                }
                return make_complex_pair(e->ty, l, r);
            }
            // clang::ComplexExprEmitter::VisitUnaryNot
            case CConj: {
                auto r = gen_complex_imag(Val);
                return make_complex_pair(e->ty, gen_complex_real(Val),
                                         e->ty->isFloating() ? B.CreateFNeg(r) : B.CreateNeg(r));
            }
            case C__real__: return gen_complex_real(Val);
            case C__imag__: return gen_complex_imag(Val);
            case ToBool:
            case AddressOf: llvm_unreachable("");
            }
            llvm_unreachable("invalid UnaryOp");
        }
        case EVar: {
            auto pvar = vars[e->sval];
            auto r = load(pvar, wrap(e->ty), e->ty->getAlignAsMaybeAlign());
            if (e->ty->hasTag(TYVOLATILE))
                r->setVolatile(true);
            if (e->ty->hasTag(TYATOMIC))
                r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
            return r;
        }
        case ECondition: {
            if (e->ty->hasTag(TYVOID)) {
                llvm::BasicBlock *iftrue = llvm::BasicBlock::Create(ctx, "", currentfunction),
                                 *iffalse = llvm::BasicBlock::Create(ctx), *ifend = llvm::BasicBlock::Create(ctx);
                B.CreateCondBr(gen_cond(e->cond), iftrue, iffalse);
                after(iftrue);
                (void)gen(e->cleft);
                B.CreateBr(ifend);
                append(iffalse);
                (void)gen(e->cright);
                append(ifend);
                return nullptr;
            }
            if (e->cleft->k == EConstant && e->cright->k == EConstant)
                return B.CreateSelect(gen_cond(e->cond), gen(e->cleft), gen(e->cright));
            llvm::Type *ty = wrap(e->cleft->ty);
            llvm::BasicBlock *iftrue = llvm::BasicBlock::Create(ctx, "", currentfunction),
                             *iffalse = llvm::BasicBlock::Create(ctx), *ifend = llvm::BasicBlock::Create(ctx);
            B.CreateCondBr(gen_cond(e->cond), iftrue, iffalse);

            after(iftrue);
            llvm::Value *left = gen(e->cleft);
            B.CreateBr(ifend);

            append(iffalse);
            llvm::Value *right = gen(e->cright);
            B.CreateBr(ifend);

            append(ifend);
            auto phi = B.CreatePHI(ty, 2);

            phi->addIncoming(left, iftrue);
            phi->addIncoming(right, iffalse);
            return phi;
        }
        case ESizeof: {
            std::pair<llvm::Value *, llvm::Type *> pair = genVLASizeof(e->theType);
            if (pair.second->isIntegerTy(1))
                return pair.first;
            return B.CreateNSWMul(pair.first, /*llvm::ConstantExpr::getSizeOf(pair.second)*/ llvm::ConstantInt::get(
                                      intptrTy, getsizeof(pair.second)));
        }
        case ECast: return B.CreateCast(getCastOp(e->castop), gen(e->castval), wrap(e->ty));
        case ECall: {
            llvm::Value *f;
            auto ty = cast<llvm::FunctionType>(wrap(e->callfunc->ty->p));
            if (e->callfunc->k == EUnary) {
                assert(e->callfunc->uop == AddressOf);
                f = getAddress(e->callfunc);
            } else {
                f = gen(e->callfunc);
            }
            size_t l = e->callargs.size();
            llvm::Value **buf = type_cache->alloc.Allocate<llvm::Value *>(l);
            for (size_t i = 0; i < l; ++i)
                buf[i] = gen(e->callargs[i]); // eval argument from left to right
            auto r = B.CreateCall(ty, f, ArrayRef<llvm::Value *>(buf, l));
            return r;
        }
        case EStruct: return getStruct(e);
        case EArray: return getArray(e);
        default: llvm_unreachable("bad enum kind!");
        }
    }

  public:
    IRGen(xcc_context &context, DiagnosticsEngine &Diag, SourceMgr &SM, LLVMContext &ctx, const Options &options)
        : DiagnosticHelper{Diag}, context{context}, SM{SM}, ctx{ctx}, B{ctx}, options{options} {
        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions opt;
        // rustc: Could not create LLVM TargetMachine for triple: ...
        machine = options.theTarget->createTargetMachine(options.triple.str(), CPU, Features, opt,
                                                         options.RelocationModel, options.CodeModel, getCGOptLevel());
        if (LLVM_UNLIKELY(!machine))
            llvm::report_fatal_error("failed to create llvm::TargetMachine: the target might has no target machine");
        layout = new llvm::DataLayout(machine->createDataLayout());

        // TODO: Decimal float types
        intptrTy = layout->getIntPtrType(ctx);
        pointerSizeInBits = intptrTy->getBitWidth();

        llvm::IntegerType *i32Ty = llvm::Type::getInt32Ty(ctx);
        i32_n1 = llvm::ConstantInt::get(i32Ty, -1);
        i32_1 = llvm::ConstantInt::get(i32Ty, 1);
        i32_0 = llvm::ConstantInt::get(i32Ty, 0);
        i1_0 = llvm::ConstantInt::getFalse(ctx);
        i1_1 = llvm::ConstantInt::getTrue(ctx);
        createModule();
    }
    uint64_t getsizeof(llvm::Type *ty) { return layout->getTypeStoreSize(ty); }
    uint64_t getsizeof(CType ty) {
        assert((!ty->isVLA()) && "VLA should handled in other case");
        type_tag_t Align = ty->getAlignLog2Value();
        if (Align)
            return uint64_t(1) << Align;
        const auto k = ty->getKind();
        if (ty->isVoid() || k == TYFUNCTION)
            return 1;
        return getsizeof(wrap(ty));
    }
    uint64_t getsizeof(Expr e) { return getsizeof(e->ty); }
    uint64_t getAlignof(llvm::Type *ty) { return layout->getPrefTypeAlign(ty).value(); }
    uint64_t getAlignof(CType ty) {
        type_tag_t Align = ty->getAlignLog2Value();
        if (Align)
            return uint64_t(1) << Align;
        return getAlignof(wrap(ty));
    }
    uint64_t getAlignof(Expr e) { return getAlignof(e->ty); }
    std::unique_ptr<llvm::Module> run(const TranslationUnit &TU, LLVMTypeConsumer &type_cache) {
        this->type_cache = &type_cache;
        assert(this->type_cache);
        this->type_cache->reset(TU.max_tags_scope);
        vars = new llvm::Value *[TU.max_typedef_scope];
        runCodeGenTranslationUnit(TU.ast);
        RunOptimizationPipeline();
        return std::move(module);
    }
    void setTypeConsumer(LLVMTypeConsumer &C) {
        this->type_cache = &C;
    }
};
