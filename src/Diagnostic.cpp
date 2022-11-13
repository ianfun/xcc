enum DiagnosticLevel {
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

    const char *fmt;
    llvm::SmallVector<storage_type, 5> data;
    Location loc;

    Diagnostic(const char *fmt, Location loc = Location()) : fmt{fmt}, data{}, loc{loc} { }
    template <typename T> void write_impl(const T *ptr) { data.push_back(reinterpret_cast<storage_type>(ptr)); }
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
};
struct DiagnosticConsumer {
    DiagnosticConsumer(const DiagnosticConsumer&) = delete;
    unsigned NumWarnings = 0;
    unsigned NumErrors = 0;
    unsigned getNumErrors() const { return NumErrors; }
    unsigned getNumWarnings() const { return NumWarnings; }
    void clear() { NumWarnings = NumErrors = 0; }
    typedef void (*PHandleDiagnosticTy)(void *self, enum DiagnosticLevel level, const Diagnostic &Info);
    typedef void (*PFinalizeTy)(void *self);
    PHandleDiagnosticTy PHandleDiagnostic;
    PFinalizeTy PFinalize;
    void finalize() {
        if (PFinalize)
            PFinalize(this);
    }
    void HandleDiagnostic(enum DiagnosticLevel level, const Diagnostic &Info) {
        if (level == Warning)
            ++NumWarnings;
        else if (level >= Error)
            ++NumErrors;
        PHandleDiagnostic(this, level, Info);
    }
    DiagnosticConsumer(PHandleDiagnosticTy impl, PFinalizeTy f = nullptr) : PHandleDiagnostic{impl}, PFinalize{f} {};
    void emitDiagnostic(const char *msg, enum DiagnosticLevel level) {
        Diagnostic Diag(msg);
        HandleDiagnostic(level, Diag);
    }
    void emitDiagnostic(Location loc, const char *msg, enum DiagnosticLevel level) {
        Diagnostic Diag(msg, loc);
        HandleDiagnostic(level, Diag);
    }
    template <typename... Args> void emitDiagnostic(const char *msg, enum DiagnosticLevel level, const Args &...args) {
        Diagnostic Diag(msg);
        Diag.write(args...);
        HandleDiagnostic(level, Diag);
    }
    template <typename... Args>
    void emitDiagnostic(Location loc, const char *msg, enum DiagnosticLevel level, const Args &...args) {
        Diagnostic Diag(msg, loc);
        Diag.write(args...);
        HandleDiagnostic(level, Diag);
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
        : DiagnosticConsumer{&HandleDiagnosticImpl, &finalizeImpl}, OS{OS}, ShowColors{OS.has_colors()}, SM{SM} { }
    void realHandleDiagnostic(enum DiagnosticLevel level, const Diagnostic &Info);
    static void HandleDiagnosticImpl(void *self, enum DiagnosticLevel level, const Diagnostic &Info) {
        return reinterpret_cast<TextDiagnosticPrinter *>(self)->realHandleDiagnostic(level, Info);
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
    void printSource(LocationBase loc);
};
// A helper class to emit Diagnostics
// such as parse_error(), lex_error(), warning() functions
struct DiagnosticHelper {
    DiagnosticConsumer &printer;
    DiagnosticHelper(DiagnosticConsumer &consumer): printer{consumer} {}
    unsigned getNumErrors() {
        return printer.getNumErrors();
    }
    unsigned getNumWarnings() {
        return printer.getNumWarnings();
    }
#define DIAGNOSTIC_HANDLER(HANDLER, LEVEL) \
    template <typename... Args> void HANDLER(const char *msg, const Args &...args) { \
        printer.emitDiagnostic(msg, DiagnosticLevel::LEVEL, args...); \
    } \
    template <typename... Args> void HANDLER(Location loc, const char *msg, const Args &...args) { \
        printer.emitDiagnostic(loc, msg, DiagnosticLevel::LEVEL, args...); \
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
    void expect(Location loc, const char *item) { parse_error(loc, "%s expected", item); }
    void expectExpression(Location loc) { parse_error(loc, "%s", "expected expression"); }
    void expectStatement(Location loc) { parse_error(loc, "%s", "expected statement"); }
    void expectLB(Location loc) { parse_error(loc, "%s", "missing " lquote "(" rquote); }
    void expectRB(Location loc) { parse_error(loc, "%s", "missing " lquote ")" rquote); }
};
