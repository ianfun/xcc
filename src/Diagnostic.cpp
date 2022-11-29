enum DiagnosticLevel: uint8_t {
    Ignored,
    Note,
    Remark,
    Warning,
    Error,
    PPError,
    LexError,
    ParseError,
    EvalError,
    TypeError,
    Fatal
};
struct Diagnostic {
    using storage_type = uintmax_t;

    const char *fmt = nullptr;
    SmallVector<storage_type, 5> data{};
    SmallVector<FixItHint, 0> FixItHints{};
    location_t loc = 0;
    SmallVector<SourceRange> ranges{};
    enum DiagnosticLevel level = Ignored;
    // default constructor - construct a invalid Diagnostic
    Diagnostic() = default;
    Diagnostic(const char *fmt, location_t loc = 0) : fmt{fmt}, loc{loc} { }
    template <typename T> void write_impl(const T *ptr) { data.push_back(reinterpret_cast<storage_type>(ptr)); }
    void write_impl(const SourceRange &range) {
        ranges.push_back(range);
    }
    void write_impl(const StringRef &str) {
        data.push_back(static_cast<storage_type>(str.size()));
        write_impl(str.data());
    }
    void write_impl(const std::string &str) {
        data.push_back(static_cast<storage_type>(str.size()));
        write_impl(str.data());
    }
    void write_impl(const SmallVectorImpl<char> &str) {
        data.push_back(static_cast<storage_type>(str.size()));
        write_impl(str.data());
    }
    void write_impl(unsigned char c) { data.push_back(static_cast<storage_type>(c)); }
    void write_impl(char c) { data.push_back(static_cast<storage_type>(static_cast<unsigned char>(c))); }
    void write_impl(uint16_t c) { data.push_back(static_cast<storage_type>(c)); }
    void write_impl(int16_t c) { data.push_back(static_cast<storage_type>(static_cast<uint16_t>(c))); }
    void write_impl(uint32_t c) { data.push_back(static_cast<storage_type>(c)); }
    void write_impl(int32_t c) { data.push_back(static_cast<storage_type>(static_cast<uint32_t>(c))); }
    void write_impl(uint64_t c) { data.push_back(static_cast<storage_type>(c)); }
    void write_impl(int64_t c) { data.push_back(static_cast<storage_type>(static_cast<uint64_t>(c))); }
    template <typename T> void write(const T &a) { write_impl(a); }
    template <typename T> void write(const T &&a) { write_impl(std::move(a)); }
    template <typename T, typename... Args> void write(const T &a, const Args &...args) {
        write_impl(a);
        write(args...);
    }
    template <typename T> const T get(unsigned idx) const { return reinterpret_cast<T>(data[idx]); }
    void FormatDiagnostic(SmallVectorImpl<char> &OutStr) const {
        llvm::raw_svector_ostream OS(OutStr);
        unsigned idx = 0;
        auto p = reinterpret_cast<const unsigned char *>(fmt);

        for (;;) {
            unsigned char i = *p++;
            if (i == '%') {
                switch (*p++) {
                case 's': OS << get<const char *>(idx++); break;
                case 'S':
                    OS << lquote;
                    llvm::printEscapedString(StringRef(get<const char *>(idx++)), OS);
                    OS << rquote;
                    break;
                case 'r': {
                    size_t len = data[idx++];
                    auto msg = get<const char *>(idx++);
                    OS << StringRef(msg, len);
                } break;
                case 'R': {
                    OS << lquote;
                    size_t len = data[idx++];
                    auto msg = get<const char *>(idx++);
                    llvm::printEscapedString(StringRef(msg, len), OS);
                    OS << rquote;
                } break;
                case 'd':
                case 'i': OS << static_cast<int>(data[idx++]); break;
                case 'I': OS << lquote << get<IdentRef>(idx++)->getKey() << rquote; break;
                case 'Z': OS << static_cast<uint64_t>(data[idx++]); break;
                case 'z': OS << static_cast<size_t>(data[idx++]); break;
                case 'u': OS << static_cast<unsigned>(data[idx++]); break;
                case 'U': {
                    auto codepoint = static_cast<uint32_t>(data[idx++]);
                    OS << "<U+";
                    // max codepoint is 0b100001111111111111111
                    if (codepoint <= 0xFFFF) {
                        char buf[4], *p = buf;
                        *p++ = hexed(codepoint >> 12);
                        *p++ = hexed(codepoint >> 8);
                        *p++ = hexed(codepoint >> 4);
                        *p++ = hexed(codepoint);
                        OS << StringRef(buf, 4);
                    } else {
                        char buf[8], *p = buf;
                        *p++ = hexed(codepoint >> 28);
                        *p++ = hexed(codepoint >> 24);
                        *p++ = hexed(codepoint >> 20);
                        *p++ = hexed(codepoint >> 16);

                        *p++ = hexed(codepoint >> 12);
                        *p++ = hexed(codepoint >> 8);
                        *p++ = hexed(codepoint >> 4);
                        *p++ = hexed(codepoint);
                        OS << StringRef(buf, 8);
                    }
                    OS << '>';
                } break;
                case 'c': OS << static_cast<unsigned char>(data[idx++]); break;
                case 'C': {
                    char c = static_cast<char>(data[idx++]);
                    OS << lquote;
                    llvm::printEscapedString(StringRef(&c, 1), OS);
                    OS << rquote;
                } break;
                case 'f': OS << static_cast<double>(data[idx++]); break;
                case 'V': (*get<const llvm::Value *>(idx++)).print(OS); break;
                case 'L': (*get<const llvm::Type *>(idx++)).print(OS); break;
                case 'E': OS << lquote << get<const Expr>(idx++) << rquote; break;
                case 'e': OS << get<const Expr>(idx++); break;
                case 'T': OS << lquote << get<const CType>(idx++) << rquote; break;
                case 't': OS << get<const CType>(idx++); break;
                case 'h': OS.write_hex(static_cast<unsigned long long>(data[idx++])); break;
                case 'p': OS << get<const void *>(idx++); break;
                case 'o':
                    // errno_t
                    OS << llvm::sys::StrError(static_cast<int>(data[idx++]));
                    break;
#if WINDOWS
                case 'w': {
                    LPSTR msg = nullptr;
                    if (::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                             FORMAT_MESSAGE_IGNORE_INSERTS,
                                         NULL, static_cast<DWORD>(data[idx++]),
                                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, nullptr)) {
                        OS << (const char *)msg;
                        ::LocalFree(msg);
                    }
                } break;
#endif
                case 'A': get<const APInt *>(idx++)->print(OS, false); break;
                case 'a': get<const APInt *>(idx++)->print(OS, true); break;
                default: llvm_unreachable("invalid format specifier");
                }
            } else if (i == 0) {
                break;
            } else {
                OS.write(i);
            }
        }
    }
    void reset(const char *msg, enum DiagnosticLevel level, location_t loc = 0) {
        this->data.clear();
        this->loc = loc;
        this->level = level;
        this->msg = msg;
    }
};
struct DiagnosticConsumer {
    DiagnosticConsumer(const DiagnosticConsumer&) = delete;
    unsigned NumWarnings = 0;
    unsigned NumErrors = 0;
    unsigned getNumErrors() const { return NumErrors; }
    unsigned getNumWarnings() const { return NumWarnings; }
    void clear() { NumWarnings = NumErrors = 0; }
    typedef void (*PHandleDiagnosticTy)(void *self, const Diagnostic &Info);
    typedef void (*PFinalizeTy)(void *self);
    PHandleDiagnosticTy PHandleDiagnostic;
    PFinalizeTy PFinalize;
    void finalize() {
        if (PFinalize)
            PFinalize(this);
    }
    void HandleDiagnostic(const Diagnostic &Diag) {
        if (Diag.level == Warning)
            ++NumWarnings;
        else if (Diag.level >= Error)
            ++NumErrors;
        PHandleDiagnostic(this, Diag);
    }
    DiagnosticConsumer(PHandleDiagnosticTy impl, PFinalizeTy f = nullptr) : PHandleDiagnostic{impl}, PFinalize{f} {};
};
struct TextDiagnosticPrinter : public DiagnosticConsumer {
    static constexpr auto noteColor = raw_ostream::GREEN, remarkColor = raw_ostream::BLUE,
                          fixitColor = raw_ostream::GREEN, caretColor = raw_ostream::GREEN,
                          warningColor = raw_ostream::MAGENTA, templateColor = raw_ostream::CYAN,
                          errorColor = raw_ostream::RED, fatalColor = raw_ostream::RED,
                          savedColor = raw_ostream::SAVEDCOLOR;
    llvm::raw_ostream &OS;
    bool ShowColors;
    struct SourceMgr *SM;
    TextDiagnosticPrinter(llvm::raw_ostream &OS = llvm::errs(), struct SourceMgr *SM = nullptr)
        : DiagnosticConsumer{&HandleDiagnosticImpl, &finalizeImpl}, OS{OS}, ShowColors{OS.has_colors()}, SM{SM} { }
    void realHandleDiagnostic(const Diagnostic &Info);
    static void HandleDiagnosticImpl(void *self, const Diagnostic &Info) {
        return reinterpret_cast<TextDiagnosticPrinter *>(self)->realHandleDiagnostic(Info);
    }
    bool hasSourceMgr() const { return SM != nullptr; }
    void setSourceMgr(struct SourceMgr *SM) { this->SM = SM; }
    static void finalizeImpl(void *self) { return reinterpret_cast<TextDiagnosticPrinter *>(self)->realfinalize(); }
    void realfinalize() {
        if (NumWarnings)
            OS << NumWarnings << (NumWarnings == 1 ? " warning" : " warnings");
        if (NumWarnings && NumErrors)
            OS << " and ";
        if (NumErrors)
            OS << NumErrors << (NumErrors == 1 ? " error" : " errors");
        if (NumWarnings || NumErrors) {
            OS << " generated.\n";
        }
    }
    void printSource(const source_location &loc);
    void write_loc(const source_location &loc);
};
struct DiagnosticsEngine {
    unsigned ErrorLimit = 0;
    Diagnostic CurrentDiagnostic;
    SmallVector<DiagnosticConsumer*, 1> consumers;
    void addConsumer(DiagnosticConsumer *C) {
        consumers.push_back(C);
    }
    unsigned getNumConsumers() {
        return consumers.size();
    }
    DiagnosticConsumer *getFirstConsumer() {
        return consumers.front();
    }
    DiagnosticConsumer *getLastConsumer() {
        return consumers.back();
    }
    void EmitCurrentDiagnostic() {
        for (const auto C: consumers)
            C->HandleDiagnostic(CurrentDiagnostic);
    }
    unsigned getNumWarnings() const {
        unsigned total = 0;
        for (const auto C: consumers)
            total += C->getNumWarnings();
        return total;
    }
    unsigned getNumErrors() const {
        unsigned total = 0;
        for (const auto C: consumers)
            total += C->getNumErrors();
        return total;
    }
};
struct DiagnosticBuilder {
    DiagnosticsEngine &engine;
    inline DiagnosticBuilder(DiagnosticsEngine &engine) : engine{engine} {}
    ~DiagnosticBuilder() { engine.EmitCurrentDiagnostic(); }
    void addFixHint(const FixItHint &Hint) {
        engine.CurrentDiagnostic.FixItHints.push_back(Hint);
    }
    template <typename T> 
    const DiagnosticBuilder &operator<<(const T &V) const {
        engine.CurrentDiagnostic.write_impl(V);
    }
};
// A helper class to emit Diagnostics
// such as parse_error(), lex_error(), warning() functions
struct DiagnosticHelper {
    DiagnosticsEngine &engine;
    DiagnosticHelper(DiagnosticsEngine &engine): engine{engine} {}
    unsigned getNumErrors() const {
        return engine.getNumErrors();
    }
    unsigned getNumWarnings() const {
        return engine.getNumWarnings();
    }
#define DIAGNOSTIC_HANDLER(HANDLER, LEVEL) \
    template <typename... Args> DiagnosticBuilder HANDLER(const char *msg, const Args &...args) { \
        engine.CurrentDiagnostic.reset(msg, LEVEL); \
        engine.CurrentDiagnostic.write(args...); \
        return DiagnosticBuilder(engine);\
    } \
    DiagnosticBuilder HANDLER(const char *msg) { \
        engine.CurrentDiagnostic.reset(msg, LEVEL); \
        return DiagnosticBuilder(engine);\
    } \
    template <typename... Args> DiagnosticBuilder HANDLER(location_t loc, const char *msg, const Args &...args) { \
        engine.CurrentDiagnostic.reset(msg, LEVEL, loc); \
        engine.CurrentDiagnostic.write(args...); \
        return DiagnosticBuilder(engine);\
    }\
    template <typename... Args> DiagnosticBuilder HANDLER(location_t loc, const char *msg) { \
        engine.CurrentDiagnostic.reset(msg, LEVEL, loc); \
        return DiagnosticBuilder(engine);\
    }
    DIAGNOSTIC_HANDLER(note, Note)
    DIAGNOSTIC_HANDLER(remark, Remark)
    DIAGNOSTIC_HANDLER(warning, Warning)
    DIAGNOSTIC_HANDLER(pp_error, PPError)
    DIAGNOSTIC_HANDLER(lex_error, LexError)
    DIAGNOSTIC_HANDLER(parse_error, ParseError)
    DIAGNOSTIC_HANDLER(type_error, TypeError)
    DIAGNOSTIC_HANDLER(error, Error)
    DIAGNOSTIC_HANDLER(fatal, Fatal)
    void expect(location_t loc, const char *item) {  parse_error(loc, "%s expected", item); }
    void expectExpression(location_t loc) {  parse_error(loc, "%s", "expected expression"); }
    void expectStatement(location_t loc) {  parse_error(loc, "%s", "expected statement"); }
    void expectLB(location_t loc) {  parse_error(loc, "%s", "missing " lquote "(" rquote); }
    void expectRB(location_t loc) {  parse_error(loc, "%s", "missing " lquote ")" rquote); }
};
struct EvalHelper: public DiagnosticHelper {
    EvalHelper(DiagnosticsEngine &engine): DiagnosticHelper(engine) {}
    uint64_t force_eval(Expr e, location_t cloc) {
        if (e->k != EConstant) {
            type_error(cloc, "not a constant expression: %E", e);
            return 0;
        }
        if (const auto CI = dyn_cast<ConstantInt>(e->C)) {
            if (CI->getValue().getActiveBits() > 64)
                warning(cloc, "integer constant expression larger exceeds 64 bit, the result is truncated");
            return CI->getValue().getLimitedValue();
        }
        type_error(cloc, "not a integer constant: %E", e);
        return 0;
    }
    uint64_t try_eval(Expr e, location_t cloc, bool &ok) {
        if (e->k == EConstant) {
            if (const auto CI = dyn_cast<ConstantInt>(e->C)) {
                if (CI->getValue().getActiveBits() > 64)
                    warning(cloc, "integer constant expression larger exceeds 64 bit, the result is truncated");
                ok = true;
                return CI->getValue().getLimitedValue();
            }
        }
        ok = false;
        return 0;
    }
    bool try_eval_as_bool(Expr e, bool &res) {
        if (e->k == EConstant) {
            if (const auto CI = dyn_cast<ConstantInt>(e->C)) {
                res = !CI->isZero();
                return true;
            }
        }
        return false;
    }
};
