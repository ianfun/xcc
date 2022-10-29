/* codegen.cpp - code generating to LLVM IR 

Ast(Stmt, Expr) => llvm Module

*/

struct IRGen: public DiagnosticHelper {

using Type = llvm::Type*;
using Value = llvm::Value*;
using DIType = llvm::DIType*;
using DIScope = llvm::DIScope*;
using LLVMContext = llvm::LLVMContext;
using Label = llvm::BasicBlock*;

static constexpr auto ExternalLinkage = llvm::GlobalValue::ExternalLinkage,
    PrivateLinkage = llvm::GlobalValue::PrivateLinkage,
    CommonLinkage = llvm::GlobalValue::CommonLinkage,
    AppendingLinkage = llvm::GlobalValue::AppendingLinkage,
    InternalLinkage = llvm::GlobalValue::InternalLinkage;
static constexpr auto
    DefaultStorageClass = llvm::GlobalValue::DefaultStorageClass,
    DLLImportStorageClass = llvm::GlobalValue::DLLImportStorageClass,
    DLLExportStorageClass = llvm::GlobalValue::DLLExportStorageClass;

IRGen(xcc_context &context, SourceMgr &SM, LLVMContext &ctx, const Options &options): 
    DiagnosticHelper{context}, SM{SM}, ctx{ctx}, builder{ctx}, options{options}, libtrace{options.libtrace}, g{options.g}, triple{options.triple} {
    std::string Error;
    // auto theTriple = llvm::Triple(options.triple);
    auto target = llvm::TargetRegistry::lookupTarget(options.triple, Error);
    if (!target)
    {
        fatal("TargetRegistry::lookupTarget: %s", Error.data());
        return;
    }
    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;

    machine = target->createTargetMachine(options.triple, CPU, Features, opt, llvm::Reloc::PIC_, llvm::None, llvm::CodeGenOpt::Aggressive);
    if (!machine) {
        fatal("Target::createTargetMachine failed!");
        return;
    }
    layout = new llvm::DataLayout(machine->createDataLayout());

    types[xvoid] = llvm::Type::getVoidTy(ctx);
    types[xptr] = llvm::PointerType::get(ctx, 0);
    types[x1] = llvm::Type::getInt1Ty(ctx);
    types[x8] = llvm::Type::getInt8Ty(ctx);
    types[x16] = llvm::Type::getInt16Ty(ctx);
    auto i32 = llvm::Type::getInt32Ty(ctx);
    types[x32] = i32;
    types[x64] = llvm::Type::getInt64Ty(ctx);
    types[xfloat] = llvm::Type::getFloatTy(ctx);
    types[xdouble] = llvm::Type::getDoubleTy(ctx);

    i32_n1 = llvm::ConstantInt::get(i32, -1);
    i32_1 = llvm::ConstantInt::get(i32, 1);
    i32_0 = llvm::ConstantInt::get(i32, 0);
    i1_0 = llvm::ConstantInt::getFalse(ctx);
    i1_1 = llvm::ConstantInt::getTrue(ctx);
}

llvm::Module *module = nullptr;
SourceMgr &SM;
LLVMContext &ctx;
llvm::IRBuilder<> builder;
SmallVector<DIScope, 7> lexBlocks;
llvm::DIBuilder *di = nullptr;

llvm::Target *target = nullptr;
llvm::TargetMachine *machine = nullptr;
llvm::DataLayout *layout = nullptr;
llvm::Triple triple;
Label topBreak, topSwitch, topdefaultCase, topCase, topContinue;
Value topTest; 
bool g;
bool libtrace;
llvm::Function *currentfunction = nullptr;
// `i32 1/0/-1` constant
llvm::ConstantInt *i32_1, *i32_0, *i32_n1;
// LLVM false/true constant
llvm::ConstantInt *i1_0, *i1_1;
// libtrace support
llvm::FunctionType *lt_call_ty, *lt_call_ty2, *lt_call_ty3;
llvm::StructType *lt_callframe;
llvm::Function 
  *lt_begin_call, 
  *lt_end_call, 
  *lt_defef_nullptr, 
  *lt_div_zero, 
  *lt_fdiv_zero,
  *lt_arr_index;
llvm::GlobalVariable *lt_now, *lt_line, *lt_file;
uint32_t last_line = -1;
unsigned lastFileID = -1;
llvm::DIFile *lastFile = nullptr;
OnceAllocator alloc {};
Type types[TypeIndexHigh];
DIType *ditypes = nullptr;
// jump labels
using labels_type = DenseMap<IdentRef, Label>;
labels_type labels{};
// struct/union
size_t scopeTop = 0;
using tags_type = DenseMap<IdentRef, Type>;
SmallVector<tags_type> tags{}; // variables
using vars_type = DenseMap<IdentRef, Value>;
SmallVector<vars_type> vars{};
using dtags_type = DenseMap<IdentRef, DIType>;
SmallVector<dtags_type> dtags{};
Location debugLoc;

const Options &options;
llvm::IRBuilder<> &B() {
    return builder;
}
llvm::Module &M() {
    return *module;
}
llvm::DIBuilder &D() {
    return *di;
}
void store(Value p, Value v, uint32_t align = 0){
    llvm::StoreInst *s = B().CreateStore(v, p);
    if (align)
      s->setAlignment(llvm::Align(align));
}
LLVM_NODISCARD llvm::LoadInst *load(Value p, Type t, uint32_t align = 0) {
    assert(p);
    assert(t);
    llvm::LoadInst *i = B().CreateLoad(t, p, false);
    if (align)
        i->setAlignment(llvm::Align(align));
    return i;
}
LLVM_NODISCARD const llvm::Instruction *getTerminator() {
    return B().GetInsertBlock()->getTerminator();
}
LLVM_NODISCARD llvm::ValueAsMetadata *mdNum(uint64_t num) {
    Value v = llvm::ConstantInt::get(types[x32], num);
    return llvm::ValueAsMetadata::get(v);
}
LLVM_NODISCARD DIScope getLexScope() {
    return lexBlocks.back();
}
LLVM_NODISCARD StringRef getFileStr(unsigned ID) {
    return SM.getFileName(ID);
}
LLVM_NODISCARD llvm::DIFile *getFile(unsigned ID) {
    if (lastFileID != ID)
        return lastFileID = ID, (lastFile = D().createFile(getFileStr(ID), StringRef(options.CWD.data(), options.CWD.size())));
    return lastFile;
}
llvm::DILocation *wrap(Location loc){
    return llvm::DILocation::get(ctx, loc.line, loc.col, getLexScope());
}
void br(Label theBB) {
    B().CreateBr(theBB);
}
void trybr(Label theBB) {
    if (!getTerminator())
        br(theBB);
}
void append(Label theBB) {
    currentfunction->getBasicBlockList().push_back(theBB);
    B().SetInsertPoint(theBB);
}
void after(Label loc) {
    B().SetInsertPoint(loc);
}
LLVM_NODISCARD Label addBlock(Twine Name = "") {
  return llvm::BasicBlock::Create(ctx, Name, currentfunction);
}
void emitDebugLocation() {
    B().SetCurrentDebugLocation(llvm::DebugLoc());
}
void enterScope() {
    if (++scopeTop >= tags.size()) {
        tags.push_back(tags_type());
        vars.push_back(vars_type());
        if (g)
            dtags.push_back(dtags_type());
    }
}
void leaveScope() {
    tags.back().clear();
    vars.back().clear();
    if (g)
        dtags.back().clear();
    --scopeTop;
}
LLVM_NODISCARD Value getVar(IdentRef s) {
    for (size_t i = scopeTop;i != (size_t)-1;i--) {
        auto it = vars[i].find(s);
        if (it != vars[i].end())
            return it->second;
    }
    return nullptr;
}
LLVM_NODISCARD Type getTags(IdentRef s) {
    for (size_t i = scopeTop;i != (size_t)-1;i--) {
        auto it = tags[i].find(s);
        if (it != tags[i].end())
            return it->second;
    }
    return nullptr;
}
LLVM_NODISCARD DIType getDIEnum(IdentRef s) {
    for (size_t i = scopeTop;i != (size_t)-1;i--) {
        auto it = dtags[i].find(s);
        if (it != dtags[i].end())
            return it->second;    
    }
    return nullptr;
}
void putTags(IdentRef name, Type ty) {
    tags[scopeTop][name] = ty;
}
void putDIEnum(IdentRef name, DIType ty) {
    dtags[scopeTop][name] = ty;
}
void putVar(IdentRef name, Value val) {
    vars[scopeTop][name] = val;
}
LLVM_NODISCARD Type wrap(CType ty) {
    // wrap a CType to LLVM Type
    switch (ty->k) {
        case TYPRIM:
            return types[getNoSignTypeIndex(ty->tags)];
        case TYPOINTER:
            return types[xptr];
        case TYSTRUCT:
        case TYUNION:
        {
            auto hasName = ty->sname && ty->sname->getKeyLength();
            // try to find old definition or declaration
            llvm::StructType *result;

            if (hasName) {
                result = cast<llvm::StructType>(getTags(ty->sname));
                if (result && !result->isOpaque())
                    return result;
            }
            size_t l = ty->selems.size();
            Type *buf = alloc.Allocate<Type>(l);
            for (size_t i = 0;i < l;++i)
                buf[i] = wrap(ty->selems[i].ty);
            // set struct body
            result = llvm::StructType::create(ArrayRef<Type>(buf, l), ty->sname->getKey(), false);
            // record it
            if (hasName)
            putTags(ty->sname, result);
            return result;
        }
        case TYFUNCTION:
        {
            size_t l = ty->params.size();
            Type *buf = alloc.Allocate<Type>(l);
            size_t i = 0;
            for (const auto &it: ty->params)
                buf[i++] = wrap(it.ty);
            return llvm::FunctionType::get(wrap(ty->ret), ArrayRef<Type>{buf, i}, ty->isVarArg);
        }
        case TYARRAY:
            return llvm::ArrayType::get(wrap(ty->arrtype), ty->arrsize);
        case TYENUM:
            return types[x32];
        case TYINCOMPLETE:
            switch (ty->tag) {
                case TYSTRUCT:
                case TYUNION:
                {
                    Type result = getTags(ty->name);
                    if (result)
                        return result;
                    return llvm::StructType::create(ctx, ty->name->getKey());
                }
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
LLVM_NODISCARD DIType wrap3(CType ty) {
    DIType result = wrap3Noqualified(ty);
    if (result) {
        if (ty->tags & TYCONST)
            result = D().createQualifiedType(llvm::dwarf::DW_TAG_const_type, result);
        if (ty->tags & TYVOLATILE)
            result = D().createQualifiedType(llvm::dwarf::DW_TAG_volatile_type, result);
        if (ty->tags & TYRESTRICT)
            result = D().createQualifiedType(llvm::dwarf::DW_TAG_restrict_type, result);
    }
    return result;
}
void emitDebugLocation(Expr e) {
    debugLoc = e->loc;
    if (g)
        B().SetCurrentDebugLocation(wrap(e->loc));
    else if (libtrace) {
        if (currentfunction) {
            auto target = e->loc.line;
            if (last_line == target)
                return;
            last_line = target;
            B().CreateStore(lt_line, llvm::ConstantInt::get(types[x32], target));
        }
    }
}
void emitDebugLocation(Stmt e) {
    debugLoc = e->loc;
    if (g)
        B().SetCurrentDebugLocation(wrap(e->loc));
    else if (libtrace) {
        if (currentfunction) {
            auto target = e->loc.line;
            if (last_line == target)
                return;
            last_line = target;
            B().CreateStore(lt_line, llvm::ConstantInt::get(types[x32], target));
        }
    }
}
LLVM_NODISCARD llvm::IntegerType *getIntPtr() {
    return layout->getIntPtrType(ctx);
}
llvm::Function *addFunction(llvm::FunctionType *ty, const Twine &N) {
    return llvm::Function::Create(ty, ExternalLinkage, N, &M());
}
void addlibtraceSupport() {
    ArrayRef<Type> arr = {
      types[x32], // line
      types[xptr], // func
      types[xptr], // file
      types[xptr]  // next
    };
    lt_callframe = llvm::StructType::create(ctx, arr, "CallFrame");
    lt_now = new llvm::GlobalVariable(M(), types[xptr], false, ExternalLinkage, nullptr, "__libtrace_now");
    lt_line = new llvm::GlobalVariable(M(), types[x32], false, ExternalLinkage, nullptr, "__libtrace_line");
    lt_file = new llvm::GlobalVariable(M(), types[xptr], false, ExternalLinkage, nullptr, "__libtrace_file");

    lt_call_ty = llvm::FunctionType::get(types[xvoid], false);
    lt_call_ty2 = llvm::FunctionType::get(types[xvoid], {types[x64], types[x64]}, false);
    lt_call_ty3 = llvm::FunctionType::get(types[xvoid], {types[xptr]}, false);

    lt_begin_call = addFunction(lt_call_ty, "__libtrace_begin_call");
    lt_end_call = addFunction(lt_call_ty, "__libtrace_end_call");
    lt_defef_nullptr = addFunction(lt_call_ty3, "__libtrace_defef_nullptr");
    lt_div_zero =  addFunction(lt_call_ty, "__libtrace_div_zero");
    lt_fdiv_zero = addFunction(lt_call_ty, "__libtrace_fdiv_zero");
    lt_arr_index = addFunction(lt_call_ty2, "__libtrace_array_index");
}
void handle_asm(StringRef s) {
    if (currentfunction)
        // module level asm
        M().appendModuleInlineAsm(s);
    else {
        auto ty = llvm::FunctionType::get(types[xvoid], false);
        auto f = llvm::InlineAsm::get(ty, s, StringRef(), true);
        B().CreateCall(ty, f);
    }
}
LLVM_NODISCARD Value gen_condition(Expr test ,Expr lhs, Expr rhs) {
    // build a `cond ? lhs : rhs` expression
    Type ty = wrap(lhs->ty);
    Label iftrue = addBlock();
    Label iffalse = addBlock();
    Label ifend = addBlock();
    condbr(test, iftrue, iffalse);

    after(iftrue);
    Value left = gen(lhs);
    br(ifend);
    
    after(iffalse);
    Value right = gen(rhs);
    br(ifend);

    after(ifend);
    auto phi = B().CreatePHI(ty, 2);

    phi->addIncoming(left, iftrue);
    phi->addIncoming(right, iffalse);
    return phi;
}
llvm::DISubroutineType *createDebugFuctionType(CType ty) {
    auto L = ty->params.size();
    llvm::Metadata **buf = alloc.Allocate<llvm::Metadata*>(L);
    for (size_t i = 0;i < L;++i)
        buf[i] = wrap3(ty->params[i].ty);
    return D().createSubroutineType(D().getOrCreateTypeArray(ArrayRef<llvm::Metadata*>(buf, L)));
}
auto getSizeInBits(Type ty) {
    return layout->getTypeSizeInBits(ty);
}
auto getSizeInBits(CType ty) {
    return getSizeInBits(wrap2(ty));
}
auto getSizeInBits(Expr e) {
    return getSizeInBits(e->ty);
}
auto getsizeof(Type ty) {
    return layout->getTypeStoreSize(ty);
}
auto getsizeof(CType ty){
    return getsizeof(wrap2(ty));
}
auto getsizeof(Expr e){
    return getsizeof(e->ty);
}
auto getAlignof(Type ty) {
    return layout->getPrefTypeAlign(ty).value();
}
auto getAlignof(CType ty){
    return getAlignof(wrap2(ty));
}
auto getAlignof(Expr e){
    return getAlignof(e->ty);
}
LLVM_NODISCARD llvm::MDString *mdStr(StringRef str) {
    return llvm::MDString::get(ctx, str);
}
LLVM_NODISCARD llvm::MDNode *mdNode(llvm::Metadata *m) {
    return llvm::MDNode::get(ctx, {m});
}
LLVM_NODISCARD llvm::MDNode *mdNode(ArrayRef<llvm::Metadata*> m) {
    return llvm::MDNode::get(ctx, m);
}
void addModule(StringRef source_file, StringRef ModuleID = "main") {
    module = new llvm::Module(ModuleID, ctx);
    M().setSourceFileName(source_file);
    M().setDataLayout(*layout);
    M().setTargetTriple(triple.str());

    auto llvm_ident = M().getOrInsertNamedMetadata("llvm.ident");
    auto llvm_flags = M().getOrInsertNamedMetadata("llvm.module.flags");
    llvm_ident->addOperand(mdNode(mdStr(CC_VERSION_FULL)));

    llvm_flags->addOperand(mdNode({mdNum(1), mdStr("short_enum"), mdNum(1)}));
    llvm_flags->addOperand(mdNode({mdNum(1), mdStr("wchar_size"), mdNum(1)}));
    llvm_flags->addOperand(mdNode({mdNum(1), mdStr("short_wchar"), mdNum(1)}));

    if (libtrace)
        addlibtraceSupport();
    if (g) {
        di = new llvm::DIBuilder(M());
        llvm::DIFile *file = D().createFile(source_file, options.CWD.str());
        lexBlocks.push_back(D().createCompileUnit(llvm::dwarf::DW_LANG_C99, file, CC_VERSION_FULL, false, "", 0));
        ditypes = new DIType[(size_t)TypeIndexHigh];
        ditypes[voidty] = nullptr;
        ditypes[i1ty] = D().createBasicType("_Bool", getSizeInBits(types[x1]), llvm::dwarf::DW_ATE_boolean);
        ditypes[i8ty] = D().createBasicType("char", getSizeInBits(types[x8]), llvm::dwarf::DW_ATE_signed_char);
        ditypes[u8ty] = D().createBasicType("unsigned char", getSizeInBits(types[x8]), llvm::dwarf::DW_ATE_unsigned_char);
        ditypes[i16ty] = D().createBasicType("short", getSizeInBits(types[x16]), llvm::dwarf::DW_ATE_signed);
        ditypes[u16ty] = D().createBasicType("unsigned short", getSizeInBits(types[x16]), llvm::dwarf::DW_ATE_unsigned);
        ditypes[i32ty] = D().createBasicType("int", getSizeInBits(types[x32]), llvm::dwarf::DW_ATE_signed);
        ditypes[u32ty] = D().createBasicType("unsigned", getSizeInBits(types[x32]), llvm::dwarf::DW_ATE_unsigned);
        ditypes[i64ty] = D().createBasicType("long long", getSizeInBits(types[x64]), llvm::dwarf::DW_ATE_signed);
        ditypes[u64ty] = D().createBasicType("unsigned long long", getSizeInBits(types[x64]), llvm::dwarf::DW_ATE_unsigned);
        ditypes[floatty] = D().createBasicType("float", getSizeInBits(types[xfloat]), llvm::dwarf::DW_ATE_decimal_float);
        ditypes[doublety] = D().createBasicType("double", getSizeInBits(types[xdouble]), llvm::dwarf::DW_ATE_decimal_float);
        ditypes[ptrty] = D().createPointerType(nullptr, layout->getPointerTypeSizeInBits(types[xptr]), 0, llvm::None, "void*");
        M().addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
        M().addModuleFlag(llvm::Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
    }
}
void finalsizeCodeGen() {
    if (g) {
        D().finalize();
        delete di;
        delete [] ditypes;
    }
}
void run(Stmt s) {
    assert(s->k == SHead && "expect translation-unit");
    addModule("main");
    enterScope();
    scopeTop = 0;
    for (Stmt ptr = s->next;ptr;ptr = ptr->next)
        gen(ptr);
    leaveScope();
    finalsizeCodeGen();
}
llvm::AllocaInst *buildlibtraceBeginCall(StringRef funcstr, uint32_t line, StringRef file) {
    B().CreateCall(lt_call_ty, lt_begin_call);
    llvm::AllocaInst *frame = B().CreateAlloca(lt_callframe);
    B().CreateStore(llvm::ConstantStruct::get(lt_callframe, {
        llvm::ConstantInt::get(types[x32], line), 
        B().CreateGlobalStringPtr(funcstr, "", 0, &M()), 
        B().CreateGlobalStringPtr(file, "", 0, &M()), 
        llvm::ConstantPointerNull::get(cast<llvm::PointerType>(types[xptr]))
    }), frame);
    auto old =  B().CreateAlloca(types[xptr]);
    auto o = load(lt_now, types[xptr]);
    store(old, o);
    auto p = B().CreateStructGEP(lt_callframe, o, 3);
    B().CreateStore(frame, p);
    B().CreateStore(frame, lt_now);
    return old;
}
void buildlibtraceAfterCall(Value old) {
    B().CreateStore(load(old, types[xptr]), lt_now);
    auto p = B().CreateStructGEP(lt_callframe, load(lt_now, types[xptr]), 3);
    B().CreateStore(llvm::Constant::getNullValue(types[xptr]), p);
    B().CreateCall(lt_call_ty, lt_end_call);
}
LLVM_NODISCARD Value gen_cond(Expr e) {
    // generate bool(conditional) expression
    Value i = gen(e);
    if (e->ty->tags & TYBOOL)
        return i;
    return B().CreateIsNotNull(i);
}
void condbr(Expr e, Label lhs, Label rhs) {
    B().CreateCondBr(gen_cond(e), lhs, rhs);
}
void condbr(Value v, Label lhs, Label rhs) {
    B().CreateCondBr(v, lhs, rhs);
}
LLVM_NODISCARD Type wrap2(CType ty) {
    switch (ty->k) {
        case TYPRIM:
          return types[getNoSignTypeIndex(ty->tags)];
        case TYPOINTER:
          return types[xptr];
        case TYSTRUCT: 
        case TYUNION:
        {
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
LLVM_NODISCARD DIType createEnum(CType ty) {
    llvm::Metadata **buf = alloc.Allocate<llvm::Metadata*>(ty->eelems.size());
    StringRef Name;
    if (ty->ename)
        Name = ty->ename->getKey();
    for (size_t i = 0;i < ty->eelems.size();i++) {
        buf[i] = D().createEnumerator(ty->eelems[i].name->getKey(), ty->eelems[i].val);
    }
    return D().createEnumerationType( 
        getLexScope(),
        Name,
        getFile(debugLoc.id), 0,
        getSizeInBits(types[x32]), 0, 
        D().getOrCreateArray(ArrayRef<llvm::Metadata*>(buf, ty->selems.size())),
        ditypes[i32ty]
    );
}
LLVM_NODISCARD DIType wrap3Noqualified(CType ty) {
    switch (ty->k) {
        case TYPRIM:
            return ditypes[getTypeIndex(ty->tags)];
        case TYPOINTER:
            if (ty->tags & TYVOID)
                return ditypes[ptrty];
            return D().createPointerType(wrap3(ty->p), ditypes[ptrty]->getSizeInBits());
        case TYSTRUCT: 
        case TYUNION:
        {
            bool hasName = ty->sname && ty->sname->getKeyLength();
            StringRef Name;
            if (hasName) {
                Name = ty->sname->getKey();
                auto result = getDIEnum(ty->sname);
                if (result)
                    return result;
            }
            auto t = cast<llvm::StructType>(wrap(ty));
            auto slayout = layout->getStructLayout(t);
            size_t L = ty->selems.size();
            llvm::Metadata **buf = alloc.Allocate<llvm::Metadata*>(L);
            for (size_t i = 0;i < L;i++) {
                buf[i] = D().createMemberType(
                    getLexScope(), ty->selems[i].name->getKey(),
                    getFile(debugLoc.id), 0, getsizeof(ty->selems[i].ty),
                    0, slayout->getElementOffsetInBits(i),
                    llvm::DINode::FlagZero,
                    wrap3(ty->selems[i].ty)
                );
            }
            DIType result = (ty->k == TYSTRUCT) ?  
                    D().createStructType(
                    getLexScope(), Name,
                    getFile(debugLoc.id), 0,
                    slayout->getSizeInBits(), 
                    slayout->getAlignment().value() * 8, 
                    llvm::DINode::FlagZero, nullptr, D().getOrCreateArray(ArrayRef<llvm::Metadata*>{buf, L})) :
                    D().createUnionType(
                    getLexScope(), Name,
                    getFile(debugLoc.id), 0,
                    slayout->getSizeInBits(), 
                    slayout->getAlignment().value() * 8, 
                    llvm::DINode::FlagZero, D().getOrCreateArray(ArrayRef<llvm::Metadata*>{buf, L}));
            if (hasName)
                putDIEnum(ty->sname, result);
            return result;
        }
        case TYARRAY:
        {
            auto size = getsizeof(ty);
            auto subscripts = ty->hassize ?
                D().getOrCreateSubrange(0, ty->arrsize) :
                D().getOrCreateSubrange(0, nullptr);
            auto arr = D().getOrCreateArray(ArrayRef<llvm::Metadata*>{subscripts});
            return D().createArrayType(size, getAlignof(ty) * 8, wrap3(ty->arrtype), arr);
        }
        case TYFUNCTION:
            return createDebugFuctionType(ty);
        case TYENUM:
        {
            bool hasName = ty->ename && ty->ename->getKeyLength();
            if (hasName) {
                auto result = getDIEnum(ty->ename);
                if (result)
                    return result;
            }
            auto result = createEnum(ty);
            if (hasName)
                putDIEnum(ty->ename, result);
            return result;
        }
        default:
            return nullptr;
    }
}
LLVM_NODISCARD Value gen_logical(Expr lhs, Expr rhs, bool isand = true) {
    Label rightBB = addBlock();
    Label phiBB = llvm::BasicBlock::Create(ctx);
    auto R = gen_cond(lhs);
    Label leftBB = B().GetInsertBlock();

    isand ? 
        condbr(R, rightBB, phiBB) : 
        condbr(R, phiBB, rightBB);

    after(rightBB);
    auto L = gen_cond(rhs);
    // gen_cond may change the basic block
    rightBB = B().GetInsertBlock();
    br(phiBB);

    append(phiBB);
    auto phi = B().CreatePHI(types[x1], 2);
    phi->addIncoming(R, leftBB);
    phi->addIncoming(L, rightBB);
    return phi;
}
void gen_if(Expr test, Stmt body) {
    auto iftrue = addBlock();
    auto ifend = llvm::BasicBlock::Create(ctx);
    condbr(gen_cond(test), iftrue, ifend);
    after(iftrue);
    gen(body);
    trybr(ifend);
    append(ifend);
}
void gen_if(Expr test, Stmt body, Stmt elsebody) {
    Label iftrue = addBlock();
    Label iffalse = llvm::BasicBlock::Create(ctx);
    Label ifend = llvm::BasicBlock::Create(ctx);
    condbr(gen_cond(test), iftrue, iffalse);
    after(iftrue);
    gen(body);
    trybr(ifend);
    append(iffalse);
    gen(elsebody);
    trybr(ifend);
    append(ifend);
}
void gen_switch(Expr test, Stmt body) {
    auto old_switch = topSwitch;
    auto old_break = topBreak;
    auto old_test = topTest;
    auto old_case = topCase;
    auto old_default = topdefaultCase;

    topTest = gen(test);
    topBreak = llvm::BasicBlock::Create(ctx);
    topSwitch = B().GetInsertBlock();
    topCase = nullptr;
    topdefaultCase = addBlock();
    gen(body);
    if (topCase) 
        trybr(topBreak);
    if (!getTerminator()) {
        after(topSwitch);
        br(topdefaultCase);
    }
    append(topBreak);

    topdefaultCase = old_default;
    topCase = old_case;
    topTest = old_test;
    topBreak = old_break;
    topSwitch = old_switch;
}
void gen_case(Expr test, Stmt body) {
    auto thiscase = llvm::BasicBlock::Create(ctx);
    if (topCase)
        trybr(thiscase);
    topCase = thiscase;
    after(topSwitch);
    auto lhs = gen(test);
    auto rhs = topTest;
    auto cond = B().CreateICmp(llvm::CmpInst::ICMP_EQ, lhs, rhs);
    topSwitch = addBlock();
    condbr(cond, thiscase, topSwitch);
    append(thiscase);
    gen(body);
}
void gen_default(Stmt body) {
    if (topCase) {
        if (!getTerminator())
            br(topdefaultCase);
    }
    topCase = topdefaultCase;
    after(topdefaultCase);
    gen(body);
}
void gen_while(Expr test, Stmt body) {
    auto old_break = topBreak;
    auto old_continue = topContinue;

    auto whilecmp = addBlock();
    auto whilebody = llvm::BasicBlock::Create(ctx);
    auto whileleave = llvm::BasicBlock::Create(ctx);

    topBreak = whileleave;
    topContinue = whilecmp;

    br(whilecmp);

    after(whilecmp);
    auto cond = gen_cond(test);
    condbr(cond, whilebody, whileleave);

    append(whilebody);
    gen(body);
    trybr(whilecmp);

    append(whileleave);
    topBreak = old_break;
    topContinue = old_continue;
}
void gen_for(Expr test, Stmt body, Stmt sforinit, Expr eforincl) {
    auto old_break = topBreak;
    auto old_continue = topContinue;
  
    if (sforinit)
        gen(sforinit);
    enterScope();
    Label forcmp = addBlock();
    Label forbody = addBlock();
    Label forleave = addBlock();
    Label forincl = addBlock();
  
    topBreak = forleave;
    topContinue = forincl;
  
    br(forcmp);

    // for.cmp
    after(forcmp);
    if (test) {
        auto cond = gen_cond(test);
        condbr(cond, forbody, forleave);
    }
    else
        br(forbody);

    // for.body
    after(forbody);
    gen(body);
    trybr(forincl);

    // for.incl
    after(forincl);
    if (eforincl)
        (void)gen(eforincl);

    // for someone may call exit() in for-incl  
    trybr(forcmp);

    after(forleave);
    leaveScope();
    topBreak = old_break;
    topContinue = old_continue;
}
void gen_dowhile(Expr test, Stmt body) {
    auto old_break = topBreak;
    auto old_continue = topContinue;
  
    Label dowhilecmp = addBlock();
    Label dowhilebody = addBlock();
    Label dowhileleave = addBlock();
 
    topBreak = dowhileleave;
    topContinue = dowhilecmp;

    br(dowhilecmp);
    after(dowhilebody);
    gen(body);
    trybr(dowhilecmp);

    after(dowhilecmp);
    auto cond = gen_cond(test);
    condbr(cond, dowhilebody, dowhileleave);

    after(dowhileleave);
    topBreak = old_break;
    topContinue = old_continue;
}

llvm::Instruction::CastOps getCastOp(CastOp a){
    switch (a) {
        case Trunc:    return llvm::Instruction::Trunc;
        case ZExt:     return llvm::Instruction::ZExt;
        case SExt:     return llvm::Instruction::SExt;
        case FPToUI:   return llvm::Instruction::FPToUI;
        case FPToSI:   return llvm::Instruction::FPToSI;
        case UIToFP:   return llvm::Instruction::UIToFP;
        case SIToFP:   return llvm::Instruction::SIToFP;
        case FPTrunc:  return llvm::Instruction::FPTrunc;
        case FPExt:    return llvm::Instruction::FPExt;
        case PtrToInt: return llvm::Instruction::PtrToInt;
        case IntToPtr: return llvm::Instruction::IntToPtr;
        case BitCast:  return llvm::Instruction::BitCast;
        default: llvm_unreachable("bad cast operator");
    }
}

void addAttribute(llvm::Function *F, llvm::Attribute::AttrKind attrID) {
    auto attr = llvm::Attribute::get(ctx, attrID);
    F->addAttributeAtIndex(llvm::AttributeList::FunctionIndex, attr);
}
llvm::Function *newFunction(llvm::FunctionType *fty, IdentRef name, uint32_t tags) {
    auto old = vars.front().find(name);
    if (old != vars.front().end())
        return cast<llvm::Function>(old->second);

    auto F = addFunction(fty, name->getKey());
    F->setDSOLocal(true);
    addAttribute(F, llvm::Attribute::NoUnwind);
    addAttribute(F, llvm::Attribute::OptimizeForSize);
    if (tags & TYSTATIC)
        F->setLinkage(llvm::GlobalValue::InternalLinkage);
    if (tags & TYNORETURN)
        addAttribute(F, llvm::Attribute::NoReturn);
    if (tags & TYINLINE)
        addAttribute(F, llvm::Attribute::InlineHint);
    vars.front()[name] = F;
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
        case SLoop:
        {
            Label old_break = topBreak;
            Label old_continue = topContinue;
            
            Label c = addBlock();
            Label leave = addBlock();
            topBreak = leave;
            topContinue = c;
            br(c);
            after(c);
            gen(s->loop);
            trybr(c);
            after(leave);
            
            topBreak = old_break;
            topContinue = old_continue;
        } break;
        case SHead: llvm_unreachable("");
        case SCompound:
        {
            if (g)
                lexBlocks.push_back(D().createLexicalBlock(getLexScope(), getFile(s->loc.id), s->loc.line, s->loc.col));
            enterScope();
            for (Stmt ptr = s->inner;ptr;ptr = ptr->next)
                gen(ptr);
            leaveScope();
            if (g)
                lexBlocks.pop_back();
        } break;
        case SExpr:
            (void)gen(s->exprbody);
            break;
        case SFunction:
        {
            assert(this->labels.empty());
            bool isMain = s->funcname->second.getToken() == PP_main;
            bool debugMain = libtrace && isMain;
            enterScope();
            if (debugMain) {
                size_t l = s->functy->params.size();
                if (l == 0)
                    s->functy->params.push_back(NameTypePair(context.table.get("argc"), context.getInt()));
                if (l <= 1)
                    s->functy->params.push_back(NameTypePair(context.table.get("argv"), context.getPointerType(context.getPointerType(context.typecache.i8))));
            }
            auto ty = cast<llvm::FunctionType>(wrap(s->functy));
            currentfunction = newFunction(ty, s->funcname, s->functy->tags);
            auto &list = currentfunction->getBasicBlockList();
            llvm::DISubprogram *sp = nullptr;
            if (g) {
                sp = D().createFunction(
                    getLexScope(), 
                    s->funcname->getKey(), getLinkageName(s->functy->ret->tags), 
                    getFile(s->loc.id), 
                    s->loc.line, createDebugFuctionType(s->functy), s->loc.line);
                currentfunction->setSubprogram(sp);
                lexBlocks.push_back(sp);
                emitDebugLocation();
            }
            Label entry = llvm::BasicBlock::Create(ctx, "entry");
            list.push_back(entry);
            B().SetInsertPoint(entry);
            if (libtrace)
              store(lt_file, B().CreateGlobalStringPtr(getFileStr(s->loc.id), "", 0, &M()));
            for (const auto L: s->labels) 
                this->labels[L] = addBlock(L->getKey());
            unsigned arg_no = 0;
            for (auto arg = currentfunction->arg_begin();arg != currentfunction->arg_end();++arg) {
                auto pty = ty->getParamType(arg_no);
                auto p = B().CreateAlloca(pty);
                auto name = s->functy->params[arg_no].name;
                if (g) {
                    auto meta = D().createParameterVariable(getLexScope(), name->getKey(), arg_no, getFile(s->loc.id), s->loc.line, wrap3(s->functy->params[arg_no].ty));
                    D().insertDeclare(p, meta, D().createExpression(), wrap(s->loc), entry);
                }
                store(p, arg);
                putVar(name, p);
                ++arg_no;
            }
            if (debugMain) {
                auto fty = llvm::FunctionType::get(types[xvoid], {types[i32ty], types[ptrty]}, false);
                auto f = addFunction(fty, "__libtrace_init");
                B().CreateCall(fty, f, {currentfunction->getArg(0), currentfunction->getArg(1)});
            }
            for (Stmt ptr = s->funcbody->next;ptr;ptr = ptr->next) 
                gen(ptr);
            leaveScope();
            if (g) {
                lexBlocks.pop_back();
                D().finalizeSubprogram(sp);
            }
            this->labels.clear();
            this->currentfunction = nullptr;
        } break;
        case SReturn:
            B().CreateRet(s->ret ? gen(s->ret) : nullptr);
            break;
        case SIf:
            s->elsebody ? gen_if(s->iftest, s->ifbody, s->elsebody) : gen_if(s->iftest, s->ifbody);
            break;
        case SWhile:
            enterScope();
            gen_while(s->whiletest, s->whilebody);
            leaveScope();
            break;
        case SDoWhile:
            enterScope();
            gen_dowhile(s->dowhiletest, s->dowhilebody);
            leaveScope();
            break;
        case SFor:
            enterScope();
            gen_for(s->forcond, s->forbody, s->forinit, s->forincl);
            leaveScope();
            break;
        case SDeclOnly:
            if (g)
                (void)wrap3Noqualified(s->decl);
            (void)wrap(s->decl);
            break;
        case SLabeled:
        {
            auto BB = labels.find(s->label)->second;
            br(BB);
            after(BB);
            if (g){
                auto LabelInfo = D().createLabel(getLexScope(), s->location->getKey(), getFile(s->loc.id), s->loc.line);
                D().insertLabel(LabelInfo, wrap(s->loc), BB);
            }
            gen(s->labledstmt);
        }
        case SGoto:
        {
            auto BB = labels.find(s->location)->second;
            br(BB);
            after(BB);
        }
        break;
        case SContinue:
            br(topContinue);
            break;
        case SBreak:
            br(topBreak);
            break;
        case SAsm:
            handle_asm(s->asms);
            break;
        case SSwitch:
            gen_switch(s->swtest, s->swbody);
            break;
        case SDefault:
            gen_default(s->default_stmt);
            break;
        case SCase:
            gen_case(s->case_expr, s->case_stmt);
            break;
        case SVarDecl:
        {
            for (const auto &it: s->vars) {
                auto name = it.name;
                auto varty = it.ty;
                auto init = it.init;
                auto align = varty->align;
                if (varty->tags & TYTYPEDEF) {
                    /* nothing */
                }
                else if (varty->k == TYFUNCTION) {
                    newFunction(cast<llvm::FunctionType>(wrap(varty)), name, varty->tags);
                }
                else if (!currentfunction || varty->tags & (TYEXTERN | TYSTATIC)) {
                    auto old = cast_or_null<llvm::GlobalVariable>(getVar(name));
                    if (old) {
                        if (varty->tags & TYEXTERN)
                            break;
                        old->eraseFromParent();
                    }
                    Type ty = wrap(varty);
                    llvm::Constant *ginit = init ? cast<llvm::Constant>(gen(init)) : llvm::Constant::getNullValue(ty);
                    llvm::GlobalVariable *GV = new llvm::GlobalVariable(M(), ty, false, ExternalLinkage, nullptr, name->getKey());
                    auto tags = varty->tags;
                    if (align)
                        GV->setAlignment(llvm::Align(align));
                    if (!(tags & TYEXTERN))
                        GV->setInitializer(ginit),
                        GV->setDSOLocal(true);
                    if (tags & TYTHREAD_LOCAL)
                        GV->setThreadLocal(true);
                    if (tags & TYSTATIC) {
                        GV->setLinkage(InternalLinkage);
                    }
                    else if (tags & TYEXTERN) {
                        GV->setLinkage(ExternalLinkage);
                    }
                    else {
                        GV->setLinkage(CommonLinkage);
                    }
                    putVar(name, GV);
                    if (g) {
                        StringRef linkage = getLinkageName(varty->tags);
                        auto gve = D().createGlobalVariableExpression(
                            getLexScope(), 
                            name->getKey(), 
                            linkage, getFile(s->loc.id), 
                            s->loc.line, 
                            wrap3(varty), 
                            false, 
                            D().createExpression()
                        );
                        GV->addDebugInfo(gve);
                    }
                } else {
                    Type ty = wrap(varty);
                    auto val = B().CreateAlloca(ty, nullptr, name->getKey());
                    if (g) {
                        auto v = D().createAutoVariable(getLexScope(), name->getKey(), getFile(s->loc.id), s->loc.line, wrap3(varty));
                        D().insertDeclare(val, v, D().createExpression(), wrap(s->loc), B().GetInsertBlock());
                    }
                    if (align) {
                        val->setAlignment(llvm::Align(align));
                        if (init) {
                            auto initv = gen(init);
                            store(val, initv, align);
                        }
                    }
                    else {
                        if (init) {
                            auto initv = gen(init);
                            store(val, initv);
                        }
                        putVar(name, val);
                    }
                }
            }
        } break;
        default:
            llvm_unreachable("");
    }
}

LLVM_NODISCARD Value getStruct(Expr e) {
    return nullptr;
    /*Type ty = wrap(e->ty);
    if (e->arr.empty())
        return llvm::Constant::getNullValue(ty);
    else {
        size_t l = e->arr.size();
        size_t L = e->ty->selems.size();
        Value *buf = alloc.Allocate<Value>(L);
        for (size_t i = 0;i < l;++i)
            buf[i] = gen(e->arr[i]);
        for (size_t i = l;i < L;++i)
            buf[i] = llvm::Constant::getNullValue(wrap(e->ty->selems[i].ty));
        return llvm::ConstantStruct::get(ty, ArrayRef<Value>(buf, L));
    }*/
}
LLVM_NODISCARD Value getArray(Expr e) {
    return nullptr;
    /*
    size_t l = e->arr.size();
    Type elemTy = wrap(e->ty->arrtype);
    Value *buf = alloc.Allocate<Value>(e->ty->arrsize);
    for (size_t i = 0;i<l;++i)
      buf[i] = gen(e->arr[i]);
    for (size_t i = 0;i < e->ty->arrsize;++i)
      buf[i] = llvm::Constant::getNullValue(elemTy);
    return llvm::ConstantArray::get(elemTy, ArrayRef<Value>(l, buf), e->ty->arrsize);*/
}
LLVM_NODISCARD llvm::GlobalVariable *CreateGlobalString(StringRef bstr, Type &ty) {
    auto str = llvm::ConstantDataArray::getString(ctx, bstr, true);
    ty = str->getType();
    auto GV = new llvm::GlobalVariable(M(), ty, true, PrivateLinkage, str);
    GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    GV->setAlignment(llvm::Align(1));
    return GV;
}
LLVM_NODISCARD Value subscript(Expr e, Type &ty) {
    Value v;
    ty = wrap(e->left->ty->p);
    if (e->left->k == EArrToAddress) {
        if (e->left->voidexpr->ty->k == TYARRAY) {
            v = getAddress(e->left->voidexpr);
    } else {
        Type ty;
        auto str = CreateGlobalString(e->left->str.str(), ty);
        ArrayRef<Value> indices = {i32_0, gen(e->right)};
        return B().CreateInBoundsGEP(ty, str, indices);
        }
    }
    else {
        v = gen(e->left);
    }
    Value r = gen(e->right);
    if (libtrace && e->left->k == EArrToAddress) {
        auto is_indexing_array = e->left->voidexpr->ty->k == TYARRAY;
        // true: indexing array, false: indexing string literal
        if ((is_indexing_array && e->left->voidexpr->ty->hassize) || (!is_indexing_array)) {
            auto max = llvm::ConstantInt::get(types[x64], 
                is_indexing_array ? 
                e->left->voidexpr->ty->arrsize :
                e->left->voidexpr->str.size());
            auto index = r;
            Label iftrue = addBlock();
            Label ifend = addBlock();
            if (!(e->right->ty->tags & (TYUINT64 | TYINT64)))
                index = B().CreateZExt(r, types[x64]);
            condbr(B().CreateICmp(llvm::CmpInst::ICMP_UGE, index, max), iftrue, ifend);
            after(iftrue);
            B().CreateCall(lt_call_ty2, lt_arr_index, {index, max});
            B().CreateUnreachable();
            after(ifend);
        }
    }
    if (e->left->k == EArrToAddress && e->left->voidexpr->ty->k == TYARRAY)
      return B().CreateInBoundsGEP(wrap(e->left->voidexpr->ty), v, {i32_0, r});
    return B().CreateInBoundsGEP(ty, v, {r});
}
LLVM_NODISCARD Value getAddress(Expr e) {
    emitDebugLocation(e);
    switch (e->k) {
        case EVar:
          return getVar(e->sval);
        //case EPointerMemberAccess:
        case EMemberAccess:
        {
            auto basep = e->k == EMemberAccess ? getAddress(e->obj) : gen(e->obj);
            auto ty = wrap(e->obj->ty);
            return B().CreateStructGEP(ty, basep, e->idx);
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
        case ESubscript:
        {
            Type ty;
            return subscript(e, ty);
        }
        case EArrToAddress:
            return getAddress(e->voidexpr);
        case EString:
            return B().CreateGlobalStringPtr(e->str.str(), "", 0, &M());
        case EArray: case EStruct:
        {
            auto v = e->k == EArray ? getArray(e) : getStruct(e);
            if (!currentfunction) {
                auto g = new llvm::GlobalVariable(M(), v->getType(), false, InternalLinkage, nullptr, "");
                //g->setInitializer(v);
                return g;
            }
            auto local = B().CreateAlloca(v->getType());
            B().CreateStore(v, local);
            return local;
        }
        default:
            llvm_unreachable("");
    }
}
LLVM_NODISCARD Value gen(Expr e) {
    emitDebugLocation(e);
    switch (e->k) {
        case EPostFix:
        {
            auto p = getAddress(e->poperand);
            auto ty = wrap(e->ty);
            auto r = load(p, ty, e->poperand->ty->align);
            Value v;
            if (e->ty->k == TYPOINTER) {
                ty = wrap(e->poperand->ty->p);
                v =  B().CreateInBoundsGEP(ty, r, {e->pop == PostfixIncrement ? i32_1 : i32_n1});
            }
            else {
                v = (e->pop == PostfixIncrement) ? 
                B().CreateAdd(r, llvm::ConstantInt::get(ty, 1)) :
                B().CreateSub(r, llvm::ConstantInt::get(ty, 1));
            }
            store(p, v, e->poperand->ty->align);
            return r;
        }
        case EUndef:
            return llvm::UndefValue::get(wrap(e->ty));
        case EArrToAddress:
            return getAddress(e->voidexpr);
        case ESubscript:
        {
            Type ty;
            auto v = subscript(e, ty);
            return load(v, ty);
        }
        case EMemberAccess:
        {
            auto base = gen(e->obj);
            //if (e->k == EPointerMemberAccess)
            //  base = load(base, wrap(e->obj->ty), e->obj->ty->align);
            return B().CreateExtractValue(base, e->idx);
        }
        case EString:
            return B().CreateGlobalStringPtr(e->str.str(), "", 0, &M());
        case EBin:
        {
            if (e->bop == LogicalAnd) 
                return gen_logical(e->lhs, e->rhs, true);
            if (e->bop == LogicalOr)
                return gen_logical(e->rhs, e->lhs, false);
            if (e->bop == Assign) {
                auto basep = getAddress(e->lhs);
                auto rhs = gen(e->rhs);
                auto s = B().CreateAlignedStore(rhs, basep, e->lhs->ty->align ? llvm::Align(e->lhs->ty->align) : llvm::MaybeAlign());
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
                case AtomicrmwAdd: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Add); goto BINOP_ATOMIC_RMW;
                case AtomicrmwSub: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Sub); goto BINOP_ATOMIC_RMW;
                case AtomicrmwXor: pop = static_cast<unsigned>(llvm::AtomicRMWInst::Xor); goto BINOP_ATOMIC_RMW;
                case AtomicrmwOr:  pop = static_cast<unsigned>(llvm::AtomicRMWInst::Or); goto BINOP_ATOMIC_RMW;
                case AtomicrmwAnd: pop = static_cast<unsigned>(llvm::AtomicRMWInst::And); goto BINOP_ATOMIC_RMW;
                BINOP_ATOMIC_RMW:
                    return B().CreateAtomicRMW(static_cast<llvm::AtomicRMWInst::BinOp>(pop), lhs, rhs, llvm::None,
                        llvm::AtomicOrdering::SequentiallyConsistent);
                case SAdd:
                  return B().CreateNSWAdd(lhs, rhs);
                case SSub:
                  return B().CreateNSWSub(lhs, rhs);
                case SMul:
                  return B().CreateNSWMul(lhs, rhs);
                case PtrDiff:
                {
                    auto sub = B().CreateSub(lhs, rhs);
                    auto s = getsizeof(e->lhs->castval->ty->p);
                    if (s != 1) {
                        auto c = llvm::ConstantInt::get(getIntPtr(), s);
                        return B().CreateExactSDiv(sub, c);
                    }
                    return sub;
                }
                case SAddP:
                    // getelementptr treat operand as signed, so we need to prmote to intptr type
                    if (!e->rhs->ty->isSigned()) {
                        lhs = B().CreateZExt(lhs, getIntPtr());
                    }
                    return B().CreateInBoundsGEP(
                    (
                        (e->lhs->ty->p->tags & TYVOID) || (e->lhs->k == EUnary && e->lhs->uop == AddressOf && e->lhs->ty->p->k == TYFUNCTION)
                    ) ? types[x8] : wrap(e->ty->p), lhs, {rhs});
                case EQ:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_EQ); goto BINOP_ICMP;
                case NE: 
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_NE); goto BINOP_ICMP;
                case UGT:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_UGT); goto BINOP_ICMP;
                case UGE:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_UGE); goto BINOP_ICMP;
                case ULT:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_ULT); goto BINOP_ICMP;
                case ULE: 
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_ULE); goto BINOP_ICMP;
                case SGT:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SGT); goto BINOP_ICMP;
                case SGE:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SGE); goto BINOP_ICMP;
                case SLT:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SLT); goto BINOP_ICMP;
                case SLE:
                    pop = static_cast<unsigned>(llvm::CmpInst::ICMP_SLE); goto BINOP_ICMP;
                BINOP_ICMP:
                    return B().CreateICmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
                case FEQ: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OEQ); goto BINOP_FCMP;
                case FNE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_ONE); goto BINOP_FCMP;
                case FGT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGT); goto BINOP_FCMP;
                case FGE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGE); goto BINOP_FCMP;
                case FLT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLT); goto BINOP_FCMP;
                case FLE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLE); goto BINOP_FCMP;
                BINOP_FCMP:
                    return B().CreateFCmp(static_cast<llvm::CmpInst::Predicate>(pop), lhs, rhs);
                case Comma:
                    return rhs;
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
            
                case Shr:  pop = llvm::Instruction::LShr; goto BINOP_SHIFT;
                case AShr: pop = llvm::Instruction::AShr; goto BINOP_SHIFT;
                case Shl:  pop = llvm::Instruction::Shl; goto BINOP_SHIFT;
            
                case And:  pop = llvm::Instruction::And; goto BINOP_BITWISE;
                case Xor:  pop = llvm::Instruction::Xor; goto BINOP_BITWISE;
                case Or:   pop = llvm::Instruction::Or; goto BINOP_BITWISE;
                BINOP_ARITH:
                    if (libtrace) {
                        Label iftrue = addBlock();
                        Label ifend = addBlock();
                        condbr(
                            isFloating(e->ty) ? 
                                B().CreateFCmp(llvm::CmpInst::Predicate::FCMP_OEQ, rhs, llvm::ConstantFP::getZero(types[xdouble])) : 
                                B().CreateIsNull(rhs),
                            iftrue, ifend);
                        after(iftrue);
                        B().CreateCall(lt_call_ty, isFloating(e->ty) ? lt_fdiv_zero : lt_div_zero);
                        B().CreateUnreachable();
                        after(ifend);
                    }
                BINOP_BITWISE:
                BINOP_SHIFT:
                return B().CreateBinOp(static_cast<llvm::Instruction::BinaryOps>(pop), lhs, rhs);
            }
        }
        case EIntLit:
           return llvm::ConstantInt::get(types[getNoSignTypeIndex(e->ty->tags)], e->ival);
        case EFloatLit:
          return llvm::ConstantFP::get((e->ty->tags & TYDOUBLE) ? types[xdouble] : types[xfloat], e->fval);
        case EVoid:
          return (void)gen(e->voidexpr), nullptr;
        case EUnary:
            switch (e->uop) {
                case SNeg: return B().CreateNSWNeg(gen(e->uoperand));
                case UNeg: return B().CreateNeg(gen(e->uoperand));
                case FNeg: return B().CreateFNeg(gen(e->uoperand));
                case Not:  return B().CreateNot(gen(e->uoperand));
                case AddressOf: return getAddress(e->uoperand);
                case Dereference:
                {
                  auto g = gen(e->uoperand);
                  auto ty = wrap(e->ty);
                  if (libtrace)
                    B().CreateCall(lt_call_ty3, lt_defef_nullptr, {g});
                  auto r = load(g, ty);
                  auto tags = e->uoperand->ty->p->tags;
                  if (tags & TYVOLATILE)
                    r->setVolatile(true);
                  if (tags & TYATOMIC)
                    r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
                  return r;
                }
                case LogicalNot:
                  return B().CreateIsNull(gen(e->uoperand));
            }
        case EVar:
        {
            auto pvar = getVar(e->sval);
            auto r = load(pvar, wrap(e->ty), e->ty->align);
            if (e->ty->tags & TYVOLATILE)
                r->setVolatile(true);
            if (e->ty->tags & TYATOMIC)
                r->setOrdering(llvm::AtomicOrdering::SequentiallyConsistent);
            return r;
        }
        case ECondition:
            return gen_condition(e->cond, e->cleft, e->cright);
        case ECast:
            return B().CreateCast(getCastOp(e->castop), gen(e->castval), wrap(e->ty));
        case EDefault:
            return llvm::Constant::getNullValue(wrap(e->ty));
        case ECall:
        {
            Value f, old;
            auto ty = cast<llvm::FunctionType>(wrap(e->callfunc->ty->p));
            if (e->callfunc->k == EUnary) {
              assert(e->callfunc->uop == AddressOf);
              f = getAddress(e->callfunc);
            } else {
              f = gen(e->callfunc);
            }
            size_t l = e->callargs.size();
            Value *buf = alloc.Allocate<Value>(l);
            for (size_t i = 0;i < l;++i)
                buf[i] = gen(e->callargs[i]); // eval argument from left to right
            if (libtrace) {
                SmallString<32> str;
                llvm::raw_svector_ostream OS(str);
                OS << e;
                old = buildlibtraceBeginCall(str.str(), e->loc.line, getFileStr(e->loc.id));
            }
            auto r = B().CreateCall(ty, f, ArrayRef<Value>(buf, l));
            if (libtrace)
                buildlibtraceAfterCall(old);
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
