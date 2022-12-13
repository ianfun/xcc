// Graph.cpp - Printing Stmt's Graph(DOT) using llvm's GraphWriter

struct GraphNodeIterator {
    using element = const_Stmt;
    using value_type = element;
    using size_type = std::size_t;
    using difference_type=std::ptrdiff_t;
    using reference = const_Stmt&;
    element s;
    GraphNodeIterator(): s{nullptr} {}
    GraphNodeIterator(std::nullptr_t): s{nullptr} {}
    GraphNodeIterator(const GraphNodeIterator &other): s{other.s} {}
    GraphNodeIterator(element s): s{s} {}
    element operator*() { return s; }
    GraphNodeIterator &operator++() {
        assert(s && "++ at end iterator");
        s = s->next;
        return *this;
    }
    element operator->() const {
        assert(s && "dereference from end iterator");
        return s;
    }
    bool operator !=(GraphNodeIterator other) const {
        return this->s != other.s;
    }
    bool operator ==(GraphNodeIterator other) const {
        return this->s == other.s;
    }
};
} // end namespace xcc

namespace std {

template <> struct std::iterator_traits<xcc::GraphNodeIterator> {
    using difference_type = xcc::GraphNodeIterator::difference_type;
    using value_type = xcc::GraphNodeIterator::value_type;
    using reference = xcc::GraphNodeIterator::reference;
    using size_type = xcc::GraphNodeIterator::size_type;
    using iterator_category = std::input_iterator_tag;
};

} // end namespace std

namespace llvm {

template <> struct GraphTraits<xcc::const_Stmt> 
{
    using NodeRef = xcc::const_Stmt;
    using nodes_iterator = xcc::GraphNodeIterator;
    using ChildIteratorType = xcc::OpaqueStmt::const_iterator;

    static NodeRef getEntryNode(xcc::const_Stmt s) {
        return s;
    }
    static nodes_iterator nodes_begin(xcc::const_Stmt s) {
        return nodes_iterator(s);
    }
    static nodes_iterator nodes_end(xcc::const_Stmt s) {
        return nodes_iterator();
    }
    static ChildIteratorType child_begin(NodeRef Node) {
        return Node->child_begin();
    }
    static ChildIteratorType child_end(NodeRef Node) {
        return Node->child_end();
    } 
};

template <> struct DOTGraphTraits<xcc::const_Stmt>: public DefaultDOTGraphTraits {
    DOTGraphTraits(bool isSimple = false): DefaultDOTGraphTraits{isSimple} {}
    static SmallString<256> buffer;
    static xcc::AstDumper<false> ast_dumper;
    static raw_svector_ostream OS;
    static std::string getNodeLabel(xcc::const_Stmt Node, xcc::const_Stmt Graph) {
        ast_dumper.dump(Node);
        std::string labelStr = std::string(buffer);
        buffer.clear();
        return labelStr;
    }
    static std::string getGraphName(xcc::const_Stmt) {
        return "Abstract Syntax Tree";
    }
};

typedef DOTGraphTraits<xcc::const_Stmt> xcc_DOTGraphTraits_const_Stmt;
llvm::SmallString<256> xcc_DOTGraphTraits_const_Stmt::buffer{};
raw_svector_ostream xcc_DOTGraphTraits_const_Stmt::OS{buffer};
xcc::AstDumper<false> xcc_DOTGraphTraits_const_Stmt::ast_dumper{OS};


} // end namespace llvm

namespace xcc {

void OpaqueStmt::viewAST() const {
    return llvm::ViewGraph(this, "AST");
}

raw_ostream &OpaqueStmt::writeGraph(raw_ostream &OS, const llvm::Twine &Title) const {
    return llvm::WriteGraph(OS, this, false, Title);
}
