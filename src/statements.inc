// C statements and declarations -- generated by gen.py
enum StmtKind: uint8_t {
    SHead,
    SCompound,
    SLabel,
    SNamedLabel,
    SGotoWithLocName,
    SGotoWithLoc,
    SGoto,
    SCondJump,
    SSwitch,
    SIndirectBr,
    SReturn,
    SExpr,
    SNoReturnCall,
    SAsm,
    SVarDecl,
    SDecl,
    SFunction
};
struct NullStmt {
    Stmt next = nullptr;
    enum StmtKind k;
};
template <typename T>
struct StmtIterator {
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;
	T s;
	StmtIterator(): s{nullptr} {}
	StmtIterator(std::nullptr_t): s{nullptr} {}
	StmtIterator(T Node) {
		switch (Node->k) {
			case SCompound:
				this->s = Node->inner;
				break;
			case SHead:
				this->s = Node->next;
				break;
			case SFunction:
				this->s = Node->funcbody->next;
				break;
    		default:
    			this->s = nullptr;
    	}
	}
	static StmtIterator get_raw(T Node) {
		StmtIterator I;
		I.s = Node;
		return I;
	}
	T operator*() {
		return s;
	}
	StmtIterator<T> &operator++();
	T operator->() const {
		return s;
	}
	bool operator !=(StmtIterator<T> other) const {
		return this->s != other.s;
	}
	bool operator ==(StmtIterator<T> other) const {
		return this->s == other.s;
	}
};
} // end namespace xcc

template <> struct std::iterator_traits<xcc::StmtIterator<xcc::Stmt>> {
    using difference_type = std::ptrdiff_t;
    using value_type = xcc::Stmt;
    using reference = xcc::Stmt&;
    using size_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
};
template <> struct std::iterator_traits<xcc::StmtIterator<xcc::const_Stmt>> {
    using difference_type = std::ptrdiff_t;
    using value_type = xcc::const_Stmt;
    using reference = xcc::const_Stmt&;
    using size_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
};

namespace xcc {
struct OpaqueStmt {
    Stmt next = nullptr;
    StmtKind k;
    using iterator = StmtIterator<Stmt>;
    using const_iterator = StmtIterator<const_Stmt>;
    iterator child_begin() { return iterator(this); }
    const_iterator child_begin() const { return const_iterator(this); }
	void viewAST() const;
	raw_ostream &writeGraph(raw_ostream &, const llvm::Twine &Title = "AST") const;
	iterator child_end() { return iterator(); }
	const_iterator child_end() const { return const_iterator(); }
    bool isTerminator() const {
        switch (k) {
            case SGoto:
            case SReturn:
            case SCondJump:
            case SIndirectBr:
            case SNoReturnCall:
            case SGotoWithLoc:
            case SGotoWithLocName:
                return true;
            default:
                return false;
        }
    }
union alignas(void*) {
    struct alignas(void*) {
        Stmt inner;
    };
    struct alignas(void*) {
        label_t label;
    };
    struct alignas(void*) {
        label_t label2;
        IdentRef labelName;
        location_t labelLoc;
    };
    struct alignas(void*) {
        label_t location3;
        location_t goto_loc3;
        IdentRef goto_name;
    };
    struct alignas(void*) {
        label_t location2;
        location_t goto_loc;
    };
    struct alignas(void*) {
        label_t location;
    };
    struct alignas(void*) {
        Expr test;
        label_t T;
        label_t F;
    };
    struct alignas(void*) {
        Expr itest;
        xvector<SwitchCase> switchs;
        xvector<GNUSwitchCase> gnu_switchs;
        label_t sw_default;
        location_t sw_default_loc;
    };
    struct alignas(void*) {
        Expr jump_addr;
    };
    struct alignas(void*) {
        Expr ret;
        location_t ret_loc;
    };
    struct alignas(void*) {
        Expr exprbody;
    };
    struct alignas(void*) {
        Expr call_expr;
    };
    struct alignas(void*) {
        xstring asms;
    };
    struct alignas(void*) {
        xvector<VarDecl> vars;
    };
    struct alignas(void*) {
        unsigned decl_idx;
        CType decl_ty;
    };
    struct alignas(void*) {
        unsigned func_idx;
        IdentRef funcname;
        CType functy;
        Stmt funcbody;
        unsigned numLabels;
        label_t *indirectBrs;
        unsigned localStart;
        unsigned localSize;
        location_t funcDefLoc;
    };
};
};
struct HeadStmt {
    Stmt next = nullptr;
    enum StmtKind k=SHead;
};
struct CompoundStmt {
    Stmt next = nullptr;
    enum StmtKind k=SCompound;
    struct alignas(void*) {
        Stmt inner;
    };
};
struct LabelStmt {
    Stmt next = nullptr;
    enum StmtKind k=SLabel;
    struct alignas(void*) {
        label_t label;
    };
};
struct NamedLabelStmt {
    Stmt next = nullptr;
    enum StmtKind k=SNamedLabel;
    struct alignas(void*) {
        label_t label2;
        IdentRef labelName;
        location_t labelLoc;
    };
};
struct GotoWithLocNameStmt {
    Stmt next = nullptr;
    enum StmtKind k=SGotoWithLocName;
    struct alignas(void*) {
        label_t location3;
        location_t goto_loc3;
        IdentRef goto_name;
    };
};
struct GotoWithLocStmt {
    Stmt next = nullptr;
    enum StmtKind k=SGotoWithLoc;
    struct alignas(void*) {
        label_t location2;
        location_t goto_loc;
    };
};
struct GotoStmt {
    Stmt next = nullptr;
    enum StmtKind k=SGoto;
    struct alignas(void*) {
        label_t location;
    };
};
struct CondJumpStmt {
    Stmt next = nullptr;
    enum StmtKind k=SCondJump;
    struct alignas(void*) {
        Expr test;
        label_t T;
        label_t F;
    };
};
struct SwitchStmt {
    Stmt next = nullptr;
    enum StmtKind k=SSwitch;
    struct alignas(void*) {
        Expr itest;
        xvector<SwitchCase> switchs;
        xvector<GNUSwitchCase> gnu_switchs;
        label_t sw_default;
        location_t sw_default_loc;
    };
};
struct IndirectBrStmt {
    Stmt next = nullptr;
    enum StmtKind k=SIndirectBr;
    struct alignas(void*) {
        Expr jump_addr;
    };
};
struct ReturnStmt {
    Stmt next = nullptr;
    enum StmtKind k=SReturn;
    struct alignas(void*) {
        Expr ret;
        location_t ret_loc;
    };
};
struct ExprStmt {
    Stmt next = nullptr;
    enum StmtKind k=SExpr;
    struct alignas(void*) {
        Expr exprbody;
    };
};
struct NoReturnCallStmt {
    Stmt next = nullptr;
    enum StmtKind k=SNoReturnCall;
    struct alignas(void*) {
        Expr call_expr;
    };
};
struct AsmStmt {
    Stmt next = nullptr;
    enum StmtKind k=SAsm;
    struct alignas(void*) {
        xstring asms;
    };
};
struct VarDeclStmt {
    Stmt next = nullptr;
    enum StmtKind k=SVarDecl;
    struct alignas(void*) {
        xvector<VarDecl> vars;
    };
};
struct DeclStmt {
    Stmt next = nullptr;
    enum StmtKind k=SDecl;
    struct alignas(void*) {
        unsigned decl_idx;
        CType decl_ty;
    };
};
struct FunctionStmt {
    Stmt next = nullptr;
    enum StmtKind k=SFunction;
    struct alignas(void*) {
        unsigned func_idx;
        IdentRef funcname;
        CType functy;
        Stmt funcbody;
        unsigned numLabels;
        label_t *indirectBrs;
        unsigned localStart;
        unsigned localSize;
        location_t funcDefLoc;
    };
};
static uint8_t stmt_size_map[] = {
    sizeof(HeadStmt),
    sizeof(CompoundStmt),
    sizeof(LabelStmt),
    sizeof(NamedLabelStmt),
    sizeof(GotoWithLocNameStmt),
    sizeof(GotoWithLocStmt),
    sizeof(GotoStmt),
    sizeof(CondJumpStmt),
    sizeof(SwitchStmt),
    sizeof(IndirectBrStmt),
    sizeof(ReturnStmt),
    sizeof(ExprStmt),
    sizeof(NoReturnCallStmt),
    sizeof(AsmStmt),
    sizeof(VarDeclStmt),
    sizeof(DeclStmt),
    sizeof(FunctionStmt)
};
static constexpr size_t stmt_max_size = std::max({sizeof(HeadStmt), sizeof(CompoundStmt), sizeof(LabelStmt), sizeof(NamedLabelStmt), sizeof(GotoWithLocNameStmt), sizeof(GotoWithLocStmt), sizeof(GotoStmt), sizeof(CondJumpStmt), sizeof(SwitchStmt), sizeof(IndirectBrStmt), sizeof(ReturnStmt), sizeof(ExprStmt), sizeof(NoReturnCallStmt), sizeof(AsmStmt), sizeof(VarDeclStmt), sizeof(DeclStmt), sizeof(FunctionStmt)});

template <typename T>
StmtIterator<T> &StmtIterator<T>::operator++() {
	s = s->next;
	return *this;
}
