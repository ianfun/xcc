namespace intepreter {

using Value = llvm::Constant*;
using Address = llvm::Constant**;
using llvm::ConstantExpr;

struct ExecutionSession {
	SmallVector<Value> globals;
};
struct CallFrame {
	Value* locals;
	CallFrame(unsigned num_locals): locals{new Value[num_locals]} {}
	~CallFrame() { delete [] locals; }
};

struct Function: public DiagnosticHelper {
	Stmt pc = nullptr, next_pc = nullptr;
	ExecutionSession &session;
	Stmt function_stmt;
	Value retVal = nullptr;
	CallFrame &frame;
	LLVMTypeConsumer &type_cache;
	const StmtEndMap stmt_map;
	enum StmtKind opCode;
	Fucntion(DiagnosticsEngine &engine, ExecutionSession &session, Stmt s, LLVMTypeConsumer &type_cache): DiagnosticHelper{engine}, session{session}, function_stmt{s}, type_cache{type_cache}, stmt_map{s} {}
	const ExecutionSession &getExecutionSession() const {
		return session;
	}
	void setLocal(unsigned idx, Value val) {
		frame.locals[idx] = val;
	}
	void setGlobal(unsigned idx, Value val) {
		session.globals[idx] = val;
	}
	Value operator()(const ArrayRef<Value> args, CallFrame &frame) {
		for (size_t i = 0;i < args.size();++i)
			session.values[i] = args[i];
		pc = s;
        loop();
        return retVal;
	}
	Value operator()(const ArrayRef<Value> args) {
		CallFrame frame(function_stmt->localSize);
		(*this)(args);
	}
	void warning(const StringRef &msg) {
		llvm::errs() << "intepreter warning: " << s << '\n';
	}
	// https://github.com/python/cpython/blob/main/Python/ceval.c
	// https://en.wikipedia.org/wiki/Instruction_cycle
	void loop(Stmt s) {
		for (;;) {
			// fetch
			fetch();
			if (!pc) break;
			// decode
			decode();	
			// execute
			execute(); 
			// repeat
			continue;
		}
	}
	void fetch() {
		pc = pc->next;
		if (!pc) {
			pc = stmt_map.getNext(s);
		}
	}
	void decode(const_Stmt s) {
		opCode = s->k;
	}
	void execute() {
		switch (opCode) {
			case SHead: llvm_unreachable("");
			case SIndirectBr:
				pc (getAddress(s->jump_addr)));
			case SCompound:
				pc = s->inner;
				break;
			case SDecl:
				type_cache->handleDecl(s);
				break;
			case SExpr:
				(void)eval(s->exprbody);
				break;
			case SNoReturnCall:
				(void)eval(s->call_expr);
				warning("noreturn call returns");
				pc = nullptr;
			case SGotoWithLoc:
    	    case SGotoWithLocName:
	       	case SGoto:
	       		pc = getLabel(s->location);
	       		break;
	       	case SCondJump:
	       		pc = getLabel(value_as_bool(eval(s->test)) ? s->T, s->F);
	       		break;
	       	case SAsm:
	       		warning("Assembly %R ignored in intepreter", s->asms);
	       		break;
	       	case SVarDecl:
	       		{
	       			for (const VarDecl &it: s->vars) {
               			if (it.ty->hasTag(TYTYPEDEF))
               				continue;
               			if (it.ty->getKind() == TYFUNCTION) {
               				// TODO: xxx
               				continue;
               			}
               			Value init = it.init ? eval(it.init) : llvm::Constant::getNullValue(type_cache.wrap(it.ty));
               			if (it.ty->isGlobalStorage()) {
               				setGlobal(it.idx, init);
               			} else {
               				setLocal(it.idx, init);
               			}
	       			}
	       		}
	       		break;
	       	case SDeclOnly:
	       		break;
	       	case SNamedLabel:
	       	case SLabel:
	       		break;
		}
	}
	Stmt *getLabel(unsigned idx) const {
		return function_stmt->labelMap[idx];
	}
	Address getAddress(Expr e) {

	}
	Value eval_cond(Expr e) {
		Value V = eval(e);
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
        return !V->isNullValue();
	}
	Value atomicOp(enum BinOp op, Value l, Value r) {
		switch (op) {
   		case AtomicrmwAdd: return ops_add(l, r);
   		case AtomicrmwSub: return ops_sub(l, r);
   		case AtomicrmwAnd: return ops_mul(l, r);
   		case AtomicrmwOr: return ops_or(l, r);
    	case AtomicrmwXor: return ops_or(l, r);
    	default: llvm_unreachable("invalid atomic op");
		}
	}
	Value eval(Expr e) {
		switch (e->k) {
	case EConstant: return e->C;
    case EBlockAddress:
    	return getLabel(e->addr);
    case EConstantArraySubstript: return llvm::ConstantExpr::getInBoundsGetElementPtr(wrap(e->ty->p), e->carray,
                                                                ConstantInt::get(type_cache.ctx, e->cidx));;
    case EConstantArray: return e->array;
    case EBin: 
    {
    	enum BinOp bop = e->bop;
    	switch (bop) {
    	case LogicalOr:
    		return llvm::ConstantInt::getBool(
    			type_cache.ctx, 
    			eval_cond(e->lhs) || eval_cond(e->rhs)
    		);
    	case LogicalAnd:
    		return llvm::ConstantInt::getBool(
    			type_cache.ctx, 
    			eval_cond(e->lhs) && eval_cond(e->rhs)
    		);
    	case Assign:
    		{
    			// limit sequence points to left => right
    			Address addr = getAddress(e->lhs);
    			Value val = eval(e->rhs);
    			return *addr = val;
    		}
    	case AtomicrmwAdd:
        case AtomicrmwSub:
        case AtomicrmwXor:
        case AtomicrmwOr: 
        case AtomicrmwAnd:
        	{
        		Address addr = getAddress(e->lhs);
        		Value prev = *addr;
        		Value r = eval(e->rhs);
        		*addr = atomicOp(bop, prev, r);
        		return prev;
        	}
     	}
    	// sequence points
    	Value lhs = eval(e->lhs);
    	Value rhs = eval(e->rhs);
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
    UAdd,
    SAdd,
    FAdd,
    CAdd,
    CFAdd,
    USub,
    SSub,
    FSub,
    CSub,
    CFSub,
    UMul,
    SMul,
    FMul,
    CMul,
    CFMul,
    UDiv,
    SDiv,
    FDiv,
    CSDiv,
    CUDiv,
    CFDiv,
    URem,
    SRem,
    FRem,
    Shr,
    AShr,
    Shl,
    And,
    Xor,
    Or,
    SAddP,
    PtrDiff
    Comma,
    Complex_CMPLX,
    EQ,
    NE,
    UGT,
    UGE,
    ULT,
    ULE,
    SGT,
    SGE,
    SLT,
    SLE,
    FEQ,
    FNE,
    FGT,
    FGE,
    FLT,
    FLE,
    CEQ,
    CNE,
    	}
    	llvm_unreachable("invalid UnaryOp");
    }
    case EUnary:
        if (e->uop == AddressOf && e->ty->p->getKind() == TYFUNCTION && !e->ty->hasTag(TYLVALUE))
            return maybe_print_paren(e->uoperand, OS), OS;
        OS << show(e->uop);
        maybe_print_paren(e->uoperand, OS);
        return OS;
    case EVoid: return maybe_print_paren(e->voidexpr, OS), OS;
    case EVar: {
        OS.changeColor(raw_ostream::MAGENTA);
        OS << e->varName->getKey();
        OS.resetColor();
        return OS;
    }
    case ECondition:
    	return value_as_bool(e->cond) ? eval(e->cleft) : eval(e->cright);
    case ECast:
    {
    	Value val = eval(e->castval);
    	return llvm::ConstantFoldCastOperand(getCastOp(e->castop), val, val->getTye(), DL);
    }
    case ESizeof:
//    	e->theType;
    	llvm_unreachable("VLA unsupported now");
    case ECall:
    	llvm_unreachable("Call unsupported now");
    	{
    		Value fn = eval(e->callfunc);
    		// CallFrame frame;
    		// e->callargs
    	}
    case ESubscript: 
    {
    	llvm::Type *ty = type_cache.wrap(e->left->ty->p);
    	Value obj = eval(e->left);
    	Value right = eval(e->right);
    	return ConstantExpr::getGetElementPtr(ty, obj, {right}, true);
    }
    case EArray:
        llvm_unreachable("array unsupported!");
    case EStruct:
        llvm_unreachable("struct unsupported!");
    case EMemberAccess:
    	llvm_unreachable("unsupported operation!");
    case EArrToAddress:
    	llvm_unreachable("unsupported operation!");
    case EPostFix: 
    {
    	Address p = getAddress(e->poperand);
        llvm::Type *ty = type_cache.wrap(e->ty);
        Value r = load(p, ty, a);
        Value v;
        if (e->ty->getKind() == TYPOINTER) {
             ty = type_cache.wrap(e->poperand->ty->p);
             v = B.CreateInBoundsGEP(ty, r, {e->pop == PostfixIncrement ? i32_1 : i32_n1});
         } else {
             v = (e->pop == PostfixIncrement) ? op_add(r, llvm::ConstantInt::get(ty, 1))
                                              : op_sub(r, llvm::ConstantInt::get(ty, 1));
         }
        *p = v;
    }
	}
};
struct Interpreter: public DiagnosticHelper
{
	Interpreter(DiagnosticsEngine &engine, LLVMTypeConsumer &type_cache): DiagnosticHelper{engine}, session{}, gen{gen} {}
	ExecutionSession session;
	LLVMTypeConsumer type_cache;
	ExecutionSession &getExecutionSession() {
		return session;
	}
	const ExecutionSession &getExecutionSession() const {
		return session;
	}
	std::unique_ptr<Function> compile(Stmt s) {
		return std::make_unique<Function>(engine, session, s, type_cache);
	}
};

}
