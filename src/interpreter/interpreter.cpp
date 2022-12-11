namespace intepreter {

using Value = llvm::Constant*;
using Address = llvm::Constant**;
using llvm::ConstantExpr;

struct ExecutionSession {
	Value *globals;
	ExecutionSession(unsigned num_vars): globals{new Value[num_vars]} {}
	~ExecutionSession() { delete [] globals; }
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
	LLVMTypeConsumer &type_cache;
	const StmtEndMap stmt_map;
	CallFrame frame;
	enum StmtKind opCode;
	Fucntion(DiagnosticsEngine &engine, ExecutionSession &session, Stmt s, LLVMTypeConsumer &type_cache): DiagnosticHelper{engine}, session{session}, function_stmt{s}, type_cache{type_cache}, stmt_map{s}, frame{s->localSize} {}
	const ExecutionSession &getExecutionSession() const {
		return session;
	}
	void setLocal(unsigned idx, Value val) {
		frame.locals[idx] = val;
	}
	void setGlobal(unsigned idx, Value val) {
		session.globals[idx] = val;
	}
	void setVar(unsigned idx, Value val) {
		if (idx >= function_stmt->localStart)
			setLocal(idx, val);
		else
			setGlobal(idx, val);
	}
	Value getLocal(unsigned idx) {
		return frame.locals[idx];
	}
	Address getLocalAddr(unsigned idx) {
		return &frame.locals[idx];
	}
	Value getGlobal(unsigned idx) {
		return session.globals[idx];
	}
	Address getGlobalAddr(unsigned idx) {
		return &session.globals[idx];
	}
	Value getVarValue(unsigned idx) {
		if (idx >= function_stmt->localStart)
			return getLocal(idx);
		return getGlobal(idx);
	}
	Address getVarAddress(unsigned idx) {
		if (idx >= function_stmt->localStart)
			return getLocalAddr(idx);
		return getGlobalAddr(idx);
	}
	Value operator()(const ArrayRef<Value> args) {
		for (size_t i = 0;i < args.size();++i)
			session.setLocal(i, args[i]);
		pc = function_stmt;
        loop();
        return retVal;
	}
	// https://github.com/python/cpython/blob/main/Python/ceval.c
	// https://en.wikipedia.org/wiki/Instruction_cycle
	void loop() {
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
		return stmt_map[idx];
	}
	Address getAddress(Expr e) {
        case EVar: return getVarAddress[e->sval];
        case EMemberAccess: llvm_unreachable("");
        case EUnary:
            switch (e->uop) {
            case AddressOf: return getAddress(e->uoperand);
            case Dereference: return eval(e->uoperand);
            case C__real__:
            case C__imag__: llvm_unreachable("");
            default: llvm_unreachable("");
            }
        case ESubscript:
        	llvm_unreachable("");
        case EArrToAddress: return getAddress(e->voidexpr);
        case EConstantArray: return e->array;
        case EArray:
        case EStruct: llvm_unreachable("");
        default:
            if (e->ty->hasTag(TYREPLACED_CONSTANT)) {
                auto e2 = reinterpret_cast<ReplacedExpr *>(e);
                return getVarValue[e2->id];
            }
            fatal("intepreter internal error: getAddress(): unhandled expression");
            return nullptr;
        }
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
   		case AtomicrmwAdd:
   			return llvm::ConstantExpr::getAdd(l, r);
   		case AtomicrmwSub:
   			return llvm::ConstantExpr::getSub(l, r);
   		case AtomicrmwAnd:
   			return llvm::ConstantExpr::getAnd(l, r);
   		case AtomicrmwOr: 
   			return llvm::ConstantExpr::getOr(l, r);
    	case AtomicrmwXor:
    		return llvm::ConstantExpr::getXor(l, r);
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
    case UAdd:
    case SAdd:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue() + CI2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::getAdd(lhs, rhs);
    case FAdd:
    	if (const ConstantFP *CF = dyn_cast<ConstantFP>(lhs)) {
    		if (const ConstantFP *CF2 = dyn_cast<ConstantFP>(rhs)) {
    			return llvm::ConstantFP::get(type_cache.ctx, CF->getValue() + CF2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::FAdd, lhs, rhs);
    case CAdd:
    case CFAdd:
    	llvm_unreachable("");
    case USub:
    case SSub:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue() - CI2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::getSub(lhs, rhs);
    case FSub:
    	if (const ConstantFP *CF = dyn_cast<ConstantFP>(lhs)) {
    		if (const ConstantFP *CF2 = dyn_cast<ConstantFP>(rhs)) {
    			return llvm::ConstantFP::get(type_cache.ctx, CF->getValue() - CF2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::FSub, lhs, rhs);
    case CSub:
    case CFSub: llvm_unreachable("");
    case SMul:
    case UMul:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue() * CI2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::getMul(lhs, rhs);
    case FMul:
    	if (const ConstantFP *CF = dyn_cast<ConstantFP>(lhs)) {
    		if (const ConstantFP *CF2 = dyn_cast<ConstantFP>(rhs)) {
    			return llvm::ConstantFP::get(type_cache.ctx, CF->getValue() * CF2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::FMul, lhs, rhs);
    case CMul:
    case CFMul: llvm_unreachable("");
    case UDiv:
    case SDiv:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			APInt val = bop == SDiv ?)CI->getValue().sdiv(CI2->getValue()) : CI->getValue().udiv(CI2->getValue());
    			return llvm::ConstantInt::get(type_cache.ctx, val);
    		}
    	}
    	return llvm::ConstantExpr::getDiv(lhs, rhs);
    case FDiv:
    	if (const ConstantFP *CF = dyn_cast<ConstantFP>(lhs)) {
    		if (const ConstantFP *CF2 = dyn_cast<ConstantFP>(rhs)) {
    			return llvm::ConstantFP::get(type_cache.ctx, CF->getValue() / CF2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::FDiv, lhs, rhs);
    case CSDiv:
    case CUDiv:
    case CFDiv:
    case URem:
      	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue().urem(CI2->getValue()));
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::URem, lhs, rhs);
    case SRem:
      	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue().srem(CI2->getValue()));
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::SRem, lhs, rhs);
    case FRem:
    	if (const ConstantFP *CF = dyn_cast<ConstantFP>(lhs)) {
    		if (const ConstantFP *CF2 = dyn_cast<ConstantFP>(rhs)) {
    			APFloat F = CF->getValue();
    			F.remainder(CF2->getValue());
    			return llvm::ConstantFP::get(type_cache.ctx, F);
    		}
    	}
    	return llvm::ConstantExpr::get(llvm::Instruction::FRem, lhs, rhs);
    case Shr:
      	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue().shr(CI2->getValue()));
    		}
    	}
    	return llvm::ConstantExpr::getShr(lhs, rhs);
    case AShr:
      	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue().ashr(CI2->getValue()));
    		}
    	}
    	return llvm::ConstantExpr::getAShr(lhs, rhs);
    case Shl:
      	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue().shl(CI2->getValue()));
    		}
    	}
    	return llvm::ConstantExpr::getShl(lhs, rhs);
    case And:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue() & CI2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::getAnd(lhs, rhs);
    case Xor:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue() ^ CI2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::getXor(lhs, rhs);
    case Or:
    	if (const ConstantInt *CI = dyn_cast<ConstantInt>(lhs)) {
    		if (const ConstantInt* CI2 = dyn_cast<ConstantInt>(rhs)) {
    			return llvm::ConstantInt::get(type_cache.ctx, CI->getValue() | CI2->getValue());
    		}
    	}
    	return llvm::ConstantExpr::getOr(lhs, rhs);
    case SAddP:
    	return llvm::ConstantExpr::getGetElementPtr(type_cache.wrap(e->ty->p), 
    		lhs, {rhs});
    case PtrDif:
    	llvm_unreachable("");
    case Comma:
    	return rhs;
    case Complex_CMPLX:
    	llvm_unreachable("");
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
        return llvm::ConstantExpr::getICmp(pop, lhs, rhs);
    case FEQ: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OEQ); goto BINOP_FCMP;
    case FNE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_ONE); goto BINOP_FCMP;
    case FGT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGT); goto BINOP_FCMP;
    case FGE: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OGE); goto BINOP_FCMP;
    case FLT: pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLT); goto BINOP_FCMP;
    case FLE:
        pop = static_cast<unsigned>(llvm::FCmpInst::FCMP_OLE);
        goto BINOP_FCMP;
BINOP_FCMP:
		return llvm::ConstantExpr::getFCmp(pop, lhs, rhs);
    case CEQ:
    case CNE:
    	llvm_unreachable("");
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
    case EVar: return getVarValue(e->sval);
    case ECondition:
    	return eval_cond(e->cond) ? eval(e->cleft) : eval(e->cright);
    case ECast:
    {
    	Value val = eval(e->castval);
    	return llvm::ConstantFoldCastOperand(getCastOp(e->castop), val, val->getTye(), DL);
    }
    case ESizeof:
    	llvm_unreachable("VLA unsupported now");
    case ECall:
    	llvm_unreachable("Call unsupported now");
    case ESubscript: 
    	llvm_unreachable("");
    case EArray:
        llvm_unreachable("array unsupported!");
    case EStruct:
        llvm_unreachable("struct unsupported!");
    case EMemberAccess:
    	llvm_unreachable("unsupported operation!");
    case EArrToAddress:
    	llvm_unreachable("unsupported operation!");
    case EPostFix: 
    	llvm_unreachable("");
	}
	llvm_unreachable("invalid ExprKind");
};
struct Interpreter: public DiagnosticHelper
{
	Interpreter(DiagnosticsEngine &engine, unsigned num_tags, unsigned num_vars, LLVMTypeConsumer &type_cache): DiagnosticHelper{engine}, session{}, type_cache{type_cache} {
		type_cache.reset(num_tags);
	}
	ExecutionSession session;
	LLVMTypeConsumer &type_cache;
	const ExecutionSession &getExecutionSession() const {
		return session;
	}
	std::unique_ptr<Function> compile(Stmt s) {
		return std::make_unique<Function>(engine, session, s, type_cache);
	}
	llvm::Constant *runFunction(Stmt s, const ArrayRef<Value> &args) {
		Function F(engine, session, s, type_cache);
		return F(args);
	}
};

}
