/* codegen.cpp - code generating to LLVM IR

Ast(Stmt, Expr) => llvm Module

*/

struct IRGen : public DiagnosticHelper {

    using Type = llvm::Type *;
    using Value = llvm::Value *;
    using DIType = llvm::DIType *;
    using DIScope = llvm::DIScope *;
    using LLVMContext = llvm::LLVMContext;
    using Label = llvm::BasicBlock *;

    static constexpr auto ExternalLinkage = llvm::GlobalValue::ExternalLinkage,
                          PrivateLinkage = llvm::GlobalValue::PrivateLinkage,
                          CommonLinkage = llvm::GlobalValue::CommonLinkage,
                          AppendingLinkage = llvm::GlobalValue::AppendingLinkage,
                          InternalLinkage = llvm::GlobalValue::InternalLinkage;
    static constexpr auto DefaultStorageClass = llvm::GlobalValue::DefaultStorageClass,
                          DLLImportStorageClass = llvm::GlobalValue::DLLImportStorageClass,
                          DLLExportStorageClass = llvm::GlobalValue::DLLExportStorageClass;

    IRGen(xcc_context &context, SourceMgr &SM, LLVMContext &ctx, const Options &options)
        : DiagnosticHelper{context}, SM{SM}, ctx{ctx}, B{ctx}, options{options}, triple{options.triple} {
        std::string Error;
        // auto theTriple = llvm::Triple(options.triple);
        auto target = llvm::TargetRegistry::lookupTarget(options.triple, Error);
        if (!target) {
            fatal("TargetRegistry::lookupTarget: %s", Error.data());
            return;
        }
        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions opt;

        machine = target->createTargetMachine(options.triple, CPU, Features, opt, llvm::Reloc::PIC_, llvm::None,
                                              llvm::CodeGenOpt::Aggressive);
        if (!machine) {
            fatal("Target::createTargetMachine failed!");
            return;
        }
        layout = new llvm::DataLayout(machine->createDataLayout());
        types[xvoid] = llvm::Type::getVoidTy(ctx);
        ;
        types[xptr] = llvm::PointerType::get(ctx, 0);
        types[x1] = llvm::Type::getInt1Ty(ctx);
        types[x8] = llvm::Type::getInt8Ty(ctx);
        types[x16] = llvm::Type::getInt16Ty(ctx);
        auto i32 = llvm::Type::getInt32Ty(ctx);
        types[x32] = i32;
        types[x64] = llvm::Type::getInt64Ty(ctx);
        types[x128] = llvm::Type::getInt128Ty(ctx);
        types[xfloat] = llvm::Type::getFloatTy(ctx);
        types[xdouble] = llvm::Type::getDoubleTy(ctx);
        types[xfp128] = llvm::Type::getFP128Ty(ctx);

        i32_n1 = llvm::ConstantInt::get(i32, -1);
        i32_1 = llvm::ConstantInt::get(i32, 1);
        i32_0 = llvm::ConstantInt::get(i32, 0);
        i1_0 = llvm::ConstantInt::getFalse(ctx);
        i1_1 = llvm::ConstantInt::getTrue(ctx);
    }

    llvm::Module *module = nullptr;
    SourceMgr &SM;
    LLVMContext &ctx;
    llvm::IRBuilder<> B;
    SmallVector<DIScope, 7> lexBlocks;
    llvm::DIBuilder *di = nullptr;

    llvm::Target *target = nullptr;
    llvm::TargetMachine *machine = nullptr;
    llvm::DataLayout *layout = nullptr;
    const Options &options;
    llvm::Triple triple;
    llvm::Function *currentfunction = nullptr;
    // `i32 1/0/-1` constant
    llvm::ConstantInt *i32_1, *i32_0, *i32_n1;
    // LLVM false/true constant
    llvm::ConstantInt *i1_0, *i1_1;
    unsigned lastFileID = -1;
    llvm::DIFile *lastFile = nullptr;
    OnceAllocator alloc{};
    Type types[TypeIndexHigh];
    DIType *ditypes = nullptr;
    // jump labels
    llvm::SmallVector<Label, 5> labels{};
    // struct/union
    llvm::Value **vars;
    llvm::Type **tags;
    llvm::DIType **dtags;
    llvm::DIType **dtypes;
    Location debugLoc;
    DenseMap<CType, llvm::FunctionType *> function_type_cache{};

    llvm::DIBuilder &D() { return *di; }
    void store(Value p, Value v, uint32_t align = 0) {
        llvm::StoreInst *s = B.CreateStore(v, p);
        if (align)
            s->setAlignment(llvm::Align(align));
    }
    llvm::LoadInst *load(Value p, Type t, uint32_t align = 0) {
        assert(p);
        assert(t);
        llvm::LoadInst *i = B.CreateLoad(t, p, false);
        if (align)
            i->setAlignment(llvm::Align(align));
        return i;
    }
    const llvm::Instruction *getTerminator() { return B.GetInsertBlock()->getTerminator(); }
    llvm::ValueAsMetadata *mdNum(uint64_t num) {
        Value v = llvm::ConstantInt::get(types[x32], num);
        return llvm::ValueAsMetadata::get(v);
    }
    DIScope getLexScope() { return lexBlocks.back(); }
    StringRef getFileStr(unsigned ID) { return SM.getFileName(ID); }
    llvm::DIFile *getFile(unsigned ID) {
        if (lastFileID != ID)
            return lastFileID = ID,
                   (lastFile = di->createFile(getFileStr(ID), StringRef(options.CWD.data(), options.CWD.size())));
        return lastFile;
    }
    llvm::DILocation *wrap(Location loc) { return llvm::DILocation::get(ctx, loc.line, loc.col, getLexScope()); }
    void append(Label theBB) {
        currentfunction->getBasicBlockList().push_back(theBB);
        B.SetInsertPoint(theBB);
    }
    void after(Label loc) { B.SetInsertPoint(loc); }
    void emitDebugLocation() { B.SetCurrentDebugLocation(llvm::DebugLoc()); }
    inline void enterScope() { }
    inline void leaveScope() { }
    Type wrap(CType ty) {
        // wrap a CType to LLVM Type
        switch (ty->k) {
        case TYPRIM:
            return types[getNoSignTypeIndex(ty->tags)];
        case TYPOINTER:
            return types[xptr];
        case TYFUNCTION: {
            // fast case: search cached
            auto it = function_type_cache.find(ty);
            if (it != function_type_cache.end())
                return it->second;
            // slow case: wrap all
            size_t l = ty->params.size();
            Type *buf = alloc.Allocate<Type>(l);
            size_t i = 0;
            for (const auto &it : ty->params)
                buf[i++] = wrap(it.ty);
            auto T = llvm::FunctionType::get(wrap(ty->ret), ArrayRef<Type>{buf, i}, ty->isVarArg);
            function_type_cache[ty] = T; // add to cache
            return T;
        }
        case TYARRAY:
            return llvm::ArrayType::get(wrap(ty->arrtype), ty->arrsize);
        case TYENUM:
            return types[x32];
        case TYINCOMPLETE:
            switch (ty->tag) {
            case TYSTRUCT:
            case TYUNION:
                return tags[ty->iidx];
            case TYENUM:
                return types[x32];
            default:
                llvm_unreachable("");
            }
        case TYBITFIELD:
        default:
            llvm_unreachable("unexpected type!");
        }
    }
    DIType wrap3(CType ty) {
        DIType result = wrap3Noqualified(ty);
        if (result) {
            if (ty->tags & TYCONST)
                result = di->createQualifiedType(llvm::dwarf::DW_TAG_const_type, result);
            if (ty->tags & TYVOLATILE)
                result = di->createQualifiedType(llvm::dwarf::DW_TAG_volatile_type, result);
            if (ty->tags & TYRESTRICT)
                result = di->createQualifiedType(llvm::dwarf::DW_TAG_restrict_type, result);
        }
        return result;
    }
    void emitDebugLocation(Expr e) {
        debugLoc = e->loc;
        if (options.g)
            B.SetCurrentDebugLocation(wrap(e->loc));
    }
    void emitDebugLocation(Stmt e) {
        debugLoc = e->loc;
        if (options.g)
            B.SetCurrentDebugLocation(wrap(e->loc));
    }
    llvm::IntegerType *getIntPtr() { return layout->getIntPtrType(ctx); }
    llvm::Function *addFunction(llvm::FunctionType *ty, const Twine &N) {
        return llvm::Function::Create(ty, ExternalLinkage, N, module);
    }
    void handle_asm(StringRef s) {
        if (currentfunction)
            // module level asm
            module->appendModuleInlineAsm(s);
        else {
            auto ty = llvm::FunctionType::get(types[xvoid], false);
            auto f = llvm::InlineAsm::get(ty, s, StringRef(), true);
            B.CreateCall(ty, f);
        }
    }
    llvm::DISubroutineType *createDebugFuctionType(CType ty) {
        auto L = ty->params.size();
        llvm::Metadata **buf = alloc.Allocate<llvm::Metadata *>(L);
        for (size_t i = 0; i < L; ++i)
            buf[i] = wrap3(ty->params[i].ty);
        return di->createSubroutineType(di->getOrCreateTypeArray(ArrayRef<llvm::Metadata *>(buf, L)));
    }
    auto getSizeInBits(Type ty) { return layout->getTypeSizeInBits(ty); }
    auto getSizeInBits(CType ty) { return getSizeInBits(wrap2(ty)); }
    auto getSizeInBits(Expr e) { return getSizeInBits(e->ty); }
    auto getsizeof(Type ty) { return layout->getTypeStoreSize(ty); }
    auto getsizeof(CType ty) { return getsizeof(wrap2(ty)); }
    auto getsizeof(Expr e) { return getsizeof(e->ty); }
    auto getAlignof(Type ty) { return layout->getPrefTypeAlign(ty).value(); }
    auto getAlignof(CType ty) { return getAlignof(wrap2(ty)); }
    auto getAlignof(Expr e) { return getAlignof(e->ty); }
    llvm::MDString *mdStr(StringRef str) { return llvm::MDString::get(ctx, str); }
    llvm::MDNode *mdNode(llvm::Metadata *m) { return llvm::MDNode::get(ctx, {m}); }
    llvm::MDNode *mdNode(ArrayRef<llvm::Metadata *> m) { return llvm::MDNode::get(ctx, m); }
    void addModule(StringRef source_file, StringRef ModuleID = "main") {
        module = new llvm::Module(ModuleID, ctx);
        module->setSourceFileName(source_file);
        module->setDataLayout(*layout);
        module->setTargetTriple(triple.str());

        auto llvm_ident = module->getOrInsertNamedMetadata("llvm.ident");
        auto llvm_flags = module->getOrInsertNamedMetadata("llvm.module.flags");
        llvm_ident->addOperand(mdNode(mdStr(CC_VERSION_FULL)));

        llvm_flags->addOperand(mdNode({mdNum(1), mdStr("short_enum"), mdNum(1)}));
        llvm_flags->addOperand(mdNode({mdNum(1), mdStr("wchar_size"), mdNum(1)}));
        llvm_flags->addOperand(mdNode({mdNum(1), mdStr("short_wchar"), mdNum(1)}));

        if (options.g) {
            di = new llvm::DIBuilder(*module);
            llvm::DIFile *file = di->createFile(source_file, options.CWD.str());
            lexBlocks.push_back(di->createCompileUnit(llvm::dwarf::DW_LANG_C99, file, CC_VERSION_FULL, false, "", 0));
            ditypes = new DIType[(size_t)TypeIndexHigh]; // not initialized!
            ditypes[voidty] = nullptr;
            ditypes[i1ty] = di->createBasicType("_Bool", 1, llvm::dwarf::DW_ATE_boolean);
            ditypes[i8ty] = di->createBasicType("char", 8, llvm::dwarf::DW_ATE_signed_char);
            ditypes[u8ty] = di->createBasicType("unsigned char", 8, llvm::dwarf::DW_ATE_unsigned_char);
            ditypes[i16ty] = di->createBasicType("short", 16, llvm::dwarf::DW_ATE_signed);
            ditypes[u16ty] = di->createBasicType("unsigned short", 16, llvm::dwarf::DW_ATE_unsigned);
            ditypes[i32ty] = di->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
            ditypes[u32ty] = di->createBasicType("unsigned", 32, llvm::dwarf::DW_ATE_unsigned);
            ditypes[i64ty] = di->createBasicType("long long", 64, llvm::dwarf::DW_ATE_signed);
            ditypes[u64ty] = di->createBasicType("unsigned long long", 64, llvm::dwarf::DW_ATE_unsigned);
            ditypes[i128ty] = di->createBasicType("__int128", 128, llvm::dwarf::DW_ATE_unsigned);
            ditypes[u128ty] = di->createBasicType("__uint128", 128, llvm::dwarf::DW_ATE_unsigned);
            ditypes[floatty] =
                di->createBasicType("float", getSizeInBits(types[xfloat]), llvm::dwarf::DW_ATE_decimal_float);
            ditypes[doublety] =
                di->createBasicType("double", getSizeInBits(types[xdouble]), llvm::dwarf::DW_ATE_decimal_float);
            ditypes[ptrty] =
                di->createPointerType(nullptr, layout->getPointerTypeSizeInBits(types[xptr]), 0, llvm::None, "void*");
            module->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
            module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
        }
    }
    void finalsizeCodeGen() {
        if (di) {
            di->finalize();
            delete di;
        }
        if (ditypes)
            delete[] ditypes;
        if (dtags)
            delete[] dtags;
    }
    void run(Stmt s, size_t num_typedefs, size_t num_tags) {
        vars = new Value[num_typedefs](); // not initialized!
        tags = new Type[num_tags]();      // not initialized!
        if (options.g)
            dtags = new DIType[num_tags](); // not initialized!
        addModule("main");
        enterScope();
        for (Stmt ptr = s->next; ptr; ptr = ptr->next)
            gen(ptr);
        leaveScope();
        finalsizeCodeGen();
    }
    Value gen_cond(Expr e) {
        // generate bool(conditional) expression
        Value i = gen(e);
        if (e->ty->tags & TYBOOL)
            return i;
        return B.CreateIsNotNull(i);
    }
    Type wrap2(CType ty) {
        switch (ty->k) {
        case TYPRIM:
            return types[getNoSignTypeIndex(ty->tags)];
        case TYPOINTER:
            return types[xptr];
        case TYSTRUCT:
        case TYUNION: {
            size_t L = ty->selems.size();
            Type *buf = alloc.Allocate<Type>(L);
            for (size_t i = 0; i < L; ++i)
                buf[i] = wrap2(ty->selems[i].ty);
            return llvm::StructType::get(ctx, ArrayRef<Type>(buf, L));
        }
        case TYARRAY:
            return llvm::ArrayType::get(wrap(ty->arrtype), ty->arrsize);
        case TYENUM:
            return types[x32];
        default:
            llvm_unreachable("");
        }
    }
    DIType createEnum(CType ty, llvm::DICompositeType *old) {
        llvm::Metadata **buf = alloc.Allocate<llvm::Metadata *>(ty->eelems.size());
        StringRef Name;
        if (ty->ename)
            Name = ty->ename->getKey();
        for (size_t i = 0; i < ty->eelems.size(); i++) {
            buf[i] = di->createEnumerator(ty->eelems[i].name->getKey(), ty->eelems[i].val);
        }

        auto MD = di->createEnumerationType(
            getLexScope(), Name, getFile(debugLoc.id), debugLoc.line, getSizeInBits(types[x32]), 0,
            di->getOrCreateArray(ArrayRef<llvm::Metadata *>(buf, ty->selems.size())), ditypes[i32ty]);
        if (old) {
            old = llvm::MDNode::replaceWithPermanent(llvm::TempDICompositeType(old));
            llvm::MetadataTracking::untrack(&old, *old);
        }
        return MD;
    }
    DIType wrap3Noqualified(CType ty) {
        switch (ty->k) {
        case TYPRIM:
            return ditypes[getTypeIndex(ty->tags)];
        case TYPOINTER:
            if (ty->tags & TYVOID)
                return ditypes[ptrty];
            return di->createPointerType(wrap3(ty->p), ditypes[ptrty]->getSizeInBits());
        case TYSTRUCT:
        case TYUNION: {
            return dtags[ty->sidx];
            /*
            StringRef Name;
            DIType T;
            if (ty->sname) {
                Name = ty->sname->getKey();
                T = dtags[ty->sidx];
                if (!T->isForwardDecl())
                    return T;
            }
            auto t = cast<llvm::StructType>(wrap(ty));
            auto slayout = layout->getStructLayout(t);
            size_t L = ty->selems.size();
            llvm::Metadata **buf = alloc.Allocate<llvm::Metadata*>(L);
            for (size_t i = 0;i < L;i++) {
                buf[i] = di->createMemberType(
                    getLexScope(), ty->selems[i].name->getKey(),
                    getFile(debugLoc.id), debugLoc.line, getsizeof(ty->selems[i].ty),
                    0, slayout->getElementOffsetInBits(i),
                    llvm::DINode::FlagZero,
                    wrap3(ty->selems[i].ty)
                );
            }
            DIType result = (ty->k == TYSTRUCT) ?
                    di->createStructType(
                    getLexScope(), Name,
                    getFile(debugLoc.id), debugLoc.line,
                    slayout->getSizeInBits(),
                    slayout->getAlignment().value() * 8,
                    llvm::DINode::FlagZero, nullptr, di->getOrCreateArray(ArrayRef<llvm::Metadata*>{buf, L})) :
                    di->createUnionType(
                    getLexScope(), Name,
                    getFile(debugLoc.id), debugLoc.line,
                    slayout->getSizeInBits(),
                    slayout->getAlignment().value() * 8,
                    llvm::DINode::FlagZero, di->getOrCreateArray(ArrayRef<llvm::Metadata*>{buf, L}));
            if (ty->sname)
                dtags[ty->sidx] = result;
            return result;
            */
        }
        case TYARRAY: {
            auto size = getsizeof(ty);
            auto subscripts =
                ty->hassize ? di->getOrCreateSubrange(0, ty->arrsize) : di->getOrCreateSubrange(0, nullptr);
            auto arr = di->getOrCreateArray(ArrayRef<llvm::Metadata *>{subscripts});
            return di->createArrayType(size, getAlignof(ty) * 8, wrap3(ty->arrtype), arr);
        }
        case TYFUNCTION:
            return createDebugFuctionType(ty);
        case TYENUM:
            return dtags[ty->eidx];
        case TYINCOMPLETE:
            return dtags[ty->iidx];
        default:
            return nullptr;
        }
    }
    Value gen_logical(Expr lhs, Expr rhs, bool isand = true) {
        Label rightBB = llvm::BasicBlock::Create(ctx);
        Label phiBB = llvm::BasicBlock::Create(ctx);
        auto R = gen_cond(lhs);
        Label leftBB = B.GetInsertBlock();

        isand ? B.CreateCondBr(R, rightBB, phiBB) : B.CreateCondBr(R, phiBB, rightBB);

        append(rightBB);
        auto L = gen_cond(rhs);
        // gen_cond may change the basic block
        rightBB = B.GetInsertBlock();
        B.CreateBr(phiBB);

        append(phiBB);
        auto phi = B.CreatePHI(types[x1], 2);
        phi->addIncoming(R, leftBB);
        phi->addIncoming(L, rightBB);
        return phi;
    }
    auto getCastOp(CastOp a) {
        switch (a) {
        case Trunc:
            return llvm::Instruction::Trunc;
        case ZExt:
            return llvm::Instruction::ZExt;
        case SExt:
            return llvm::Instruction::SExt;
        case FPToUI:
            return llvm::Instruction::FPToUI;
        case FPToSI:
            return llvm::Instruction::FPToSI;
        case UIToFP:
            return llvm::Instruction::UIToFP;
        case SIToFP:
            return llvm::Instruction::SIToFP;
        case FPTrunc:
            return llvm::Instruction::FPTrunc;
        case FPExt:
            return llvm::Instruction::FPExt;
        case PtrToInt:
            return llvm::Instruction::PtrToInt;
        case IntToPtr:
            return llvm::Instruction::IntToPtr;
        case BitCast:
            return llvm::Instruction::BitCast;
        default:
            llvm_unreachable("bad cast operator");
        }
    }
    llvm::Function *newFunction(llvm::FunctionType *fty, IdentRef name, uint32_t tags, size_t idx) {
        auto old = module->getFunction(name->getKey());
        if (old)
            return old;
        auto F = addFunction(fty, name->getKey());
        F->setDSOLocal(true);
        F->setDoesNotThrow();
        F->addFnAttr(llvm::Attribute::OptimizeForSize);
        if (tags & TYSTATIC)
            F->setLinkage(InternalLinkage);
        if (tags & TYNORETURN)
            F->setDoesNotReturn();
        if (tags & TYINLINE)
            F->addFnAttr(llvm::Attribute::InlineHint);
        vars[idx] = F;
        return F;
    }
    StringRef getLinkageName(uint32_t tags) {
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
        emitDebugLocation(s);
        switch (s->k) {
        case SHead:
            llvm_unreachable("");
        case SCompound: {
            if (options.g)
                lexBlocks.push_back(di->createLexicalBlock(getLexScope(), getFile(s->loc.id), s->loc.line, s->loc.col));
            enterScope();
            for (Stmt ptr = s->inner; ptr; ptr = ptr->next)
                gen(ptr);
            leaveScope();
            if (options.g)
                lexBlocks.pop_back();
        } break;
        case SDecl: {
            if (s->decl_ty->k == TYINCOMPLETE) {
                if (s->decl_ty->tag != TYENUM) {
                    auto T = llvm::StructType::create(ctx, s->decl_ty->name->getKey());
                    tags[s->decl_idx] = T;
                }
                if (options.g) {
                    unsigned tag;
                    switch (s->decl_ty->tag) {
                    case TYSTRUCT:
                        tag = llvm::dwarf::DW_TAG_structure_type;
                        break;
                    case TYENUM:
                        tag = llvm::dwarf::DW_TAG_enumeration_type;
                        break;
                    case TYUNION:
                        tag = llvm::dwarf::DW_TAG_union_type;
                        break;
                    default:
                        llvm_unreachable("invalid type tag");
                    }
                    auto MD = di->createReplaceableCompositeType(tag, s->decl_ty->name->getKey(), getLexScope(),
                                                                 getFile(s->loc.line), s->loc.line);
                    dtags[s->decl_idx] = MD;
                }
            } else {
                if (s->decl_ty->k != TYENUM) {
                    size_t l = s->decl_ty->selems.size();
                    Type *buf = alloc.Allocate<Type>(l);
                    for (size_t i = 0; i < l; ++i)
                        buf[i] = wrap(s->decl_ty->selems[i].ty);
                    ArrayRef<Type> arr(buf, l);
                    tags[s->decl_idx] = s->decl_ty->sname ? llvm::StructType::create(arr, s->decl_ty->sname->getKey())
                                                          : llvm::StructType::create(arr);
                }
            }
        } break;
        case SUpdateForwardDecl: {
            if (s->now->tags != TYENUM) {
                auto T = cast<llvm::StructType>(tags[s->prev_idx]);
                size_t L = s->now->selems.size();
                Type *buf = alloc.Allocate<Type>(L);
                for (size_t i = 0; i < L; ++i)
                    buf[i] = wrap(s->now->selems[i].ty);
                T->setBody(ArrayRef<Type>(buf, L));
            }
            // replaceAllUsesWith
        } break;
        case SExpr:
            (void)gen(s->exprbody);
            break;
        case SFunction: {
            assert(this->labels.empty());
            enterScope();
            auto ty = cast<llvm::FunctionType>(wrap(s->functy));
            currentfunction = newFunction(ty, s->funcname, s->functy->tags, s->func_idx);
            llvm::DISubprogram *sp = nullptr;
            if (options.g) {
                sp =
                    di->createFunction(getLexScope(), s->funcname->getKey(), getLinkageName(s->functy->ret->tags),
                                       getFile(s->loc.id), s->loc.line, createDebugFuctionType(s->functy), s->loc.line);
                currentfunction->setSubprogram(sp);
                lexBlocks.push_back(sp);
                emitDebugLocation();
            }
            Label entry = llvm::BasicBlock::Create(ctx, "entry");
            append(entry);
            for (unsigned i = 0; i < s->numLabels; i++)
                this->labels.push_back(llvm::BasicBlock::Create(ctx));
            unsigned arg_no = 0;
            for (auto arg = currentfunction->arg_begin(); arg != currentfunction->arg_end(); ++arg) {
                auto pty = ty->getParamType(arg_no);
                auto p = B.CreateAlloca(pty);
                auto name = s->functy->params[arg_no].name;
                if (options.g) {
                    auto meta = di->createParameterVariable(getLexScope(), name->getKey(), arg_no, getFile(s->loc.id),
                                                            s->loc.line, wrap3(s->functy->params[arg_no].ty));
                    di->insertDeclare(p, meta, di->createExpression(), wrap(s->loc), entry);
                }
                store(p, arg);
                vars[s->args[arg_no]] = p;
                ++arg_no;
            }
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
            leaveScope();
            if (options.g) {
                lexBlocks.pop_back();
                di->finalizeSubprogram(sp);
            }
            this->labels.clear();
            this->currentfunction = nullptr;
        } break;
        case SReturn:
            B.CreateRet(s->ret ? gen(s->ret) : nullptr);
            break;
        case SDeclOnly:
            if (options.g)
                (void)wrap3Noqualified(s->decl);
            (void)wrap(s->decl);
            break;
        case SLabel: {
            auto BB = labels[s->label];
            if (!getTerminator())
                B.CreateBr(BB);
            after(BB);
            BB->insertInto(currentfunction);
            if (options.g && s->labelName) {
                BB->setName(s->labelName->getKey());
                auto LabelInfo =
                    di->createLabel(getLexScope(), s->labelName->getKey(), getFile(s->loc.id), s->loc.line);
                di->insertLabel(LabelInfo, wrap(s->loc), BB);
            }
        } break;
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
                auto ty = llvm::FunctionType::get(types[xvoid], false);
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
                auto align = varty->align;
                if (varty->tags & TYTYPEDEF) {
                    /* nothing */
                } else if (varty->k == TYFUNCTION) {
                    newFunction(cast<llvm::FunctionType>(wrap(varty)), name, varty->tags, idx);
                } else if (!currentfunction || varty->tags & (TYEXTERN | TYSTATIC)) {
                    auto GV = module->getGlobalVariable(name->getKey(), true);
                    if (GV) {
                        auto L = GV->getLinkage();
                        if (!(varty->tags & TYEXTERN)) {
                            if (L == ExternalLinkage) {
                                if (varty->tags & TYSTATIC)
                                    GV->setLinkage(InternalLinkage); // make it from extern declaration to static
                                else
                                    GV->setLinkage(ExternalLinkage); // a definition!
                            }
                        }
                        if (init) {
                            GV->setInitializer(cast<llvm::Constant>(gen(init))); // now update initializer
                            if (!(varty->tags & TYSTATIC))
                                GV->setLinkage(ExternalLinkage); // a definition!
                        }
                        break;
                    }
                    Type ty = wrap(varty);
                    llvm::Constant *ginit = init ? cast<llvm::Constant>(gen(init)) : llvm::Constant::getNullValue(ty);
                    GV = new llvm::GlobalVariable(*module, ty, false, ExternalLinkage, nullptr, name->getKey());
                    auto tags = varty->tags;
                    if (align)
                        GV->setAlignment(llvm::Align(align));
                    if (!(tags & TYEXTERN)) {
                        GV->setInitializer(ginit), GV->setDSOLocal(true);
                    }
                    if (tags & TYTHREAD_LOCAL)
                        GV->setThreadLocal(true);
                    if (tags & TYSTATIC) {
                        GV->setLinkage(InternalLinkage);
                    } else if (tags & TYEXTERN) {
                        GV->setLinkage(ExternalLinkage);
                    } else {
                        if (!init)
                            GV->setLinkage(CommonLinkage);
                    }
                    vars[idx] = GV;
                    if (options.g) {
                        StringRef linkage = getLinkageName(varty->tags);
                        auto gve = di->createGlobalVariableExpression(getLexScope(), name->getKey(), linkage,
                                                                      getFile(s->loc.id), s->loc.line, wrap3(varty),
                                                                      false, di->createExpression());
                        GV->addDebugInfo(gve);
                    }
                } else {
                    Type ty = wrap(varty);
                    auto val = B.CreateAlloca(ty, nullptr, name->getKey());
                    if (options.g) {
                        auto v = di->createAutoVariable(getLexScope(), name->getKey(), getFile(s->loc.id), s->loc.line,
                                                        wrap3(varty));
                        di->insertDeclare(val, v, di->createExpression(), wrap(s->loc), B.GetInsertBlock());
                    }
                    if (align) {
                        val->setAlignment(llvm::Align(align));
                        if (init) {
                            auto initv = gen(init);
                            store(val, initv, align);
                        }
                    } else {
                        if (init) {
                            auto initv = gen(init);
                            store(val, initv);
                        }
                        vars[idx] = val;
                    }
                }
            }
        } break;
        }
    }
    Value getStruct(Expr e) { return nullptr; }
    Value getArray(Expr e) { return nullptr; }
    llvm::GlobalVariable *CreateGlobalString(StringRef bstr, Type &ty) {
        auto str = llvm::ConstantDataArray::getString(ctx, bstr, true);
        ty = str->getType();
        auto GV = new llvm::GlobalVariable(*module, ty, true, PrivateLinkage, str);
        GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        GV->setAlignment(llvm::Align(1));
        return GV;
    }
    Value subscript(Expr e, Type &ty) {
        Value v;
        ty = wrap(e->left->ty->p);
        if (e->left->k == EArrToAddress) {
            if (e->left->voidexpr->ty->k == TYARRAY) {
                v = getAddress(e->left->voidexpr);
            } else {
                Type ty;
                auto str = CreateGlobalString(e->left->str.str(), ty);
                ArrayRef<Value> indices = {i32_0, gen(e->right)};
                return B.CreateInBoundsGEP(ty, str, indices);
            }
        } else {
            v = gen(e->left);
        }
        Value r = gen(e->right);
        if (e->left->k == EArrToAddress && e->left->voidexpr->ty->k == TYARRAY)
            return B.CreateInBoundsGEP(wrap(e->left->voidexpr->ty), v, {i32_0, r});
        return B.CreateInBoundsGEP(ty, v, {r});
    }
    Value getAddress(Expr e) {
        emitDebugLocation(e);
        switch (e->k) {
        case EVar:
            return vars[e->sval];
        case EMemberAccess: {
            auto basep = e->k == EMemberAccess ? getAddress(e->obj) : gen(e->obj);
            auto ty = wrap(e->obj->ty);
            return B.CreateStructGEP(ty, basep, e->idx);
        }
        case EUnary:
            switch (e->uop) {
            case AddressOf:
                return getAddress(e->uoperand);
            case Dereference:
                return gen(e->uoperand);
            default:
                llvm_unreachable("");
            }
        case ESubscript: {
            Type ty;
            return subscript(e, ty);
        }
        case EArrToAddress:
            return getAddress(e->voidexpr);
        case EString:
            return B.CreateGlobalStringPtr(e->str.str(), "", 0, module);
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
            llvm::errs() << "expression =" << e << "\nkind = " << (unsigned)e->k << "\n\n";
            getchar();
            llvm_unreachable("");
        }
    }
    Value gen(Expr e) {
        emitDebugLocation(e);
        switch (e->k) {
        case EConstant:
            return e->C;
        case EPostFix: {
            auto p = getAddress(e->poperand);
            auto ty = wrap(e->ty);
            auto r = load(p, ty, e->poperand->ty->align);
            Value v;
            if (e->ty->k == TYPOINTER) {
                ty = wrap(e->poperand->ty->p);
                v = B.CreateInBoundsGEP(ty, r, {e->pop == PostfixIncrement ? i32_1 : i32_n1});
            } else {
                v = (e->pop == PostfixIncrement) ? B.CreateAdd(r, llvm::ConstantInt::get(ty, 1))
                                                 : B.CreateSub(r, llvm::ConstantInt::get(ty, 1));
            }
            store(p, v, e->poperand->ty->align);
            return r;
        }
        case EArrToAddress:
            return getAddress(e->voidexpr);
        case ESubscript: {
            Type ty;
            auto v = subscript(e, ty);
            return load(v, ty);
        }
        case EMemberAccess: {
            auto base = gen(e->obj);
            // if (e->k == EPointerMemberAccess)
            //   base = load(base, wrap(e->obj->ty), e->obj->ty->align);
            return B.CreateExtractValue(base, e->idx);
        }
        case EString:
            return B.CreateGlobalStringPtr(e->str.str(), "", 0, module);
        case EBin: {
            if (e->bop == LogicalAnd)
                return gen_logical(e->lhs, e->rhs, true);
            if (e->bop == LogicalOr)
                return gen_logical(e->rhs, e->lhs, false);
            if (e->bop == Assign) {
                auto basep = getAddress(e->lhs);
                auto rhs = gen(e->rhs);
                auto s = B.CreateAlignedStore(rhs, basep,
                                              e->lhs->ty->align ? llvm::Align(e->lhs->ty->align) : llvm::MaybeAlign());
                if (e->lhs->ty->tags & TYVOLATILE)
                    s->setVolatile(true);
                if (e->lhs->ty->tags & TYATOMIC)
                    s->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
                return rhs;
            }
            Value lhs = gen(e->lhs);
            Value rhs = gen(e->rhs);
            unsigned pop = 0;
            switch (e->bop) {
            case LogicalOr:
                llvm_unreachable("");
            case LogicalAnd:
                llvm_unreachable("");
            case Assign:
                llvm_unreachable("");
            case AtomicrmwAdd:
                pop = static_cast<unsigned>(llvm::AtomicRMWInst::Add);
                goto BINOP_ATOMIC_RMW;
            case AtomicrmwSub:
                pop = static_cast<unsigned>(llvm::AtomicRMWInst::Sub);
                goto BINOP_ATOMIC_RMW;
            case AtomicrmwXor:
                pop = static_cast<unsigned>(llvm::AtomicRMWInst::Xor);
                goto BINOP_ATOMIC_RMW;
            case AtomicrmwOr:
                pop = static_cast<unsigned>(llvm::AtomicRMWInst::Or);
                goto BINOP_ATOMIC_RMW;
            case AtomicrmwAnd:
                pop = static_cast<unsigned>(llvm::AtomicRMWInst::And);
                goto BINOP_ATOMIC_RMW;
BINOP_ATOMIC_RMW:
                return B.CreateAtomicRMW(static_cast<llvm::AtomicRMWInst::BinOp>(pop), lhs, rhs, llvm::None,
                                         llvm::AtomicOrdering::SequentiallyConsistent);
            case SAdd:
                return B.CreateNSWAdd(lhs, rhs);
            case SSub:
                return B.CreateNSWSub(lhs, rhs);
            case SMul:
                return B.CreateNSWMul(lhs, rhs);
            case PtrDiff: {
                auto sub = B.CreateSub(lhs, rhs);
                auto s = getsizeof(e->lhs->castval->ty->p);
                if (s != 1) {
                    auto c = llvm::ConstantInt::get(getIntPtr(), s);
                    return B.CreateExactSDiv(sub, c);
                }
                return sub;
            }
            case SAddP:
                // getelementptr treat operand as signed, so we need to prmote to intptr type
                if (!e->rhs->ty->isSigned()) {
                    lhs = B.CreateZExt(lhs, getIntPtr());
                }
                return B.CreateInBoundsGEP(wrap(e->ty->p), lhs, {rhs});
            case EQ:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_EQ);
                goto BINOP_ICMP;
            case NE:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_NE);
                goto BINOP_ICMP;
            case UGT:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_UGT);
                goto BINOP_ICMP;
            case UGE:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_UGE);
                goto BINOP_ICMP;
            case ULT:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_ULT);
                goto BINOP_ICMP;
            case ULE:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_ULE);
                goto BINOP_ICMP;
            case SGT:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SGT);
                goto BINOP_ICMP;
            case SGE:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SGE);
                goto BINOP_ICMP;
            case SLT:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SLT);
                goto BINOP_ICMP;
            case SLE:
                pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SLE);
                goto BINOP_ICMP;
BINOP_ICMP:
                return B.CreateICmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
            case FEQ:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OEQ);
                goto BINOP_FCMP;
            case FNE:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_ONE);
                goto BINOP_FCMP;
            case FGT:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGT);
                goto BINOP_FCMP;
            case FGE:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGE);
                goto BINOP_FCMP;
            case FLT:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLT);
                goto BINOP_FCMP;
            case FLE:
                pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLE);
                goto BINOP_FCMP;
BINOP_FCMP:
                return B.CreateFCmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
            case Comma:
                return rhs;
            case UAdd:
                pop = llvm::Instruction::Add;
                goto BINOP_ARITH;
            case FAdd:
                pop = llvm::Instruction::FAdd;
                goto BINOP_ARITH;
            case USub:
                pop = llvm::Instruction::Sub;
                goto BINOP_ARITH;
            case FSub:
                pop = llvm::Instruction::FSub;
                goto BINOP_ARITH;
            case UMul:
                pop = llvm::Instruction::Mul;
                goto BINOP_ARITH;
            case FMul:
                pop = llvm::Instruction::FMul;
                goto BINOP_ARITH;
            case UDiv:
                pop = llvm::Instruction::UDiv;
                goto BINOP_ARITH;
            case SDiv:
                pop = llvm::Instruction::SDiv;
                goto BINOP_ARITH;
            case FDiv:
                pop = llvm::Instruction::FDiv;
                goto BINOP_ARITH;
            case URem:
                pop = llvm::Instruction::URem;
                goto BINOP_ARITH;
            case SRem:
                pop = llvm::Instruction::SRem;
                goto BINOP_ARITH;
            case FRem:
                pop = llvm::Instruction::FRem;
                goto BINOP_ARITH;

            case Shr:
                pop = llvm::Instruction::LShr;
                goto BINOP_SHIFT;
            case AShr:
                pop = llvm::Instruction::AShr;
                goto BINOP_SHIFT;
            case Shl:
                pop = llvm::Instruction::Shl;
                goto BINOP_SHIFT;

            case And:
                pop = llvm::Instruction::And;
                goto BINOP_BITWISE;
            case Xor:
                pop = llvm::Instruction::Xor;
                goto BINOP_BITWISE;
            case Or:
                pop = llvm::Instruction::Or;
                goto BINOP_BITWISE;
BINOP_ARITH:
BINOP_BITWISE:
BINOP_SHIFT:
                return B.CreateBinOp(static_cast<llvm::Instruction::BinaryOps>(pop), lhs, rhs);
            }
        }
        case EVoid:
            return (void)gen(e->voidexpr), nullptr;
        case EUnary:
            switch (e->uop) {
            case SNeg:
                return B.CreateNSWNeg(gen(e->uoperand));
            case UNeg:
                return B.CreateNeg(gen(e->uoperand));
            case FNeg:
                return B.CreateFNeg(gen(e->uoperand));
            case Not:
                return B.CreateNot(gen(e->uoperand));
            case AddressOf:
                return getAddress(e->uoperand);
            case Dereference: {
                auto g = gen(e->uoperand);
                auto ty = wrap(e->ty);
                auto r = load(g, ty);
                auto tags = e->uoperand->ty->p->tags;
                if (tags & TYVOLATILE)
                    r->setVolatile(true);
                if (tags & TYATOMIC)
                    r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
                return r;
            }
            case LogicalNot:
                return B.CreateIsNull(gen(e->uoperand));
            }
        case EVar: {
            auto pvar = vars[e->sval];
            auto r = load(pvar, wrap(e->ty), e->ty->align);
            if (e->ty->tags & TYVOLATILE)
                r->setVolatile(true);
            if (e->ty->tags & TYATOMIC)
                r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
            return r;
        }
        case ECondition: {
            if (e->ty->tags & TYVOID) {
                Label iftrue = llvm::BasicBlock::Create(ctx, "", currentfunction);
                Label iffalse = llvm::BasicBlock::Create(ctx);
                Label ifend = llvm::BasicBlock::Create(ctx);
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
            Type ty = wrap(e->cleft->ty);
            Label iftrue = llvm::BasicBlock::Create(ctx, "", currentfunction);
            Label iffalse = llvm::BasicBlock::Create(ctx);
            Label ifend = llvm::BasicBlock::Create(ctx);
            B.CreateCondBr(gen_cond(e->cond), iftrue, iffalse);

            after(iftrue);
            Value left = gen(e->cleft);
            B.CreateBr(ifend);

            append(iffalse);
            Value right = gen(e->cright);
            B.CreateBr(ifend);

            append(ifend);
            auto phi = B.CreatePHI(ty, 2);

            phi->addIncoming(left, iftrue);
            phi->addIncoming(right, iffalse);
            return phi;
        }
        case ECast:
            return B.CreateCast(getCastOp(e->castop), gen(e->castval), wrap(e->ty));
        case ECall: {
            Value f;
            auto ty = cast<llvm::FunctionType>(wrap(e->callfunc->ty->p));
            if (e->callfunc->k == EUnary) {
                assert(e->callfunc->uop == AddressOf);
                f = getAddress(e->callfunc);
            } else {
                f = gen(e->callfunc);
            }
            size_t l = e->callargs.size();
            Value *buf = alloc.Allocate<Value>(l);
            for (size_t i = 0; i < l; ++i)
                buf[i] = gen(e->callargs[i]); // eval argument from left to right
            auto r = B.CreateCall(ty, f, ArrayRef<Value>(buf, l));
            return r;
        }
        case EStruct:
            return getStruct(e);
        case EArray:
            return getArray(e);
        default:
            llvm_unreachable("bad enum kind!");
        }
    }
};
