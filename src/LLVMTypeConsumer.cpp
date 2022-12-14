struct LLVMTypeConsumer
{
    enum TypeIndex {
        voidty,
        i1ty,
        i8ty,
        u8ty,
        i16ty,
        u16ty,
        i32ty,
        u32ty,
        i64ty,
        u64ty,
        i128ty,
        u128ty,
        bfloatty,
        halfty,
        floatty,
        doublety,
        fp80ty,
        fp128ty,
        fp128ppcty,
        fdecimal32ty,
        fdecimal64ty,
        fdecimal128ty,
        ptrty,
        TypeIndexHigh
    };
	LLVMContext &ctx;
    SmallVector<llvm::Type*, 0> tags;
   	llvm::StructType *_complex_float, *_complex_double;
    llvm::IntegerType *integer_types[8];
    llvm::Type *float_types[FloatKind::MAX_KIND];
    llvm::PointerType *pointer_type;
    llvm::Type *void_type;
    llvm::DIBuilder *di;
    OnceAllocator alloc;
    DenseMap<CType, llvm::FunctionType *> function_type_cache;
    DenseMap<Expr, llvm::Value *> vla_size_map;
    llvm::ConstantPointerNull *null_ptr;
    llvm::ConstantInt *i32_1, *i32_0, *i32_n1;
    llvm::IntegerType *intptrTy;
    unsigned pointerSizeInBits;
    llvm::FunctionType *implict_func_ty;
    llvm::ConstantInt *i1_0, *i1_1;
    llvm::PointerType *AllocaTy;
    const Options &options;
    
    uint64_t getsizeof(llvm::Type *ty) { return options.DL.getTypeStoreSize(ty); }
    uint64_t getsizeof(CType ty) {
        assert((!ty->isVLA()) && "VLA should handled in other case");
        type_tag_t Align = ty->getAlignLog2Value();
        if (Align)
            return uint64_t(1) << Align;
        if (ty->isVoid() || ty->getKind() == TYFUNCTION)
            return 1;
        return getsizeof(wrap(ty));
    }
    uint64_t getsizeof(Expr e) { return getsizeof(e->ty); }
    uint64_t getAlignof(llvm::Type *ty) { return options.DL.getPrefTypeAlign(ty).value(); }
    uint64_t getAlignof(CType ty) {
        type_tag_t Align = ty->getAlignLog2Value();
        if (Align)
            return uint64_t(1) << Align;
        return getAlignof(wrap(ty));
    }
    uint64_t getAlignof(Expr e) { return getAlignof(e->ty); }
    LLVMContext &getLLVMContext() {
        return ctx;
    }
    llvm::Type *wrapNoComplexScalar(CType ty) {
        assert(ty->getKind() == TYPRIM);
        if (ty->isInteger())
            return integer_types[ty->getIntegerKind().asLog2()];
        return float_types[static_cast<uint64_t>(ty->getFloatKind())];
    }
    static unsigned getTypeIndex(CType ty) {
        // TODO: Complex, Imaginary
        if (ty->hasTag(TYVOID))
            return voidty;
        if (ty->isInteger()) {
            bool s = ty->isSigned();
            auto k = ty->getIntegerKind();
            switch (k.asLog2()) {
            case 1: return s ? i1ty : i1ty;
            case 3: return s ? i8ty : u8ty;
            case 4: return s ? i16ty : u16ty;
            case 5: return s ? i32ty : u32ty;
            case 6: return s ? i64ty : u64ty;
            case 7: return s ? i128ty : u128ty;
            default: llvm_unreachable("invalid IntegerKind");
            }
        }
        auto k = ty->getFloatKind();
        switch (k.asEnum()) {
        case F_Half: return halfty;
        case F_BFloat: return bfloatty;
        case F_Float: return floatty;
        case F_Double: return doublety;
        case F_x87_80: return fp80ty;
        case F_Quadruple: return fp128ty;
        case F_PPC128: return fp128ppcty;
        case F_Decimal32: return fdecimal32ty;
        case F_Decimal64: return fdecimal64ty;
        case F_Decimal128: return fdecimal128ty;
        case F_Invalid: break;
        }
        llvm_unreachable("invalid FloatKind");
    }
    llvm::Type *wrapFloating(CType ty) { return float_types[static_cast<uint64_t>(ty->getFloatKind())]; }
    llvm::StructType *wrapComplex(const_CType ty) {
        if (ty->isFloating()) {
            FloatKind k = ty->getFloatKind();
            if (k.equals(F_Double))
                return _complex_double;
            if (k.equals(F_Float))
                return _complex_float;
            auto T = float_types[(uint64_t)k];
            return llvm::StructType::get(T, T);
        }
        return wrapComplexForInteger(ty);
    }
    llvm::StructType *wrapComplexForInteger(const_CType ty) {
        auto T = integer_types[ty->getIntegerKind().asLog2()];
        return llvm::StructType::get(T, T);
    }
    void handleDecl(Stmt s) {
    	assert(s->k == SDecl);
    	CType ty = s->decl_ty;
        if (ty->isEnum())
            return;
        IdentRef Name = ty->getTagName();
        RecordDecl *RD = ty->getRecord();
        SmallString<64> str;
        if (Name) {
            str.assign("struct.");
            str += Name->getKey();
        }
        else
            str.assign("struct.anon");
        if (!RD) {
            tags[s->decl_idx] = llvm::StructType::create(ctx, str.str());
            return;
        }
        const auto &fields = RD->fields;
        size_t l = fields.size();
        llvm::Type **buf = alloc.Allocate<llvm::Type *>(l);
        for (size_t i = 0; i < l; ++i)
            buf[i] = wrap(fields[i].ty);
        tags[s->decl_idx] = llvm::StructType::create(ctx, llvm::makeArrayRef(buf, l), str.str());
    }
    llvm::Type *wrap(CType ty) {
        assert(ty && "wrap a nullptr");
        switch (ty->getKind()) {
        case TYPRIM: return wrapPrim(ty);
        case TYPOINTER: return pointer_type;
        case TYTAG: {
            if (ty->isEnum())
                return integer_types[5];
            return tags[ty->idx];
        }
        case TYFUNCTION: {
            // fast case: search cached
            auto it = function_type_cache.find(ty);
            if (it != function_type_cache.end())
                return it->second;
            // slow case: wrap all
            size_t l = ty->params.size();
            llvm::Type **buf = alloc.Allocate<llvm::Type *>(l);
            size_t i = 0;
            for (const auto &it : ty->params)
                buf[i++] = wrap(it.ty);
            auto T = llvm::FunctionType::get(wrap(ty->ret), ArrayRef<llvm::Type *>{buf, i}, ty->isVarArg);
            function_type_cache[ty] = T; // add to cache
            return T;
        }
        case TYARRAY: return llvm::ArrayType::get(wrap(ty->arrtype), ty->arrsize);
        case TYBITINT: return llvm::IntegerType::get(ctx, ty->getBitIntBits());
        case TYBITFIELD: llvm_unreachable("bit field should handled other case!");
        case TYVLA: llvm_unreachable("VLA should handled other case!");
        default: llvm_unreachable("unexpected type!");
        }
    }
    llvm::Type *wrapPrim(const_CType ty) {
        assert(ty->getKind() == TYPRIM);
        if (LLVM_UNLIKELY(ty->isComplex()))
            return wrapComplex(ty);
        if (ty->isVoid())
            return void_type;
        // for _Imaginary types, they are just reprsentationed as floating types(or interger types as a extension).
        if (ty->isInteger()) {
            return integer_types[ty->getIntegerKind().asLog2()];
        }
        return float_types[static_cast<uint64_t>(ty->getFloatKind())];
    }
    void reset(unsigned num_tags) {
        tags.resize_for_overwrite(num_tags);
    }
    LLVMTypeConsumer(LLVMContext &ctx, const Options &options): ctx{ctx}, alloc{}, options{options} {
        pointer_type = llvm::PointerType::get(ctx, 0);
        void_type = llvm::Type::getVoidTy(ctx);
        null_ptr = llvm::ConstantPointerNull::get(pointer_type);
        integer_types[0] = llvm::Type::getInt1Ty(ctx);
        integer_types[1] = nullptr;
        integer_types[2] = nullptr;
        integer_types[3] = llvm::Type::getInt8Ty(ctx);
        integer_types[4] = llvm::Type::getInt16Ty(ctx);
        integer_types[5] = llvm::Type::getInt32Ty(ctx);
        integer_types[6] = llvm::Type::getInt64Ty(ctx);
        integer_types[7] = llvm::Type::getInt128Ty(ctx);

        float_types[F_Half] = llvm::Type::getHalfTy(ctx);
        float_types[F_BFloat] = llvm::Type::getBFloatTy(ctx);
        float_types[F_Float] = llvm::Type::getFloatTy(ctx);
        float_types[F_Double] = llvm::Type::getDoubleTy(ctx);
        float_types[F_Quadruple] = llvm::Type::getFP128Ty(ctx);
        float_types[F_PPC128] = llvm::Type::getPPC_FP128Ty(ctx);
        _complex_double = llvm::StructType::get(float_types[F_Double], float_types[F_Double]);
        _complex_float = llvm::StructType::get(float_types[F_Float], float_types[F_Float]);
        intptrTy = options.DL.getIntPtrType(getLLVMContext());
        pointerSizeInBits = intptrTy->getBitWidth();
        implict_func_ty = llvm::FunctionType::get(integer_types[5], {}, true);

        llvm::IntegerType *i32Ty = cast<llvm::IntegerType>(integer_types[5]);
        i32_n1 = llvm::ConstantInt::get(i32Ty, -1);
        i32_1 = llvm::ConstantInt::get(i32Ty, 1);
        i32_0 = llvm::ConstantInt::get(i32Ty, 0);
        i1_0 = llvm::ConstantInt::getFalse(getLLVMContext());
        i1_1 = llvm::ConstantInt::getTrue(getLLVMContext());

        AllocaTy = llvm::PointerType::get(ctx, options.DL.getAllocaAddrSpace());
    }
};
