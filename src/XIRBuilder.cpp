namespace {

using namespace llvm;

struct XIRBuilder {
	BasicBlock *BB;
	BasicBlock::InstListType *InstList;
	LLVMContext Context;
	DebugLoc loc;
	explicit XIRBuilder(LLVMContext &ctx): BB{nullptr}, InstList{nullptr}, Context{ctx}, loc(DebugLoc()) {}
	BasicBlock *GetInsertBlock() const { return BB; }
	BasicBlock::iterator GetInsertPoint() const { return BB->end(); }
	LLVMContext &getContext() const { return Context; }
	void SetInsertPoint(BasicBlock *TheBB) { BB = TheBB; InstList = &TheBB->getInstList(); }
	void insert(Instruction *I) {
		InstList.push_back(I);
	}
	Value *CreateBinOp(Instruction::BinaryOps Opc, Value *LHS, Value *RHS) {
    	return BinaryOperator::Create(Opc, LHS, RHS, Twine());
  	}
};

}
