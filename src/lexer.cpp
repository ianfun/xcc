/* lexer.cpp - C's lexer and preprocessor */

struct Parser;

struct ScratchBuffer {
    SourceMgr &SM;
    static constexpr unsigned ScratchBufSize = 4060;
    IncludeFile *curFile = nullptr;
    unsigned BytesUsed = ScratchBufSize;
    char *CurBuffer = nullptr;
    ScratchBuffer(SourceMgr &SM): SM{SM} {}
    TokenV getToken(const char *Buf, unsigned Len, Token kind = TStringLit) {
        TokenV theTok = kind;
        if (BytesUsed+Len+1 > ScratchBufSize)
            AllocScratchBuffer(Len+1);
        memcpy((void*)(theTok.str = CurBuffer+BytesUsed), Buf, Len);
        location_t newLoc = curFile->getStartLoc() + BytesUsed;
        BytesUsed += Len;
        CurBuffer[BytesUsed++] = '\n';
        theTok.setLoc(newLoc);
        theTok.setLength(Len);
        return theTok;
    }
    TokenV getToken(StringRef str, Token kind = TStringLit) {
        return getToken(str.data(), str.size(), kind);
    }
    void AllocScratchBuffer(unsigned RequestLen) {
        if (RequestLen < ScratchBufSize)
            RequestLen = ScratchBufSize;
        llvm::WritableMemoryBuffer *newBuffer = llvm::WritableMemoryBuffer::getNewMemBuffer(RequestLen, "<scratch space>").release();
        CurBuffer = const_cast<char*>(newBuffer->getBufferStart());
        curFile = SM.addScratchBuffer(newBuffer);
        BytesUsed = 0;
    }
};

struct Lexer : public EvalHelper {
    enum PPFlags : uint8_t {
        PFNormal = 1,
        PFPP = 2
    };
    TokenV tok = TNul;
    bool want_expr = false;
    bool isPPMode = false;
    bool isDisableSpace = false;
    bool lexRawEnabled = false;
    llvm::SmallVector<uint8_t, 4> ppstack;
    bool ok = true;
    char c;
    static const char months[12][4];
    DenseMap<IdentRef, PPMacroDef *> macros;
    Parser *parser;
    SourceMgr &SM;
    std::deque<TokenV> tokenq;
    std::vector<PPMacroDef *> expansion_list;
    xstring lexIdnetBuffer = xstring::get_with_capacity(20);
    uint32_t counter = 0;
    location_t loc = 0, endLoc = 0;
    xcc_context &context;
    ScratchBuffer ScratchBuf;

    ~Lexer() {
        lexIdnetBuffer.free();
    }
    Lexer(SourceMgr &SM, xcc_context &context, Parser *parser = nullptr)
        : EvalHelper{SM}, parser{parser}, SM{SM}, context{context}, ScratchBuf{SM} { initC(); }
    void initC() { c = ' '; }
    location_t getLoc() const { return loc; }
    location_t getEndLoc() const { return endLoc; }
    Expr constant_expression();
    void updateLoc() {
        endLoc = loc = SM.getLoc() - 1;
    }
    static bool isCSkip(char c) {
        // space, tab, new line, form feed are translate into ' '
        return c == ' ' || c == '\t' || c == '\f' || c == '\v';
    }
    void validUCN(Codepoint codepoint, location_t loc) {
        if (codepoint < 0xA0) {
            if (codepoint == 0x24 || codepoint == 0x40 || codepoint == 0x60)
                return;
            lex_error(loc, "%U is not a valid universal character", codepoint);
        }
        if (codepoint > 0x10FFFF)
            lex_error(loc, "codepoint too large(larger than 0x10FFFF)"), codepoint = 0x10FFFF - 1;
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
            lex_error(loc, "universal character %U in surrogate range", codepoint);
    }
    inline void eat() { c = SM.skip_read(); }
    Codepoint lexHexChar(const unsigned char * &s) {
        Codepoint n = 0;
        for (;;) {
            Codepoint t = *s++;
            if (llvm::isDigit(t))
                n = (n << 4) | (t - '0' + 0);
            else if (t >= 'a' && t <= 'f')
                n = (n << 4) | (t - 'a' + 10);
            else if (t >= 'A' && t <= 'F')
                n = (n << 4) | (t - 'A' + 10);
            else
                return n;
        }
    }
    Codepoint lexHexChar() {
        Codepoint n = 0;
        for (;;) {
            Codepoint t = (unsigned char)c;
            if (llvm::isDigit(t))
                n = (n << 4) | (t - '0' + 0);
            else if (t >= 'a' && t <= 'f')
                n = (n << 4) | (t - 'a' + 10);
            else if (t >= 'A' && t <= 'F')
                n = (n << 4) | (t - 'A' + 10);
            else
                return n;
            eat();
        }
    }
    void consumeHexChar() {
        while (llvm::isHexDigit(c))
            eat();
    }
    void consumeUChar(unsigned count) {
        unsigned i = 0;
        do {
            if (!llvm::isHexDigit(c))
                lex_error(getLoc(), "incomplete universal character name (expect %u hex digits)", count);
            eat();
        } while (i < count);
    }
    Codepoint lexUChar(unsigned count, const unsigned char * &s, location_t TheLoc) {
        unsigned i = 0;
        Codepoint n = 0;
        do {
            Codepoint t = *s++;
            i++;
            if (llvm::isDigit(t))
                n = (n << 4) | (t - '0');
            else if (t >= 'a' && t <= 'f')
                n = (n << 4) | (t - 'a' + 10);
            else
                n = (n << 4) | (t - 'A' + 10);
        } while (i < count);
        validUCN(n, TheLoc);
        return n;
    }
    Codepoint lexUChar(unsigned count) {
        unsigned i = 0;
        Codepoint n = 0;
        do {
            Codepoint t = static_cast<unsigned char>(c);
            i++;
            if (llvm::isDigit(t))
                n = (n << 4) | (t - '0');
            else if (t >= 'a' && t <= 'f')
                n = (n << 4) | (t - 'a' + 10);
            else
                n = (n << 4) | (t - 'A' + 10);
            eat();
        } while (i < count);
        validUCN(n, getLoc());
        return n;
    }
    Codepoint lexEscape(const unsigned char *&s, location_t TheLoc) {
        switch (*s++) {
        case 'a': return 7;
        case 'b': return 8;
        case 'f': return 12;
        case 'n': return 10;
        case 'r': return 13;
        case 't': return 9;
        case 'v': return 11;
        case 'e':
        case 'E': return 27;
        case 'x': return lexHexChar(s);
        case 'u': return lexUChar(4, s, TheLoc);
        case 'U': return lexUChar(8, s, TheLoc);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            Codepoint n = Codepoint(s[-1]) - '0';
            if (*s >= '0' && *s <= '7') {
                n = (n << 3) | (Codepoint(*s) - '0');
                s++;
                if (*s >= '0' && *s <= '7') {
                    n = (n << 3) | (Codepoint(*s) - '0');
                    s++;
                }
            }
            return n;
        }
        default: {
            return s[-1];
        }
        }
    }
    Codepoint lexEscape() {
        eat();
        switch (c) {
        case 'a': return eat(), 7;
        case 'b': return eat(), 8;
        case 'f': return eat(), 12;
        case 'n': return eat(), 10;
        case 'r': return eat(), 13;
        case 't': return eat(), 9;
        case 'v': return eat(), 11;
        case 'e':
        case 'E': return eat(), 27;
        case 'x': return lexHexChar();
        case 'u': return lexUChar(4);
        case 'U': return lexUChar(8);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            unsigned char t = c;
            Codepoint n = Codepoint(t) - '0';
            eat();
            t = c;
            if (t >= '0' && t <= '7') {
                n = (n << 3) | (Codepoint(t) - '0');
                eat();
                t = c;
                if (t >= '0' && t <= '7') {
                    n = (n << 3) | (Codepoint(t) - '0');
                    eat();
                }
            }
            return n;
        }
        default: {
            unsigned char t = c;
            eat();
            return t; 
        }
        }
    }
    void consumeEscape() {
        eat();
        switch (c) {
        case 'x': return eat(), consumeHexChar();
        case 'u': return eat(), consumeUChar(4);
        case 'U': return eat(), consumeUChar(8);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': 
        {
            eat();
            while (c >= '0' && c <= '7')
                eat();
            return;
        }
        default: {
            eat();
        }
        }
    }
    TokenV lexCharLit(enum StringPrefix enc = Prefix_none) {
        // C23
        // An integer character constant is a sequence of one or more multibyte characters enclosed in singlequotes, as
        // in ’x’. A UTF-8 character constant is the same, except prefixed by u8. A wchar_t character constant is
        // prefixed by the letter L. A UTF-16 character constant is prefixed by the letter u. A UTF-32 character
        // constant is prefixed by the letter U.
        TokenV theTok = TokenV(TCharLit);
        theTok.itag = enc;
        eat(); // eat '
        if (c == '\\') {
            auto codepoint = lexEscape();
            // C23
            // If an integer character constant contains a single character or escape sequence, its value is the one
            // that results when an object with type char whose value is that of the single character or escape sequence
            // is converted to type int.
            if (codepoint > 0xFF && (enc == Prefix_none || enc == Prefix_u8))
                warning(loc, "character constant exceeds 8 bit");
            else if (codepoint > 0xFFFF && (enc == Prefix_L || enc == Prefix_u)) {
                if (enc == Prefix_L)
                    warning(loc, "A wchar_t character constant exceeds 16 bit is not portable");
                else
                    warning(loc, "UTF-16 character constant exceeds 16 bit");
            }
            // C23
            // In an implementation in which type char has the same range of values as signed char, the integer
            // character constant ’\xFF’ has the value −1; if type char has the same range of values as unsigned char,
            // the character constant ’\xFF’ has the value +255.
            theTok.i = codepoint;
        } else {
            theTok.i = (unsigned char)c;
            eat();
        }
        if (c != '\'')
            warning(loc, "missing terminating " lquote "'" rquote " character in character literal");
        endLoc = SM.getLoc() - 1;
        eat();
        return theTok;
    }
    // copy from utf-8.h
    void cat_codepoint(xstring &str, Codepoint chr) {
        if (0 == (0xffffff80 & chr)) {
            /* 1-byte/7-bit ascii
             * (0b0xxxxxxx) */
            str.push_back(chr);
        } else if (0 == (0xfffff800 & chr)) {
            /* 2-byte/11-bit utf8 code point
             * (0b110xxxxx 0b10xxxxxx) */
            str.push_back(0xc0 | (chr >> 6));
            str.push_back(0x80 | (chr & 0x3f));
        } else if (0 == (0xffff0000 & chr)) {
            /* 3-byte/16-bit utf8 code point
             * (0b1110xxxx 0b10xxxxxx 0b10xxxxxx) */
            str.push_back(0xe0 | (chr >> 12));
            str.push_back(0x80 | ((chr >> 6) & 0x3f));
            str.push_back(0x80 | (chr & 0x3f));
        } else { /* if (0 == ((int)0xffe00000 & chr)) { */
            /* 4-byte/21-bit utf8 code point
             * (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx) */
            str.push_back(0xf0 | (chr >> 18));
            str.push_back(0x80 | ((chr >> 12) & 0x3f));
            str.push_back(0x80 | ((chr >> 6) & 0x3f));
            str.push_back(0x80 | (chr & 0x3f));
        }
    }
    void cat_codepoint(SmallVectorImpl<char> &str, Codepoint chr) {
        if (0 == (0xffffff80 & chr)) {
            /* 1-byte/7-bit ascii
             * (0b0xxxxxxx) */
            str.push_back(chr);
        } else if (0 == (0xfffff800 & chr)) {
            /* 2-byte/11-bit utf8 code point
             * (0b110xxxxx 0b10xxxxxx) */
            str.push_back(0xc0 | (chr >> 6));
            str.push_back(0x80 | (chr & 0x3f));
        } else if (0 == (0xffff0000 & chr)) {
            /* 3-byte/16-bit utf8 code point
             * (0b1110xxxx 0b10xxxxxx 0b10xxxxxx) */
            str.push_back(0xe0 | (chr >> 12));
            str.push_back(0x80 | ((chr >> 6) & 0x3f));
            str.push_back(0x80 | (chr & 0x3f));
        } else { /* if (0 == ((int)0xffe00000 & chr)) { */
            /* 4-byte/21-bit utf8 code point
             * (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx) */
            str.push_back(0xf0 | (chr >> 18));
            str.push_back(0x80 | ((chr >> 12) & 0x3f));
            str.push_back(0x80 | ((chr >> 6) & 0x3f));
            str.push_back(0x80 | (chr & 0x3f));
        }
    }
    TokenV lexIdent() {
        TokenV theTok = TokenV(PPIdent);
        for (;;) {
            unsigned n = 8;
            if (c == '\\') {
                eat();
                if (c == 'U') {
R:
                    eat();
                    auto codepoint = lexUChar(n);
                    cat_codepoint(lexIdnetBuffer, codepoint);
                } else if (c == 'u') {
                    n = 4;
                    goto R;
                } else {
                    lex_error(loc, "%s", "stray '\\' in program");
                    eat();
                }
            } else {
                if (!(llvm::isAlnum(c) || c == '_' || (unsigned char)c & 128 || c == '$'))
                    break;

                lexIdnetBuffer.push_back(c);
                eat();
            }
        }
        if (!enc::IsUTF8(lexIdnetBuffer.str()))
            warning(loc, "identfier is not UTF-8 encoded");
        theTok.s = context.table.get(lexIdnetBuffer.str(), PPIdent);
        theTok.tok = std::min(theTok.tok, theTok.s->second);
        lexIdnetBuffer.clear();
        return theTok;
    }
    TokenV lexIdent(StringRef s) {
        lexIdnetBuffer.push_back(s);
        return lexIdent();
    }
    TokenV lexIdent(char c) {
        lexIdnetBuffer.push_back(c);
        return lexIdent();
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // pp-number can store any numbers:
    //     0b1010, 10, 012, 0xA, 3.14, 1UL, 2lf, 10e3 ...
    //
    // 6.4.8 Preprocessing numbers
    //    A preprocessing number does not have type or a value;
    //    it acuires both after sucessful conversion (as part of translation phase 7)
    //    to a floating constant token or integer constant token
    //
    // pp-number:
    //             digit
    //             .digit
    //             pp-number digit
    //             pp-number idnetfier-digit
    //             pp-number e sign
    //             pp-number E sign
    //             pp-number p sign
    //             pp-number P sign
    //             pp-number .
    //
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //
    // pp-number:
    //             digit[s] pp-number-end
    //             .digit[s] pp-number-end
    // pp-number-end:
    //             none
    //             e sign pp-number-end
    //             E sign pp-number-end
    //             p sign pp-number-end
    //             P sign pp-number-end
    //             digit pp-number-end
    //             idnetfier-digit pp-number-end
    //             . pp-number-end
    //
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    TokenV lexPPNumber(const char *BufferPtr) {
        do {
            eat();
            switch (c) {
            case 'P':
            case 'p':
            case 'E':
            case 'e':
                eat();
                if (c == '+' || c == '-')
                    eat();
                continue;
            }
        } while (llvm::isAlnum(c) || c == '\'' || c == '.');
        endLoc = SM.getLoc() - 2;
        return TokenV(PPNumber, BufferPtr);
    }
    TokenV lexStringLit(const char *BufferPtr) {
        eat();
        for (;c != '\"';) {
            if (c == '\\') {
                consumeEscape();
            } else {
                eat();
            }
        }
        eat();
        endLoc = SM.getLoc() - 2;
        return TokenV(TStringLit, BufferPtr);
    }
    void lexString(SmallVectorImpl<char> &buffer, const TokenV &theTok) {
        const unsigned char *s = reinterpret_cast<const unsigned char*>(theTok.getStringLiteral().data());
        do {} while (*s++ != '\"');
        for (;;) {
            char c = *s++;
            if (c == '"') break;
            if (c == '\\') {
                cat_codepoint(buffer, lexEscape(s, theTok.getLoc()));
            } else {
                buffer.push_back(c);
            }
        }
    }
    void beginExpandMacro(PPMacroDef *M) {
        SM.beginExpandMacro(M, context);
        expansion_list.push_back(M);
        tokenq.push_back(SM.getLocTree());
        tokenq.push_back(TokenV(PPMacroPop));
    }
    bool isMacroInUse(IdentRef Name) const {
        for (const auto &it : expansion_list)
            if (it->getName() == Name)
                return true;
        return false;
    }
    template <unsigned Len>
    void getTokenSpelling(const TokenV &tok, SmallString<Len> &result) {
        switch (tok.tok) {
        case TCharLit:
        {
            const char *BufferPtr = SM.getBufferForLoc(tok.getLoc());
            if (!BufferPtr) {
                formatChar(tok.i, result);
            } else {
                result += StringRef(BufferPtr, tok.getLength());
            }
        } break;
        case TStringLit:
            result += tok.getStringLiteral();
            break;
        case PPNumber:
            result += tok.getPPNumberLit();
            break;
        default:
            if (tok.tok >= kw_start)
                result += tok.s->getKey();
            else 
                result += show(tok.tok);
            break;
        }
    }
    TokenV paste_tokens(const TokenV &Lhs, const TokenV &Rhs) {
        SmallString<32> Str;
        // common case: paste two identfiers
        if (Lhs.tok > kw_start && Rhs.tok > kw_start) {
            TokenV it = PPIdent;
            Str += Lhs.s->getKey();
            Str += Rhs.s->getKey();
            it.s = context.table.get(Str, PPIdent);
            it.tok = std::min(PPIdent, it.s->second);
            it.setLoc(Lhs.getLoc());
            it.setEndLoc(Rhs.getEndLoc());
            return it;
        }
        getTokenSpelling(Lhs, Str);
        getTokenSpelling(Rhs, Str);
        Str.push_back(0);

        unsigned oldSize = SM.active_files();

        char oldC = this->c;
        this->c = ' ';
        isPPMode = false;

        StringRef source_string = StringRef(Str.data(), Str.size() - 1);

        SM.addStringAndCopy(source_string, "<token pasting>");

        TokenV theTok = lexAndLoc();

        if (this->c != 0)
            pp_error(Lhs.loc, "pasting formed %R, an invalid preprocessing token", source_string);

        if (SM.active_files() != oldSize)
            SM.nextFile();

        updateLoc();
        this->c = oldC;
        isPPMode = true;

        return theTok;
    }
public:
    void cpp() {
        if (tokenq.empty())
            tok = lexAndLoc();
        else
            tok = tokenq.back(), tokenq.pop_back();
        if (tok.tok >= kw_start && tok.tok != TIdentifier) {
            isPPMode = true;
            checkMacro();
            isPPMode = false;
            if (tok.tok == TNewLine)
                return cpp();
            if (tok.tok > TIdentifier)
                tok.tok = TIdentifier;
        } else if (tok.tok == PPMacroPop) {
            SM.endTree();
            expansion_list.pop_back();
            return cpp();
        } else if (tok.tok == PPMacroTraceLoc) {
            // loc.setParent(tok.tree);
            return cpp();
        }
    }
#define TOK2(tc, ac, at, bt)                                                                                           \
    case tc:                                                                                                           \
        eat();                                                                                                         \
        if (c == ac)                                                                                                   \
            return eat(), at;                                                                                          \
        return bt;

#define TOK3(tc, ac, at, bc, bt, d)                                                                                    \
    case tc:                                                                                                           \
        eat();                                                                                                         \
        if (c == ac)                                                                                                   \
            return eat(), at;                                                                                          \
        if (c == bc)                                                                                                   \
            return eat(), bt;                                                                                          \
        return d;

#define TOK4(tc, ac, at, bc, bt, cc, ct, d)                                                                            \
    case tc:                                                                                                           \
        eat();                                                                                                         \
        switch (c) {                                                                                                   \
        case ac: return eat(), at;                                                                                     \
        case bc: return eat(), bt;                                                                                     \
        case cc: return eat(), ct;                                                                                     \
        default: return d;                                                                                             \
        }

#define TOK5(tc, ac, at, bc, bt, cc, ct, dc, dt, d)                                                                    \
    case tc:                                                                                                           \
        eat();                                                                                                         \
        switch (c) {                                                                                                   \
        case ac: return eat(), at;                                                                                     \
        case bc: return eat(), bt;                                                                                     \
        case cc: return eat(), ct;                                                                                     \
        case dc: return eat(), dt;                                                                                     \
        default: return d;                                                                                             \
        }
    // lex a token and update the location
    inline TokenV lexAndLoc() {
        TokenV it = lex();
        it.setLoc(loc);
        it.setEndLoc(endLoc);
        return it;
    }
    void setLexRawMode(bool enable) {
        lexRawEnabled = enable;
    }
    bool isLexRawEnabled() const {
        return lexRawEnabled;
    }
    void startLex() {
        eat();
    }
    // lex and retrun a token without expanding macros
    TokenV lex() {
        for (;;) {
            if (c == '\0' || c == 26) {
                if (!SM.isFileEOF()) {
                    warning(loc, "null character ignored");
                    eat();
                    continue;
                }
                if (SM.nextFile())
                    return isPPMode = false, TEOF;
                eat();
                continue;
            }
            updateLoc();
            if (c == '#') {
                eat();
                goto RUN;
BAD_RET:
                return isPPMode = false, lex();
RUN:
                if (isPPMode || isLexRawEnabled()) {
                    if (c == '#')
                        return eat(), PPSharpSharp;
                    return PPSharp;
                }
                isPPMode = true;
                tok = lex(); // directive
                if (tok.tok == TSpace)
                    tok = lex();
                if (tok.tok < kw_start) {
                    if (tok.tok != TNewLine)
                        pp_error(loc, "%s", "expect identifier or null directive(new line)");
                    goto BAD_RET;
                }
                IdentRef saved_name = tok.s;
                Token saved_tok = saved_name->second;
                if (saved_tok != PPinclude) {
                    tok = lex();
                    if (tok.tok == TSpace)
                        tok = lex();
                }
                switch (saved_tok) {
                case PPdefine: {
                    if (tok.tok < kw_start) {
                        pp_error(loc, "%s", "expect identifier");
                        goto BAD_RET;
                    }
                    PPMacroDef *theMacro = new (context.getAllocator())PPMacroDef(tok.s, SM.getLoc());
                    if (tok.tok >= PP__LINE__ && tok.tok <= PP_Pragma)
                        warning(loc, "redefining builtin macro %I", theMacro->getName());
                    tok = lex();
                    if (tok.tok == TLbracket)
                        theMacro->setFunctionMacro();
                    while (tok.tok == TSpace)
                        tok = lex();
                    if (theMacro->isFunction()) {
                        isDisableSpace = true;
                        for (;;) {
                            tok = lex();
                            if (tok.tok >= kw_start) {
                                theMacro->params.push_back(tok.s);
                                tok = lex();
                                if (tok.tok == TRbracket)
                                    break;
                                if (tok.tok == TComma)
                                    continue;
                                parse_error(loc, "%s", "')' or ',' expected");
                                goto BAD_RET;
                            }
                            if (tok.tok == TEllipsis) {
                                tok = lex();
                                if (tok.tok != TEllipsis) {
                                    parse_error(loc, "%s", "'.' expected");
                                    goto BAD_RET;
                                }
                                tok = lex();
                                if (tok.tok != TEllipsis) {
                                    parse_error(loc, "%s", "'.' expected");
                                    goto BAD_RET;
                                }
                                theMacro->setVarArgs();
                                tok = lex();
                                if (tok.tok != TRbracket) {
                                    parse_error(loc, "%s", "')' expected");
                                    goto BAD_RET;
                                }
                                continue;
                            }
                            if (tok.s->second == PP__VA_ARGS__) {
                                theMacro->setVarArgs();
                                tok = lex();
                                if (tok.tok != TRbracket) {
                                    parse_error(loc, "%s", "')' expected");
                                    goto BAD_RET;
                                }
                                break;
                            }
                            if (tok.tok == TRbracket)
                                break;
                            else
                                pp_error(loc, "unexpected token: %C", c);
                        }
                        tok = lex(); // eat ')'
                        isDisableSpace = false;
                    }
                    tok.setLoc(loc);
                    tok.setEndLoc(endLoc);
                    while (isPPMode) {
                        theMacro->tokens.push_back(tok);
                        tok = lexAndLoc();
                    }
                    if (theMacro->tokens.size())
                        while (theMacro->tokens.back().tok == TSpace)
                            theMacro->tokens.pop_back();
                    bool ok = true;
                    if (theMacro->tokens.size()) {
                        if (theMacro->tokens.front().tok == PPSharpSharp)
                            parse_error(loc, "%s", "'##' cannot appear at start of macro expansion"), ok = false;
                        else if (theMacro->tokens.back().tok == PPSharpSharp)
                            parse_error(loc, "'##' cannot appear at end of macro expansion"), ok = false;
                    }
                    if (ok) {
                        IdentRef Name = theMacro->getName();
                        auto it = macros.insert({Name, nullptr});
                        if (!it.second) {
                            if (!theMacro->equals(*it.first->second))
                                warning(loc, "macro %I redefined", Name);
                        }
                        it.first->second = theMacro;
                    }
                } break;
                case Kif: {
                    Expr e;

                    want_expr = true;
                    e = constant_expression();
                    want_expr = false;
                    ok = force_eval(e);
                    dbgprint("#if: %s\n", ok ? "true" : "false");
                    ppstack.push_back(ok);
                    (void)e;
                } break;
                case PPifdef:
                case PPifndef: {
                    if (tok.tok < kw_start) {
                        isPPMode = false;
                        pp_error(loc, "expect identifier");
                        goto BAD_RET;
                    }
                    ok = macros.find(tok.s) != macros.end();
                    if (saved_tok == PPundef)
                        ok = !ok;
#if CC_DEBUG
                    if (saved_tok == PPundef)
                        dbgprint("#ifndef: %s\n", ok ? "true" : "false");
                    else
                        dbgprint("#ifdef: %s\n", ok ? "true" : "false");
#endif
                    ppstack.push_back(ok ? 1 : 0);
                } break;
                case Kelse: {
                    if (ppstack.empty()) {
                        pp_error(loc, "no matching #if");
                        goto BAD_RET;
                    }
                    if (ppstack.back() & 2) {
                        pp_error(loc, "#else after #else");
                        goto BAD_RET;
                    }
                    auto &ref = ppstack.back();
                    ref |= 2;
                    ok = !ok;
                    dbgprint("#else: %s\n", ok ? "true" : "false");
                } break;
                case PPelif: {
                    if (ppstack.empty()) {
                        pp_error(loc, "%s", "no matching #if");
                        break;
                    }
                    if (ppstack.back() & 2) {
                        pp_error(loc, "%s", "#elif after #else");
                        break;
                    }
                    if (!ok) {
                        Expr e;
                        want_expr = true;
                        e = constant_expression();
                        want_expr = false;
                        ok = false;
                        if (!e)
                            pp_error(loc, "%s", "expect constant_expression");
                        else {
                            ok = force_eval(e);
                            dbgprint("#if: %s\n", ok ? "true" : "false");
                        }
                    } else
                        ok = false;
                } break;
                case PPendif: {
                    if (ppstack.empty()) {
                        pp_error(loc, "%s", "no matching #if");
                        break;
                    }
                    ppstack.pop_back();
                    if (ppstack.empty())
                        ok = true;
                    else
                        ok = (ppstack.back() & 1);
                    dbgprint("#endif: reset to %s\n", ok ? "true" : "false");
                } break;
                case PPinclude: {
                    while (c == ' ')
                        eat();
                    char is_std = '\"';
                    xstring path = xstring::get();

                    switch (c) {
                    case '"':
STD_INCLUDE:
                        for (;;) {
                            eat();
                            if (c == is_std) {
                                for (;c != '\0' && c != '\n';) {
                                    eat();
                                }
                                if (path.empty()) {
                                    pp_error("%s", "empty filename in #include");
                                } else {
                                    path.make_eos();
                                    SM.addIncludeFile(path, is_std == '>', loc);
                                }
                                path.free();
                                break;
                            }
                            if (c == '\0' || c == '\n') {
                                pp_error(loc, "%s", "expect \"FILENAME\" or <FILENAME>");
                                goto BAD_RET;
                            }
                            path.push_back(c);
                        }
                        eat();
                        isPPMode = false;
                        continue;
                    case '<': is_std = '>'; goto STD_INCLUDE;
                    default: 
                        pp_error(loc, "%s", "expect \"FILENAME\" or <FILENAME>");
                        path.free();
                        goto BAD_RET;
                    }
                } break;
                case PPline: {
                    uint32_t line = 0;
                    if (!llvm::isDigit(c)) {
                        pp_error(loc, "%s", "expect digits (positive line number)");
                        break;
                    }
                    do
                        line = line * 10 + (unsigned char)c;
                    while (eat(), llvm::isDigit(c));
                    SM.setLine(line);
                    while (isCSkip(c))
                        eat();
                    if (c == '"') {
                        xstring str = xstring::get(SM.getFileName());
                        for (;;) {
                            eat();
                            if (c == '\n' || c == '"' || c == '\\' || c == '\0')
                                break;
                            str.push_back(c);
                        }
                        str.make_eos();
                        SM.setFileName(str.data());
                        // the string will not freed!
                        if (c != '"') {
                            pp_error(loc, "%s", "'\"' expected");
                            break;
                        }
                        eat();
                    } else {
                        if (c != '\n' && c != '\0') {
                            pp_error(loc, "%s", "expect \"FILENAME\"");
                            goto BAD_RET;
                        }
                    }
                } break;
                case PPundef: {
                    if (tok.tok < kw_start)
                        pp_error(loc, "%s", "macro name should be a identifier");
                    else {
#if CC_DEBUG
                        macros.erase(tok.s);
#else
                        dbgprint("#undef: ", macros.erase(tok.s) ? "found" : "not found");
#endif
                    }
                } break;
                case PPpragma: {
                    llvm::SmallVector<TokenV> pragmas;
                    while (isPPMode) {
                        if (tok.tok != TSpace)
                            pragmas.push_back(tok);
                        tok = lex();
                    }
                    // TODO: pragma
                } break;
                case PPerror:
                case PPwarning: {
                    SmallString<128> s;
                    for (; c != '\n' && c != '\0';) {
                        s.push_back(c), eat();
                    }
                    StringRef str(s.data(), s.size());
                    if (saved_tok == PPwarning)
                        warning(loc, "#warning: %r", str);
                    else
                        pp_error(loc, "#error: %r", str);
                } break;
                default: pp_error(loc, "invalid preprocessing directive: %I", saved_name);
                }
                while (tok.tok != TNewLine && tok.tok != TEOF)
                    tok = lex();
                isPPMode = false;
                continue;
            }
            if (!isPPMode && !ok) {
                char lastc = c;
                for (;;) {
                    if (c == '\0')
                        return isPPMode = false, TEOF;
                    lastc = c;
                    eat();
                    if (lastc == '\n' && c == '#')
                        goto RUN;
                    if (lastc == '%') {
                        eat();
                        if (c == ':') {
                            eat();
                            goto RUN;
                        }
                    }
                }
            }
            switch (c) {
            case ' ':
            case '\t':
            case '\v':
            case '\f':
                for (;;) {
                    eat();
                    if (!isCSkip(c))
                        break;
                }
                if (isPPMode) {
                    if (want_expr || isDisableSpace)
                        continue;
                    return TSpace;
                }
                continue;
            case '\n':
                if (isPPMode) {
                    return isPPMode = false, TNewLine;
                }
                eat();
                continue;
            case 'u':
                eat();
                if (c == '"')
                    return lexStringLit(SM.getPrevLexBufferPtr() - 1);
                if (c == '\'')
                    return lexCharLit(Prefix_u);
                if (c == '8') {
                    eat();
                    if (c != '"') {
                        if (c == '\'')
                            return lexCharLit(Prefix_u8);
                        return lexIdent("u8");
                    }
                    return lexStringLit(SM.getPrevLexBufferPtr() - 2);
                }
                return lexIdent("u");
            case 'U':
                eat();
                if (c != '"') {
                    if (c == '\'')
                        return lexCharLit(Prefix_U);
                    return lexIdent("U");
                }
                return lexStringLit(SM.getPrevLexBufferPtr() - 1);
            case 'L':
                eat();
                if (c != '"') {
                    if (c == '\'')
                        return lexCharLit(Prefix_L);
                    return lexIdent("L");
                }
                return lexStringLit(SM.getPrevLexBufferPtr() - 1);
            case '.':
            {
                const char *Ptr = SM.getCurLexBufferPtr();
                eat(); // first
                if (Ptr[0] == '.' && Ptr[1] == '.') {
                    SM.advanceBufferPtr(1);
                    eat();
                    return TEllipsis;
                }
                if (llvm::isDigit(c))
                    return lexPPNumber(Ptr - 1);
                return TDot;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return lexPPNumber(SM.getPrevLexBufferPtr());
            case '\'': return lexCharLit(Prefix_none);
            case '(': eat(); return TLbracket;
            case ')': eat(); return TRbracket;
            case '~': eat(); return TBitNot;
            case '?': eat(); return TQuestionMark;
            case '{': eat(); return TLcurlyBracket;
            case '}': eat(); return TRcurlyBracket;
            case ',': eat(); return TComma;
            case '[': eat(); return TLSquareBrackets;
            case ']': eat(); return TRSquareBrackets;
            case ';': eat(); return TSemicolon;
            case '@': eat(); return TMouse;
            case '"':
                return lexStringLit(SM.getPrevLexBufferPtr());
                TOK2('*', '=', TAsignMul, TMul)
                TOK2('=', '=', TEq, TAssign)
                TOK2('^', '=', TAsignBitXor, TXor)
                TOK2('/', '=', TAsignDiv, TSlash)
                TOK2('!', '=', TNe, TNot)
                // ':>' convert to ']'
                TOK2(':', '>', TRSquareBrackets, TColon)
                TOK3('+', '+', TAddAdd, '=', TAsignAdd, TAdd)
                TOK3('>', '=', TGe, '>', Tshr, TGt)
                TOK3('|', '=', TAsignBitOr, '|', TLogicalOr, TBitOr)
                TOK3('&', '=', TAsignBitAnd, '&', TLogicalAnd, TBitAnd)
            // '%>' convert to '}'
            // '%:' convert to '#'
            case '%':
                eat();
                switch (c) {
                case '=': return eat(), TAsignRem;
                case '>': return eat(), TRcurlyBracket;
                case ':':
                    eat();
                    if (c == '%') {
                        eat();
                        if (c == ':') // convert '%:%:' to '##'
                            return PPSharpSharp;
                    }
                    goto RUN;
                default: return TPercent;
                }
                TOK4('-', '-', TSubSub, '=', TAsignSub, '>', TArrow, TDash)
                // '<:' convert to '['
                // '<%' convert to '{'
                TOK5('<', '<', Tshl, '=', TLe, ':', TLSquareBrackets, '%', TLcurlyBracket, TLt)
            default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c == '\\' ||
                    (unsigned char)c >= 128)
                    return lexIdent();
                warning(loc, "stray %C(ascii %u) in program", c, (unsigned)(unsigned char)c);
                break;
            }
            eat();
        }
    }

template <typename T> static void StringifyImpl(T &Str, char Quote) {
  typename T::size_type i = 0, e = Str.size();
  while (i < e) {
    if (Str[i] == '\\' || Str[i] == Quote) {
      Str.insert(Str.begin() + i, '\\');
      i += 2;
      ++e;
    } else if (Str[i] == '\n' || Str[i] == '\r') {
      // Replace '\r\n' and '\n\r' to '\\' followed by 'n'.
      if ((i < e - 1) && (Str[i + 1] == '\n' || Str[i + 1] == '\r') &&
          Str[i] != Str[i + 1]) {
        Str[i] = '\\';
        Str[i + 1] = 'n';
      } else {
        // Replace '\n' and '\r' to '\\' followed by 'n'.
        Str[i] = '\\';
        Str.insert(Str.begin() + i + 1, 'n');
        ++e;
      }
      i += 2;
    } else
      ++i;
  }
}
    std::string Stringify(StringRef Str, bool Charify) {
        std::string Result = std::string(Str);
        char Quote = Charify ? '\'' : '"';
        StringifyImpl(Result, Quote);
        return Result;
    }
    void formatChar(unsigned i, SmallVectorImpl<char> &Str) {
        switch (i) {
            case 7:
                Str.push_back('\\');
                Str.push_back('a');
                break;
            case 8:
                Str.push_back('\\');
                Str.push_back('b');
                break;
            case 12:
                Str.push_back('\\');
                Str.push_back('f');
                break;
            case 10:
                Str.push_back('\\');
                Str.push_back('n');
                break;
            case 13:
                Str.push_back('\\');
                Str.push_back('r');
                break;
            case 9:
                Str.push_back('\\');
                Str.push_back('t');;
                break;
            case 11:
                Str.push_back('\\');
                Str.push_back('v');
                break;
            default:
                Str.push_back('\\');
                Str.push_back('x');
                Str.push_back(hexed(i) >> 4);
                Str.push_back(hexed(i));
                break;
        }
    }
    void maybe_paste_tokens(size_t begin, size_t end) {
        for (;;) {
            if (begin == end)
                return;
            begin++;
            if (begin == end)
                return;
            const TokenV &H = tokenq[begin];
            if (H.tok == PPSharpSharp) {
                begin++;
                if (begin == end)
                    return;
                TokenV &R = tokenq[begin - 2];
                auto it = tokenq.begin() + begin;
                *it = paste_tokens(*it, R);
                it -= 2;
                it = tokenq.erase(it);
                tokenq.erase(it);
                end--;
            }
        }
    }
    TokenV createStringify(ArrayRef<TokenV> tokens) {
        SmallString<64> Str;
        Str.push_back('\"');
        for (const TokenV &it: tokens) {
            switch (it.tok) {
                case PPNumber:
                {
                    Str += it.getPPNumberLit();
                    break;
                }
                case TStringLit:
                {
                    std::string s = Stringify(it.getStringLiteral(), true);
                } break;
                case TCharLit:
                {
                    formatChar(it.i, Str);
                } break;
                default: 
                {
                    if (it.tok >= kw_start) 
                        Str += it.s->getKey();
                    else 
                        Str += show(it.tok); 
                    break;
                } break;
            }
        }
        Str.push_back('\"');
        return ScratchBuf.getToken(Str);
    }
    void checkMacro() {
        IdentRef name = tok.s;
        Token saved_tok;
        switch ((saved_tok = name->second)) {
        case PP__LINE__:
        case PP__COUNTER__: {
            unsigned val = saved_tok == PP__LINE__ ? SM.current_line : counter++;
            SmallString<13> str;
            raw_svector_ostream OS(str);
            OS << val;
            tok = ScratchBuf.getToken(str);
            return;
        }
        case PP__DATE__:
        case PP__TIME__: {
            time_t now = time(nullptr);
            struct tm *t = localtime(&now);

            char buffer[32];
            int N;
            if (saved_tok == PP__DATE__)
                // Mmm dd yyyy
                N = snprintf(buffer, sizeof(buffer), "\"%s %2d %4d\"", months[t->tm_mon], t->tm_mday, t->tm_year + 1900);
            else
                // hh:mm:ss
                N = snprintf(buffer, sizeof(buffer), "\"%02d:%02d:%02d\"", t->tm_hour, t->tm_min, t->tm_sec);
            tok = ScratchBuf.getToken(buffer, N);
            return;
        }
        case PP__FILE__: {
            SmallString<128> str;
            raw_svector_ostream OS(str);
            OS << '"' << SM.getFileName() << '"';
            tok = ScratchBuf.getToken(str);
            return;
        }
        case PP_Pragma: {
            cpp();
            if (tok.tok != TLbracket)
                return expectLB(loc);
            cpp();
            if (tok.tok != TStringLit)
                return expect(loc, "string literal");
            cpp();
            if (tok.tok != TRbracket)
                return expectRB(loc);
            cpp(); // advance to next token
            return;
        }
        default: {
            auto it = macros.find(name);
            if (it == macros.end()) {
                if (tok.tok == PPIdent)
                    tok.tok = TIdentifier;
                return;
            }
            const PPMacroDef *m = it->second;
            if (m->isObj()) {
                ArrayRef<TokenV> tokens = m->getTokens();
                if (tokens.size()) {
                    beginExpandMacro(it->second);
                    size_t start = tokenq.size();
                    for (size_t i = tokens.size(); i--;) {
                        TokenV theTok = tokens[i];
                        if (theTok.tok != TSpace) {
                            if (theTok.tok >= kw_start) {
                                if (isMacroInUse(theTok.s)) {
                                    // https://gcc.gnu.org/onlinedocs/cpp/Self-Referential-Macros.html
                                    if (theTok.tok == PPIdent)
                                        theTok.tok = TIdentifier;
                                }
                            }
                            tokenq.push_back(theTok);
                        }
                    }
                    size_t End = tokenq.size();
                    maybe_paste_tokens(start, End);
                    tokenq.push_back(SM.getLocTree());
                }
                return cpp();
            }
            if (m->isFunction()) {
                TokenV saved_token = tok; // copy ctor
                cpp();
                if (tok.tok == TLbracket) {
                    SmallVector<SmallVector<TokenV, 3>, 5> args;
                    args.push_back(SmallVector<TokenV, 3>());
                    cpp();
                    for (;; cpp()) {
                        if (tok.tok == TEOF) {
                            pp_error(loc, "unexpected EOF while parsing function-like macro arguments");
                            return;
                        }
                        if (tok.tok == TRbracket)
                            break;
                        if (tok.tok == TComma)
                            args.push_back(SmallVector<TokenV, 3>());
                        else if (tok.tok == TNewLine) {
                            //  new-line is considered a normal white-space character
                            args.back().emplace_back(TSpace);
                            isPPMode = true;
                            eat();
                        } else if (tok.tok == TSpace) {
                            if (args.back().size())
                                args.back().push_back(tok);
                        } else
                            args.back().push_back(tok);
                    }
                    for (size_t i = 0; i < args.size(); ++i) {
                        while (args[i].size() && args[i].back().tok == TSpace)
                            args[i].pop_back();
                    }
                    ArrayRef<IdentRef> params = m->getParams();
                    if (m->isVararg() ? args.size() < params.size() : args.size() != params.size()) {
                        if (args.size() == 1 && args.front().empty() && params.size() <= 1) {
                            if (params.empty())
                                args.clear();
                            else
                                args.front().clear();
                        } else {
                            return (void)pp_error(loc, "macro %I expect %z arguments, %z provided", name,
                                                  params.size(), args.size());
                        }
                    }
                    ArrayRef<TokenV> tokens = m->getTokens();
                    if (tokens.size()) {
                        beginExpandMacro(it->second);
                        size_t start = tokenq.size();
                        for (size_t i = tokens.size(); i--;) {
                            TokenV theTok = tokens[i];
                            if (theTok.tok != TSpace) {
                                if (theTok.tok >= kw_start) {
                                    IdentRef s = theTok.s;
                                    for (unsigned j = 0; j < params.size(); j++) {
                                        if (s == params[j]) {
                                            if (i && tokens[i - 1].tok == PPSharp) {
                                                i--;
                                                tokenq.push_back(createStringify(args[j]));
                                                goto CONTINUE;
                                            }
                                            for (const auto &it : args[j])
                                                tokenq.push_back(it);
                                            goto CONTINUE;
                                        }
                                    }
                                    if (isMacroInUse(s)) {
                                        if (theTok.tok == PPIdent)
                                            theTok.tok = TIdentifier;
                                    }
                                    tokenq.push_back(theTok);
                                } else {
                                    if (theTok.tok == PPSharp)
                                        pp_error(getLoc(), "'#' is not followed by a macro parameter");
                                    tokenq.push_back(theTok);
                                }
                            }
                            CONTINUE: ;
                        }
                        size_t End = tokenq.size();
                        maybe_paste_tokens(start, End);
                        tokenq.push_back(SM.getLocTree());
                    }
                    return cpp();
                }
                tokenq.push_back(tok);
                tok = saved_token;
                if (saved_token.tok == PPIdent)
                    tok.tok = TIdentifier;
                return;
            }
            llvm_unreachable("invalid macro kind");
        }
        }
    }
};
