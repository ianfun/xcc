static
int64_t clamp(int64_t Value, int64_t Low, int64_t High) {
  return std::min(High, std::max(Low, Value));
}

llvm::Value *IRGen::GenBuiltinCall(Expr e) {
    using namespace llvm;
    ArrayRef<Expr> Args = e->cbc_args;
    switch (e->cbc_ID) {
    case BI__builtin_setjmp:
    {
        Value *Buf = gen(Args[0]);
        Value *FrameAddr = call(Intrinsic::frameaddress, type_cache.i32_0, type_cache.AllocaTy);
        store(Buf, FrameAddr);
        Value *StackAddr = call(Intrinsic::stacksave);
        Value *StackSaveSlot = gep(type_cache.AllocaTy, Buf, 2);
        store(StackSaveSlot, StackAddr);
        return call(Intrinsic::eh_sjlj_setjmp, Buf);
    }
    case BI__builtin_longjmp:
    {
        Value *Buf = gen(Args[0]);
        call(Intrinsic::eh_sjlj_longjmp, Buf);
        createUnreachable();
        insertBB = addBB();
        return nullptr;
    }
    default:
        llvm_unreachable("unsupported builtin function");
    }
}
