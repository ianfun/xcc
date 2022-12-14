// https://clang.llvm.org/doxygen/classclang_1_1StmtVisitor.html

template <typename ErrorReporter>
struct VLAJumpDetector {
    ArrayRef<label_t> labels;
    ErrorReporter &reporter;
    SmallVector<SmallVector<location_t, 0>> vla_founded;
    // the input must be sorted
    VLAJumpDetector(const ArrayRef<label_t> labels, ErrorReporter &reporter): labels{labels}, reporter{reporter}, vla_founded{} {}
    void Visit(Stmt s) {
        Stmt start;
        switch (s->k) {
        case SHead:
            start = s->next;
            goto COMPOUND;
        case SCompound:
            start = s->inner;
COMPOUND:
            vla_founded.push_back({});
            for (Stmt ptr = start; ptr; ptr = ptr->next)
                Visit(ptr);
            vla_founded.pop_back();
            break;
        case SNamedLabel:
            if (std::binary_search(labels.begin(), labels.end(), s->label)) {
                reporter.Report(s, vla_founded.back());
            }
            break;
        case SVarDecl:
            for (const VarDecl &decl: s->vars) {
                if (!decl.ty->hasTag(TYTYPEDEF)) {
                    vla_founded.back().push_back(decl.loc);
                }
            }
            break;
        default: break;
        }
    }
};

struct StmtEndMap {
    DenseMap<Stmt, Stmt> endMap;
    Stmt *labelMap;
    StmtEndMap(Stmt s) {
        assert(s->k == SFunction);
        labelMap = new Stmt[s->numLabels];
        for (Stmt ptr = s->next;ptr;ptr = ptr->next)
            Visit(ptr);
    }
    ~StmtEndMap() {
        delete [] labelMap;
    }
    void Visit(Stmt s) {
        switch (s->k) {
        case SCompound:
        {
            Stmt lastPtr = nullptr;
            for (Stmt ptr = s->inner; ptr; ptr = ptr->next) {
                lastPtr = ptr;
                Visit(ptr);
            }
            if (lastPtr && s->next)
                endMap[lastPtr] = s->next;
        } break;
        default:
            break;
        }
    }
    Stmt getNext(Stmt s) const {
        auto it = endMap.find(s);
        if (it != endMap.end())
            return it->second;
        return nullptr;
    }
    Stmt getLabel(size_t i) const {
        return labelMap[i];
    }
};

template <typename T, bool enableExpr>
struct StmtVisitor {
    void ActOnStmt(Stmt ) {}
    void ActOnExpr(Expr ) {}
    void VisitExpr(Expr e) {
        assert(enableExpr);
        switch (e->k) {
            case EBin:
                VisitExpr(e->lhs);
                VisitExpr(e->rhs);
                break;
            case EUnary:
                VisitExpr(e->uoperand);
                break;
            case EVoid:
                VisitExpr(e->voidexpr);
                break;
            case ECondition:
                VisitExpr(e->cond);
                VisitExpr(e->cleft);
                VisitExpr(e->cright);
                break;
            case ECast:
                VisitExpr(e->castval);
                break;
            case ECall:
                for (const auto it: e->callargs) 
                    VisitExpr(it);
                break;
            case ESubscript:
                VisitExpr(e->left);
                VisitExpr(e->right);
                break;
            case EInitList:
                for (const Initializer &it: e->inits)
                    VisitExpr(it.value);
                break;
            case EMemberAccess:
                VisitExpr(e->obj);
                break;
            case EArrToAddress:
                VisitExpr(e->arr3);
                break;
            case EPostFix:
                VisitExpr(e->poperand);
                break;
            default: break;
        }
        static_cast<T*>(this)->ActOnExpr(e);
    }
    void Visit(Stmt s) {
        switch (s->k) {
            case SHead:
                for (Stmt ptr = s->next; ptr; ptr = ptr->next)
                    Visit(ptr);
                break;
            case SCompound:
                for (Stmt ptr = s->inner; ptr; ptr = ptr->next)
                    Visit(ptr);
                break;
            case SFunction:
                for (Stmt ptr = s->funcbody->next; ptr; ptr = ptr->next)
                    Visit(ptr);
                break;
            default: 
            {
                if (enableExpr) {
                    switch (s->k) {
                        case SExpr:
                            VisitExpr(s->exprbody);
                            break;
                        case SNoReturnCall:
                            VisitExpr(s->call_expr);
                            break;
                        case SReturn:
                            if (s->ret)
                                VisitExpr(s->ret);
                            break;
                        case SCondJump:
                            VisitExpr(s->test);
                            break;
                        case SSwitch:
                            VisitExpr(s->itest);
                            break;
                        case SIndirectBr:
                            VisitExpr(s->jump_addr);
                            break;
                        default: break;
                    }
                }
            }
        }
        static_cast<T*>(this)->ActOnStmt(s);
    }
};
// Free all AST objects within the statement (xvector/xstring, ...)
struct StmtReleaser: public StmtVisitor<StmtReleaser, true> {
    void ActOnExpr(Expr e) {
        switch (e->k) {
        case ECall: e->callargs.free(); break;
        case EInitList: e->inits.free();
        default: break;
        }
    }
    void ActOnStmt(Stmt s) {
        switch (s->k) {
        case SSwitch:
            s->switchs.free();
            s->gnu_switchs.free();
            break;
        case SVarDecl:
            s->vars.free();
            break;
        case SFunction:
            s->functy->params.free();
            break;
        default: break;
        }
    }
};
