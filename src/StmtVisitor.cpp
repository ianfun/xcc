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
            puts("SVarDecl");
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
