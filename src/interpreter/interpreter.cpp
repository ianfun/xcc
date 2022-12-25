namespace intepreter {

typedef union Value {
    uint64_t I;
    void *P;
    double D;
    float F;
    static inline union Value fromU64(uint64_t val) {
        union Value V;
        V.I = val;
        return V;
    }
    static inline union Value fromFloat(float val) {
        union Value V;
        V.F = val;
        return V;
    }
    static inline union Value fromDouble(double val) {
        union Value V;
        V.D = val;
        return V;
    }
    static union Value fromPointer(void *val) {
        union Value V;
        V.P = val;
        return V;
    }
    static union Value getZero();
    static union Value getVoid() {
        return getZero();
    }
} Value, *Address;

Value Value::getZero() {
    Value V;
    memset(&V, 0, sizeof(V));
    return V;
}

struct ExecutionSession {
	SmallVector<Value> globals;
    llvm::DataLayout DL;
    void resetNumGlobal(size_t num) {
        globals.resize_for_overwrite(num);
    }
	ExecutionSession(const llvm::DataLayout &DL): globals{}, DL{DL} {}
};
struct CallFrame {
	Value* locals;
	CallFrame(unsigned num_locals): locals{new Value[num_locals]} {}
	~CallFrame() { delete [] locals; }
};
static bool is_double(CType ty) {
    return ty->getFloatKind().getBitWidth() <= 32;
}
static bool is_double(Expr e) {
    return is_double(e->ty);
}
struct Function: public DiagnosticHelper {
	Stmt pc = nullptr, next_pc = nullptr;
	ExecutionSession &session;
	Stmt function_stmt;
	Value retVal = Value::getVoid();
	LLVMTypeConsumer &type_cache;
	const StmtEndMap stmt_map;
	CallFrame frame;
	enum StmtKind opCode;
	Function(DiagnosticsEngine &engine, ExecutionSession &session, Stmt s, LLVMTypeConsumer &type_cache): DiagnosticHelper{engine}, session{session}, function_stmt{s}, type_cache{type_cache}, stmt_map{s}, frame{s->localSize} {}
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
	Address getVarAddress(unsigned idx) {
		if (idx >= function_stmt->localStart)
			return getLocalAddr(idx);
		return getGlobalAddr(idx);
	}
	Value operator()(const ArrayRef<Value> args) {
		for (size_t i = 0;i < args.size();++i)
			setLocal(i, args[i]);
		pc = function_stmt;
        loop();
        return retVal;
	}
	// https://github.com/python/cpython/blob/main/Python/ceval.c
	// https://en.wikipedia.org/wiki/Instruction_cycle
	void loop() {
		for (;;) {
			fetch();
			if (!pc) break;
			decode();	
			execute(); 
			continue;
		}
	}
	void fetch() {
		if (pc->next) 
            pc = pc->next;
		else 
            pc = stmt_map.getNext(pc);
	}
	void decode() {
		opCode = pc->k;
	}
	void execute() {
		switch (opCode) {
            case SFunction:
			case SHead: 
                llvm_unreachable("");
            case SReturn:
                pc = nullptr;
                if (pc->ret)
                    retVal = eval(pc->ret);
                break;
            case SSwitch:
            {
                uint64_t test = eval(pc->itest).I;
                for (const auto &it : pc->gnu_switchs) {
                    test -= it.CaseStart->getLimitedValue();
                    if (test <= it.range.getLimitedValue()) {
                        pc = getLabel(it.label);
                        break;
                    }
                }
                for (const auto &it : pc->switchs) {
                    if (test == it.CaseStart->getLimitedValue()) {
                        pc = getLabel(it.label);
                        break;
                    }
                }
                pc = getLabel(pc->sw_default);
            } break;
			case SIndirectBr:
				pc = reinterpret_cast<Stmt>(getAddress(pc->jump_addr));
                break;
			case SCompound:
				pc = pc->inner;
				break;
			case SDecl:
				type_cache.handleDecl(pc);
				break;
			case SExpr:
				(void)eval(pc->exprbody);
				break;
			case SNoReturnCall:
				(void)eval(pc->call_expr);
				warning("noreturn call returns");
				pc = nullptr;
			case SGotoWithLoc:
    	    case SGotoWithLocName:
	       	case SGoto:
	       		pc = getLabel(pc->location);
	       		break;
	       	case SCondJump:
	       		pc = getLabel(eval_cond(pc->test) ? pc->T : pc->F);
	       		break;
	       	case SAsm:
	       		warning("Assembly %R ignored in intepreter", pc->asms.str());
	       		break;
	       	case SVarDecl:
	       		{
	       			for (const VarDecl &it: pc->vars) {
               			if (it.ty->hasTag(TYTYPEDEF))
               				continue;
               			if (it.ty->getKind() == TYFUNCTION) {
               				// TODO: xxx
               				continue;
               			}
               			Value init = it.init ? eval(it.init) : Value::getZero();
               			if (it.ty->isGlobalStorage()) {
               				setGlobal(it.idx, init);
               			} else {
               				setLocal(it.idx, init);
               			}
	       			}
	       		}
	       		break;
	       	case SNamedLabel:
	       	case SLabel:
	       		break;
		}
	}
	Stmt getLabel(unsigned idx) const {
		return stmt_map.getLabel(idx);
	}
	Address getAddress(Expr e) {
        switch (e->k) {
        case EVar: return getVarAddress(e->sval);
        case EMemberAccess: llvm_unreachable("");
        case EUnary:
            switch (e->uop) {
            case AddressOf: return getAddress(e->uoperand);
            case Dereference: return reinterpret_cast<Address>(eval(e->uoperand).P);
            case C__real__:
            case C__imag__: llvm_unreachable("");
            default: llvm_unreachable("");
            }
        case ESubscript:
        	llvm_unreachable("");
        case EArrToAddress: return getAddress(e->voidexpr);
        case EString: llvm_unreachable("");
        default:
            if (e->ty->hasTag(TYREPLACED_CONSTANT)) {
                auto e2 = reinterpret_cast<ReplacedExpr *>(e);
                return getVarAddress(e2->id);
            }
            fatal("intepreter internal error: getAddress(): unhandled expression");
            return nullptr;
        }
	}
    bool eval_cond(Expr e) {
        return eval(e).I != 0;
	}
	Value atomicOp(enum BinOp op, Value l, Value r, CType ty) {
        if (ty->isInteger()) {
		  switch (op) {
   		    case AtomicrmwAdd:
   		    	return Value::fromU64(l.I + r.I);
   		    case AtomicrmwSub:
   		    	return Value::fromU64(l.I - r.I);
   		    case AtomicrmwAnd:
   		    	return Value::fromU64(l.I & r.I);
   		    case AtomicrmwOr: 
   		    	return Value::fromU64(l.I | r.I);
    	    case AtomicrmwXor:
    	    	return Value::fromU64(l.I ^ r.I);
    	    default: llvm_unreachable("invalid atomic op");
            }
        }
        llvm_unreachable("");
	}
	Value eval(Expr e) {
		switch (e->k) {
            case EConstant:
            {
                const llvm::Constant * const C = e->C;
                if (const ConstantInt *CI = dyn_cast<ConstantInt>(C)) {
                    return Value::fromU64(CI->getValue().getLimitedValue());
                }
                if (const ConstantFP *CF = dyn_cast<ConstantFP>(C)) {
                    const APFloat &F = CF->getValue();
                    switch (APFloat::SemanticsToEnum(F.getSemantics())) {
                        case APFloat::S_IEEEhalf:
                        case APFloat::S_BFloat:
                        case APFloat::S_IEEEsingle:
                            return Value::fromFloat(F.convertToFloat());
                        default:
                            return Value::fromDouble(F.convertToDouble());
                    }
                }
            }
            case ECallCompilerBuiltinCall:
            case ECallImplictFunction:
            case EBuiltinCall: llvm_unreachable("unsupported builtin function");
            case EInitList: llvm_unreachable("");
            case EBlockAddress:
        	   return Value::fromPointer(getLabel(e->addr));
            case EConstantArraySubstript: 
                llvm_unreachable("");
            case EString: llvm_unreachable("");
            case EBin:
    {
        enum BinOp bop = e->bop;
        switch (bop) {
        default: break;
        case LogicalOr:
            return Value::fromU64(eval_cond(e->lhs) || eval_cond(e->rhs));
        case LogicalAnd:
            return Value::fromU64(eval_cond(e->lhs) && eval_cond(e->rhs));
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
                *addr = atomicOp(bop, prev, r, e->ty);
                return prev;
            }
        }
        // sequence points
        unsigned pop;
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
        return Value::fromU64(lhs.I + rhs.I);
    case SAdd:
        return Value::fromU64((int64_t)lhs.I + (int64_t)rhs.I);
    case FAdd:
        if (is_double(e))
            return Value::fromDouble(lhs.D + rhs.D);
        return Value::fromFloat(lhs.F + rhs.F);
    case CAdd:
    case CFAdd:
        llvm_unreachable("");
    case USub:
        return Value::fromU64(lhs.I - rhs.I);
    case SSub:
        return Value::fromU64((int64_t)lhs.I - (int64_t)rhs.I);
    case FSub:
        if (is_double(e))
            return Value::fromDouble(lhs.D - rhs.D);
        return Value::fromFloat(lhs.F - rhs.F);
    case CSub:
    case CFSub: llvm_unreachable("");
    case SMul:
        return Value::fromU64((int64_t)lhs.I * (int64_t)rhs.I);
    case UMul:
        return Value::fromU64(lhs.I * rhs.I);
    case FMul:
        if (is_double(e))
            return Value::fromDouble(lhs.D * rhs.D);
        return Value::fromFloat(lhs.F * rhs.F);
    case CMul:
    case CFMul: llvm_unreachable("");
    case UDiv:
        return Value::fromU64(lhs.I / rhs.I);
    case SDiv:
        return Value::fromU64((int64_t)lhs.I / (int64_t)rhs.I);
    case FDiv:
        if (is_double(e))
            return Value::fromDouble(lhs.D / rhs.D);
        return Value::fromFloat(lhs.F / rhs.F);
    case CSDiv:
    case CUDiv:
    case CFDiv: llvm_unreachable("");
    case URem:
        return Value::fromU64(lhs.I % rhs.I);
    case SRem:
        return Value::fromU64((int64_t)lhs.I % (int64_t)rhs.I);
    case FRem:
        llvm_unreachable("FRem is un-implemented");
    case Shr:
        return Value::fromU64(lhs.I >> rhs.I);
    case AShr:
        return Value::fromU64((int64_t)lhs.I >> (int64_t)rhs.I);
    case Shl:
        return Value::fromU64(lhs.I << rhs.I);
    case And:
        return Value::fromU64(lhs.I & rhs.I);
    case Xor:
        return Value::fromU64(lhs.I ^ rhs.I);
    case Or:
        return Value::fromU64(lhs.I | rhs.I);
    case SAddP:
        llvm_unreachable("");
    case PtrDiff:
        llvm_unreachable("");
    case Comma:
        return rhs;
    case Complex_CMPLX:
        llvm_unreachable("");
    case EQ:
        return Value::fromU64(lhs.I == rhs.I);
    case NE: 
        return Value::fromU64(lhs.I != rhs.I);
    case UGT:
        return Value::fromU64(lhs.I > rhs.I);
    case UGE:
        return Value::fromU64(lhs.I >= rhs.I);
    case ULT:
        return Value::fromU64(lhs.I < rhs.I);
    case ULE:
        return Value::fromU64(lhs.I <= rhs.I);
    case SGT:
        return Value::fromU64((int64_t)lhs.I > (int64_t)rhs.I);
    case SGE:
        return Value::fromU64((int64_t)lhs.I >= (int64_t)rhs.I);
    case SLT:
        return Value::fromU64((int64_t)lhs.I < (int64_t)rhs.I);
    case SLE:
        return Value::fromU64((int64_t)lhs.I <= (int64_t)rhs.I);
    case FEQ:
        if (is_double(e))
            return Value::fromDouble(lhs.D == rhs.D);
        return Value::fromFloat(lhs.F == rhs.F);
    case FNE:
        if (is_double(e))
            return Value::fromDouble(lhs.D != rhs.D);
        return Value::fromFloat(lhs.F != rhs.F);
    case FGT:
        if (is_double(e))
            return Value::fromDouble(lhs.D > rhs.D);
        return Value::fromFloat(lhs.F > rhs.F);
    case FGE:
        if (is_double(e))
            return Value::fromDouble(lhs.D >= rhs.D);
        return Value::fromFloat(lhs.F >= rhs.F);
    case FLT:
        if (is_double(e))
            return Value::fromDouble(lhs.D < rhs.D);
        return Value::fromFloat(lhs.F < rhs.F);
    case FLE:
        if (is_double(e))
            return Value::fromDouble(lhs.D <= rhs.D);
        return Value::fromFloat(lhs.F <= rhs.F);
    case CEQ:
    case CNE:
        llvm_unreachable("");
    }   
    }
            case EUnary:
                
            case EVoid: 
                return (void)eval(e->voidexpr), Value::getVoid();
            case EVar:
            {
                Address p = getVarAddress(e->sval);
                switch (e->ty->getKind()) {
                case TYPRIM:
                    if (e->ty->isInteger()) {
                        switch (e->ty->getIntegerKind().asLog2()) {
                        case 0:
                        case 1: return Value::fromU64(*reinterpret_cast<bool*>(p));
                        case 3: return Value::fromU64(*reinterpret_cast<uint8_t*>(p));
                        case 4: return Value::fromU64(*reinterpret_cast<uint16_t*>(p));
                        case 5: return Value::fromU64(*reinterpret_cast<uint32_t*>(p));
                        case 6: return Value::fromU64(*reinterpret_cast<uint64_t*>(p));
                        default: llvm::report_fatal_error("int128 unsupported");
                        }
                    }
                    switch (e->ty->getFloatKind().asEnum()) {
                    case F_Half:
                    case F_BFloat:
                    case F_Float: return Value::fromFloat(*reinterpret_cast<float*>(p));
                    case F_Double: return Value::fromDouble(*reinterpret_cast<double*>(p));
                    case F_x87_80:
                    case F_Quadruple:
                    case F_PPC128: // return Value::fromLongDouble(*reinterpret_cast<long double*>(p));
                    default: llvm::report_fatal_error("Unsupported floating number in this machine");
                    }
                default: llvm_unreachable("un-implemented");
                }
            }
            case ECondition:
        	return eval_cond(e->cond) ? eval(e->cleft) : eval(e->cright);
            case ECast:
            {
            	llvm_unreachable("");
            }
            case ESizeof:
            	llvm_unreachable("VLA unsupported now");
            case ECall:
            	llvm_unreachable("Call unsupported now");
            case ESubscript: 
            	llvm_unreachable("");
            case EMemberAccess:
            	llvm_unreachable("unsupported operation!");
            case EArrToAddress:
            	llvm_unreachable("unsupported operation!");
            case EPostFix: 
            	llvm_unreachable("");
	       }
           llvm_unreachable("invalid ExprKind");
    }
};

// parse code, line by line(like a interpreter)
struct IncrementalParser {
    CompilerInstance &CI;
    IncrementalParser(CompilerInstance &CI): CI{CI} {
        CI.createParser();
    }
    unsigned InputCount = 0;
    // note: this function does not copy the input string
    // Return true on sucess, false if parsing error occurred
    bool Parse(StringRef input_line, TranslationUnit &TU) {
        SourceMgr &SM = CI.getSourceManager();
        char file_name[30];
        int N = snprintf(file_name, sizeof(file_name), "input_line_%u", InputCount++);
        
        size_t InputSize = input_line.size();
        std::unique_ptr<llvm::MemoryBuffer> MB(
            llvm::WritableMemoryBuffer::getNewUninitMemBuffer(InputSize + 2, StringRef(file_name, N))
        );
        char *MBStart = const_cast<char*>(MB->getBufferStart());
        memcpy(MBStart, input_line.data(), InputSize);
        MBStart[InputSize++] = '\n';
        MBStart[InputSize] = '\0';
        SM.addMemoryBuffer(MB.release());

        Lexer &L = CI.getParser().l;
        if (L.tok.tok == TEOF) {
            L.initC();
        }
        CI.Parse(TU);
        unsigned errs = CI.getDiags().getNumErrors();
        CI.getDiags().reset();
        return errs == 0;
    }
};
struct IncrementalExecutor
{
    CompilerInstance &CI;
    ExecutionSession session;
    IncrementalExecutor(CompilerInstance &CI): CI{CI}, session{CI.getOptions().DL} {}
	const ExecutionSession &getExecutionSession() const {
		return session;
	}
	std::unique_ptr<Function> compile(Stmt s) {
		return std::make_unique<Function>(CI.getDiags(), session, s, CI.getTypeCache());
	}
	Value runFunction(Stmt s, const ArrayRef<Value> &args) {
		Function F(CI.getDiags(), session, s, CI.getTypeCache());
		return F(args);
	}
    bool Execute(const TranslationUnit &TU) {
        llvm::errs() << "\nExecuting...\n";
        llvm::errs() << "\nfinished.\n";
        return true;
    }
};

struct Interpreter {
    IncrementalParser parser;
    IncrementalExecutor exe;
    SmallVector<Stmt> asts;
    Interpreter(CompilerInstance &CI): parser{CI}, exe{CI} {}
    bool Parse(StringRef input_line, TranslationUnit &TU) {
        return parser.Parse(input_line, TU);
    }
    bool Execute(TranslationUnit &TU) {
        return exe.Execute(TU);
    }
    bool ParseAndExecute(StringRef input_line) {
        TranslationUnit TU;
        bool ret;
        if (Parse(input_line, TU)) {
            // exe.runFunction(nullptr, {});
            return Execute(TU);
        } else {ret = false;}
        return ret;
    }
    ~Interpreter() {
        StmtReleaser releaser;
        for (Stmt ast: asts)
            releaser.Visit(ast);
    }
};

}
