// C++'s Varargs cannot pass non-trivial objects, so we write our argument passing system.
// There are two ways to add argument to format string:
// 1. function call: like printf
// 2. Use the '<<' operator.
//    Like llvm::raw_ostream and clang::StreamingDiagnostic(and std::cout)

enum DiagnosticLevel : uint8_t {
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
    using storage_type = std::conditional<sizeof(uintptr_t) >= sizeof(uint64_t), uintptr_t, uint64_t>::type;

    const char *fmt = nullptr;
    location_t loc = 0;
    enum DiagnosticLevel level = Ignored;
    SmallVector<SourceRange, 2> ranges;
    SmallVector<storage_type, 3> data;
    SmallVector<FixItHint, 0> FixItHints;

    // default constructor - construct a invalid Diagnostic

    Diagnostic() = default;

    Diagnostic(const char *fmt, location_t loc = 0) : fmt{fmt}, loc{loc}, level{Ignored}, ranges{}, data{}, FixItHints{} { }

    // deep copy a Diagnostic object
    Diagnostic(const Diagnostic &) = default;

    void write_impl(const FixItHint &Hint) { FixItHints.push_back(Hint); }
    template <typename T> void write_impl(const T *ptr) { data.push_back(reinterpret_cast<storage_type>(ptr)); }
    void write_impl(SourceRange range) { ranges.push_back(range); }
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
    void reset(const char *fmt, enum DiagnosticLevel level, location_t loc = 0) {
        this->data.clear();
        this->ranges.clear();
        this->FixItHints.clear();
        this->loc = loc;
        this->level = level;
        this->fmt = fmt;
    }
};
struct DiagnosticConsumer {
    DiagnosticConsumer(const DiagnosticConsumer &) = delete;
    DiagnosticConsumer(): NumWarnings{0}, NumErrors{0} {}
    unsigned NumWarnings;
    unsigned NumErrors;
    unsigned getNumErrors() const { return NumErrors; }
    unsigned getNumWarnings() const { return NumWarnings; }
    virtual void clear() { NumWarnings = NumErrors = 0; }
    virtual void finalize() {}
    virtual void HandleDiagnostic(const Diagnostic &Diag) {
        if (Diag.level == Warning)
            ++NumWarnings;
        else if (Diag.level >= Error)
            ++NumErrors;
    }
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
        :OS{OS}, ShowColors{OS.has_colors()}, SM{SM} { }
    void HandleDiagnostic(const Diagnostic &Info) override;
    bool hasSourceMgr() const { return SM != nullptr; }
    void setSourceMgr(struct SourceMgr *SM) { this->SM = SM; }
    virtual void finalize() override {
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
    void printSource(source_location &loc, const ArrayRef<FixItHint> FixItHints);
    void write_loc(const source_location &loc, const struct IncludeFile *file);
};
struct TextDiagnosticBuffer : public DiagnosticConsumer {
    using DiagList = std::vector<std::pair<location_t, std::string>>;
    using iterator = DiagList::iterator;
    using const_iterator = DiagList::const_iterator;

    DiagList Errors, Warnings, Remarks, Notes;

    SmallVector<std::pair<enum DiagnosticLevel, size_t>> All;

    const_iterator err_begin() const { return Errors.begin(); }
    const_iterator err_end() const { return Errors.end(); }
    
    const_iterator warn_begin() const { return Warnings.begin(); }
    const_iterator warn_end() const { return Warnings.end(); }
    
    const_iterator remark_begin() const { return Remarks.begin(); }
    const_iterator remark_end() const { return Remarks.end(); }
    
    const_iterator note_begin() const { return Notes.begin(); }
    const_iterator note_end() const { return Notes.end(); }

    auto all_begin() const { return All.begin(); }
    auto all_end() const { return All.end(); }
   
    virtual void clear() override {
        DiagnosticConsumer::clear();
        Notes.clear();
        All.clear();
        Remarks.clear();
        Warnings.clear();
        Errors.clear();
    }
    virtual void HandleDiagnostic(const Diagnostic &Info) override {
        DiagnosticConsumer::HandleDiagnostic(Info);
        SmallString<100> Buf;
        switch (Info.level) {
            default:
            case Ignored: llvm_unreachable("Diagnostic not handled during diagnostic buffering!");
            case Note:
                All.emplace_back(Info.level, Notes.size());
                Notes.emplace_back(Info.loc, std::string(Buf.str()));
                break;
            case Remark:
                All.emplace_back(Info.level, Remarks.size());
                Remarks.emplace_back(Info.loc, std::string(Buf.str()));
                break;
            case Warning:
                All.emplace_back(Info.level, Warnings.size());
                Warnings.emplace_back(Info.loc, std::string(Buf.str()));
                break;
            case Error:
            case PPError:
            case LexError:
            case ParseError:
            case EvalError:
            case TypeError:
            case Fatal:
                All.emplace_back(Info.level, Errors.size());
                Errors.emplace_back(Info.loc, std::string(Buf.str()));
                break;
        }
    }
    virtual void FlushDiagnostics(struct DiagnosticsEngine&) const;
    TextDiagnosticBuffer() {}
};
struct DiagnosticsStore: public DiagnosticConsumer {
    std::vector<Diagnostic> diagnostics;
    virtual void HandleDiagnostic(const Diagnostic &Info) override {
        DiagnosticConsumer::HandleDiagnostic(Info);
        diagnostics.push_back(Info);
    }
    virtual void clear() override {
        DiagnosticConsumer::clear();
        diagnostics.clear();
    }
    void FlushDiagnostics(struct DiagnosticsEngine &) const;
    void FlushDiagnostics(DiagnosticConsumer &) const;
};
struct DiagnosticsEngine {
    unsigned ErrorLimit = 0;
    Diagnostic CurrentDiagnostic;
    SmallVector<DiagnosticConsumer *, 2> consumers;
    void addConsumer(DiagnosticConsumer *C) { consumers.push_back(C); }
    unsigned getNumConsumers() { return consumers.size(); }
    DiagnosticConsumer *getFirstConsumer() { return consumers.front(); }
    DiagnosticConsumer *getLastConsumer() { return consumers.back(); }
    void Emit(const Diagnostic &Diag) {
        for (const auto C : consumers)
            C->HandleDiagnostic(Diag);
    }
    void EmitCurrentDiagnostic() {
        return Emit(CurrentDiagnostic);
    }
    void operator <<(const Diagnostic &Diag) {
        Emit(Diag);
    }
    unsigned getNumWarnings() const {
        unsigned total = 0;
        for (const DiagnosticConsumer *C : consumers)
            total += C->getNumWarnings();
        return total;
    }
    unsigned getNumErrors() const {
        unsigned total = 0;
        for (const DiagnosticConsumer *C : consumers)
            total += C->getNumErrors();
        return total;
    }
    void reset() {
        for (DiagnosticConsumer *C : consumers)
            C->clear();
    }
};
struct DiagnosticBuilder {
    DiagnosticsEngine &engine;
    bool Emited;
    DiagnosticBuilder(const DiagnosticBuilder &other) : engine{other.engine}, Emited{other.Emited} {};
    DiagnosticBuilder(DiagnosticsEngine &engine) : engine{engine}, Emited{false} { }
    ~DiagnosticBuilder() {
        if (!Emited)
            engine.EmitCurrentDiagnostic();
    }
    void Emit() {
        if (!Emited) {
            Emited = true;
            engine.EmitCurrentDiagnostic();
        }
    }
    template <typename T> const DiagnosticBuilder &operator<<(const T &V) const {
        engine.CurrentDiagnostic.write_impl(V);
        return *this;
    }
    void setLoc(location_t loc) {
        engine.CurrentDiagnostic.loc = loc;
    }
};
// A helper class to emit Diagnostics
// such as parse_error(), lex_error(), warning() functions
struct DiagnosticHelper {
    DiagnosticsEngine &engine;
    DiagnosticHelper(DiagnosticsEngine &engine) : engine{engine} { }
    DiagnosticHelper(const DiagnosticHelper &other): engine{other.engine} {}
    unsigned getNumErrors() const { return engine.getNumErrors(); }
    unsigned getNumWarnings() const { return engine.getNumWarnings(); }
#define DIAGNOSTIC_HANDLER(HANDLER, LEVEL)                                                                             \
    template <typename... Args> [[maybe_unused]] DiagnosticBuilder HANDLER(const char *msg, const Args &...args) {     \
        engine.CurrentDiagnostic.reset(msg, LEVEL);                                                                    \
        engine.CurrentDiagnostic.write(args...);                                                                       \
        return DiagnosticBuilder(engine);                                                                              \
    }                                                                                                                  \
    [[maybe_unused]] DiagnosticBuilder HANDLER(const char *msg) {                                                      \
        engine.CurrentDiagnostic.reset(msg, LEVEL);                                                                    \
        return DiagnosticBuilder(engine);                                                                              \
    }                                                                                                                  \
    template <typename... Args>                                                                                        \
    [[maybe_unused]] DiagnosticBuilder HANDLER(location_t loc, const char *msg, const Args &...args) {                 \
        engine.CurrentDiagnostic.reset(msg, LEVEL, loc);                                                               \
        engine.CurrentDiagnostic.write(args...);                                                                       \
        return DiagnosticBuilder(engine);                                                                              \
    }                                                                                                                  \
    template <typename... Args> [[maybe_unused]] DiagnosticBuilder HANDLER(location_t loc, const char *msg) {          \
        engine.CurrentDiagnostic.reset(msg, LEVEL, loc);                                                               \
        return DiagnosticBuilder(engine);                                                                              \
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
    void expect(location_t loc, const char *item) { parse_error(loc, "%s expected", item); }
    void expectExpression(location_t loc) { parse_error(loc, "%s", "expected expression"); }
    void expectStatement(location_t loc) { parse_error(loc, "%s", "expected statement"); }
    void expectLB(location_t loc) { parse_error(loc, "%s", "missing " lquote "(" rquote); }
    void expectRB(location_t loc) { parse_error(loc, "%s", "missing " lquote ")" rquote); }
};
struct EvalHelper : public DiagnosticHelper {
    EvalHelper(DiagnosticsEngine &engine) : DiagnosticHelper{engine} { }
    EvalHelper(const DiagnosticHelper &other): DiagnosticHelper{other} {}
    [[nodiscard]] uint64_t force_eval(Expr e) {
        if (e->k != EConstant) {
            type_error(e->getBeginLoc(), "not a constant expression: %E", e) << e->getSourceRange();
            return 0;
        }
        if (const auto CI = dyn_cast<ConstantInt>(e->C)) {
            if (CI->getValue().getActiveBits() > 64)
                warning(e->getBeginLoc(), "integer constant expression larger exceeds 64 bit, the result is truncated")
                    << e->getSourceRange();
            return CI->getValue().getLimitedValue();
        }
        type_error(e->getBeginLoc(), "expect integer constant expression") << e->getSourceRange();
        return 0;
    }
    [[nodiscard]] uint64_t try_eval(Expr e, bool &ok) {
        if (e->k == EConstant) {
            if (const auto CI = dyn_cast<ConstantInt>(e->C)) {
                if (CI->getValue().getActiveBits() > 64)
                    warning(e->getBeginLoc(),
                            "integer constant expression larger exceeds 64 bit, the result is truncated")
                        << e->getSourceRange();
                ok = true;
                return CI->getValue().getLimitedValue();
            }
        }
        ok = false;
        return 0;
    }
    [[nodiscard]] bool try_eval_as_bool(Expr e, bool &res) {
        if (e->k == EConstant) {
            if (const auto CI = dyn_cast<ConstantInt>(e->C)) {
                res = !CI->isZero();
                return true;
            }
        }
        return false;
    }
};
void DiagnosticsStore::FlushDiagnostics(struct DiagnosticsEngine &engine) const {
    for (const Diagnostic &Diag: diagnostics)
        engine << Diag;
}
void DiagnosticsStore::FlushDiagnostics(DiagnosticConsumer &C) const {
    for (const Diagnostic &Diag: diagnostics) {
        printf("flush %p\n", &Diag);
        C.HandleDiagnostic(Diag);
    }
}
void
TextDiagnosticBuffer::FlushDiagnostics(struct DiagnosticsEngine &engine) const {
    for (const auto &I: All) {
        engine.CurrentDiagnostic.reset("%R", I.first);
        DiagnosticBuilder Diag = DiagnosticBuilder(engine);
        switch (I.second) {
            default:
                llvm_unreachable("Diagnostic not handled during diagnostic flushing!");
            case Note:
                Diag << Notes[I.second].second;
                Diag.setLoc(Notes[I.second].first);
                break;
            case Remark:
                Diag << Remarks[I.second].second;
                Diag.setLoc(Remarks[I.second].first);
                break;
            case Warning:
                Diag << Warnings[I.second].second;
                Diag.setLoc(Warnings[I.second].first);
                break;
            case Error:
            case PPError:
            case LexError:
            case ParseError:
            case EvalError:
            case TypeError:
            case Fatal:
                Diag << Errors[I.second].second;
                Diag.setLoc(Errors[I.second].first);
                break;
        }
    }
}
