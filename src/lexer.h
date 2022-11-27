/* lexer.cpp - C's lexer and preprocessor */

struct Parser;

struct Lexer : public EvalHelper {
    enum PPFlags : uint8_t {
        PFNormal = 1,
        PFPP = 2
    };
    static const char months[12][4];
    TokenV tok = TNul;
    bool want_expr = false;
    bool isPPMode = false;
    bool isDisableSpace = false;
    llvm::SmallVector<uint8_t, 4> ppstack;
    bool ok = true;
    char c = ' ';
    DenseMap<IdentRef, PPMacroDef*> macros;
    Parser &parser;
    SourceMgr &SM;
    std::deque<TokenV> tokenq;
    std::vector<PPMacroDef*> expansion_list;
    xstring lexIdnetBuffer = xstring::get_with_capacity(20);
    uint32_t counter = 0;
    location_t loc;
    xcc_context &context;

    Lexer(SourceMgr &SM, Parser &parser, xcc_context &context, DiagnosticsEngine &Diag) : EvalHelper{Diag}, parser{parser}, SM{SM}, context{context} { }
    location_t getLoc() { return loc; }
    Expr constant_expression();
    void updateLoc() { loc = SM.getLoc(); }
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
    void eat() { c = SM.skip_read(); }
    Codepoint lexHexChar() {
        Codepoint n = 0;
        for (;;) {
            unsigned char t = c;
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
    Codepoint lexUChar(unsigned count) {
        location_t loc = getLoc();
        unsigned n = 0, i = 0;

        for (;;) {
            unsigned char t = c;
            i++;
            if (llvm::isDigit(t))
                n = (n << 4) | (t - '0');
            else if (t >= 'a' && t <= 'f')
                n = (n << 4) | (t - 'a' + 10);
            else if (t >= 'A' && t <= 'F')
                n = (n << 4) | (t - 'A' + 10);
            else {
                warning("unexpected token %C, expect %u hex digits for universal character", t, count);
                break;
            }
            eat();
            if (i == count)
                break;
        }
        validUCN(n, loc);
        return n;
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
        case 'x': return eat(), lexHexChar();
        case 'u': return eat(), lexUChar(4);
        case 'U': return eat(), lexUChar(8);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            auto n = (Codepoint)c - '0';
            eat(); // eat first
            if ((unsigned char)c >= '0' && (unsigned char)c <= '7') {
                n = (n << 3) | ((unsigned char)c - '0');
                eat(); // eat second
                if ((unsigned char)c >= '0' && (unsigned char)c <= '7') {
                    n = (n << 3) | ((unsigned char)c - '0');
                    eat(); // eat third
                }
            }
            return n;
        }
        default: {
            auto c2 = c;
            eat();
            return c2;
        }
        }
    }
    TokenV lexCharLit(enum StringPrefix enc = Prefix_none) {
        // C23
        // An integer character constant is a sequence of one or more multibyte characters enclosed in singlequotes, as in ’x’. A UTF-8 character constant is the same, except prefixed by u8. A wchar_t character constant is prefixed by the letter L. A UTF-16 character constant is prefixed by the letter u. A UTF-32 character constant is prefixed by the letter U.
        TokenV theTok = TokenV(ATokenVChar, TCharLit);
        theTok.itag = enc;
        eat(); // eat '
        if (c == '\\') {
            auto codepoint = lexEscape();
            // C23
            // If an integer character constant contains a single character or escape sequence, its value is the one that results when an object with type char whose value is that of the single character or escape sequence is converted to type int.
            if (codepoint > 0xFF && (enc == Prefix_none || enc == Prefix_u8))
                warning(loc, "character constant exceeds 8 bit");
            else if (codepoint > 0xFFFF && (enc == Prefix_L || enc == Prefix_u)) {
                if (enc == Prefix_L)
                    warning(loc, "A wchar_t character constant exceeds 16 bit is not portable");
                else
                    warning(loc, "UTF-16 character constant exceeds 16 bit");
            }
            // C23
            // In an implementation in which type char has the same range of values as signed char, the integer character constant ’\xFF’ has the value −1; if type char has the same range of values as unsigned char, the character constant ’\xFF’ has the value +255.
            theTok.i = codepoint;
        } else {
            theTok.i = (unsigned char)c;
            eat();
        }
        if (c != '\'')
            warning(loc, "missing terminating " lquote "'" rquote " character in character literal");
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
    TokenV lexIdent() {
        TokenV theTok = TokenV(ATokenIdent, PPIdent);
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
                if (!(isalnum(c) || c == '_' || (unsigned char)c & 128 || c == '$'))
                    break;

                lexIdnetBuffer.push_back(c);
                eat();
            }
        }
        if (!enc::IsUTF8(lexIdnetBuffer.str()))
            warning(loc, "identfier is not UTF-8 encoded");
        theTok.s = context.table.get(lexIdnetBuffer.str(), PPIdent);
        theTok.tok = std::min(theTok.tok, theTok.s->second.getToken());
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

    TokenV lexPPNumberEnd(xstring &s) {
        switch (c) {
        case 'p':
        case 'P':
        case 'e':
        case 'E': {
            s.push_back(c);
            eat();
            if (c == '+' || c == '-')
                s.push_back(c), eat();
            return lexPPNumberEnd(s);
        }
        default: {
            if (c == '\'')
                eat();
            if (isalnum(c) || c == '.') {
                return lexPPNumberDigits(s);
            }
            if (c == '\\') {
                Codepoint codepoint;
                s.push_back(c);
                eat();
                if (c == 'u')
                    codepoint = lexUChar(4);
                else if (c == 'U')
                    codepoint = lexUChar(8);
                else
                    goto END;
                cat_codepoint(s, codepoint);
END:
                return lexPPNumberEnd(s);
            }
            s.make_eos();
            TokenV theTok = TokenV(ATokenVNumLit, PPNumber);
            theTok.str = s;
            return theTok;
        }
        }
    }
    TokenV lexPPNumberDigits(xstring &s) {
        do {
            s.push_back(c);
            eat();
            if (c == '\'')
                eat();
        } while (isalnum(c) || c == '\'');
        return lexPPNumberEnd(s);
    }
    TokenV lexPPNumberAfterDot() {
        xstring s = xstring::get_with_capacity(14);
        s.push_back('.');
        return lexPPNumberDigits(s);
    }
    TokenV lexPPNumber() {
        xstring s = xstring::get_with_capacity(13);
        return lexPPNumberDigits(s);
    }
    TokenV lexStringLit(enum StringPrefix enc = Prefix_none) {
        xstring str = xstring::get();
        eat(); // eat "
        TokenV theTok = TokenV(ATokenVStrLit, TStringLit);
        for (;;) {
            if (c == '\\')
                cat_codepoint(str, lexEscape());
            else if (c == '"') {
                eat();
                break;
            } else {
                if (c == '\0') {
                    warning(loc, "missing terminating \" character");
                    break;
                }
                str.push_back(c);
                eat();
            }
        }
        theTok.str = str;
        theTok.str.push_back(static_cast<char>(enc));
        return theTok;
    }
    void beginExpandMacro(PPMacroDef *M) {
        SM.beginExpandMacro(M, context);
        expansion_list.push_back(M);
        tokenq.push_back(SM.getLocTree());
        tokenq.push_back(TokenV(PPMacroPop));
    }
    bool isMacroInUse(IdentRef Name) const { 
        for (const auto &it: expansion_list)
            if (it->m.Name == Name)
                return true;
        return false;
    }
    void cpp() {
        if (tokenq.empty())
            tok = lex();
        else
            tok = tokenq.back(), tokenq.pop_back();
        if (tok.tok >= kw_start && tok.tok != TIdentifier) {
            isPPMode = true;
            checkMacro();
            isPPMode = false;
            if (tok.tok == TNewLine)
                return cpp();
        } else if (tok.tok == PPMacroPop) {
            SM.endTree();
            expansion_list.pop_back();
            return cpp();
        } else if (tok.tok == PPMacroTraceLoc) {
            //loc.setParent(tok.tree);
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

    TokenV lex() {
        // Tokenize
        for (;;) {
            if (c == '\0') {
                return isPPMode = false, TEOF;
            }
            if (isCSkip(c)) {
                for (;;) {
                    eat();
                    if (!isCSkip(c))
                        break;
                }
                if (isDisableSpace || want_expr)
                    continue;
                if (isPPMode) 
                    return TSpace;
                continue;
            }
            updateLoc();
            if (c == '#') {
                eat();
                goto RUN;
BAD_RET:
                return isPPMode = false, lex();
RUN:
                if (isPPMode) {
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
                Token saved_tok = saved_name->second.getToken();
                if (saved_tok != PPinclude) {
                    tok = lex();
                    if (tok.tok == TSpace)
                        tok = lex();
                }
                switch (saved_tok) {
                case PPdefine: {
                    PPMacroDef *theMacro = context.newMacro();
                    theMacro->loc = SM.getLoc();
                    IdentRef name;
                    if (tok.tok < kw_start) {
                        pp_error(loc, "%s", "expect identifier");
                        goto BAD_RET;
                    }
                    name = tok.s;
                    theMacro->m.Name = name;
                    if (tok.tok >= PP__LINE__ && tok.tok <= PP_Pragma)
                        warning(loc, "redefining builtin macro %I", name);
                    tok = lex();
                    if (tok.tok == TLbracket)
                        theMacro->m.k = MFUNC;
                    while (tok.tok == TSpace)
                        tok = lex();
                    if (theMacro->m.k == MFUNC) {
                        isDisableSpace = true;
                        for (;;) {
                            tok = lex();
                            if (tok.tok >= kw_start) {
                                theMacro->m.params.push_back(tok.s);
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
                                theMacro->m.ivarargs = true;
                                tok = lex();
                                if (tok.tok != TRbracket) {
                                    parse_error(loc, "%s", "')' expected");
                                    goto BAD_RET;
                                }
                                continue;
                            }
                            if (tok.s->second.getToken() == PP__VA_ARGS__) {
                                theMacro->m.ivarargs = true;
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
                    while (isPPMode) {
                        theMacro->m.tokens.push_back(tok), tok = lex();
                    }
                    if (theMacro->m.tokens.size())
                        while (theMacro->m.tokens.back().tok == TSpace)
                            theMacro->m.tokens.pop_back();
                    bool ok = true;
                    if (theMacro->m.tokens.size()) {
                        if (theMacro->m.tokens.front().tok == PPSharpSharp)
                            parse_error(loc, "%s", "'##' cannot appear at start of macro expansion"), ok = false;
                        if (theMacro->m.tokens.size() >= 2)
                            if (theMacro->m.tokens.back().tok == PPSharpSharp)
                                parse_error(loc, "'##' cannot appear at end of macro expansion"), ok = false;
                    }
                    if (ok) {
                        const auto it = macros.find(name);
                        if (it != macros.end())
                            if (!theMacro->m.equals(it->second->m))
                                warning(loc, "macro %I redefined", name);
                        macros[name] = theMacro;
                    }
                } break;
                case Kif: {
                    Expr e;

                    want_expr = true;
                    e = constant_expression();
                    want_expr = false;
                    ok = force_eval(e, e->loc);
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
                        location_t cloc = getLoc();
                        Expr e;
                        want_expr = true;
                        e = constant_expression();
                        want_expr = false;
                        ok = false;
                        if (!e)
                            pp_error(loc, "%s", "expect constant_expression");
                        else {
                            ok = force_eval(e, cloc);
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
                    if (SM.includeStack.empty()) {
                        pp_error(loc, "unexpected EOF");
                        goto BAD_RET;
                    }
                    auto theFD = SM.includeStack.back();
                    auto &f = SM.streams[theFD];
                    char is_std = '\"';
                    xstring path = xstring::get();

                    switch (c) {
                    case '"':
STD_INCLUDE:
                        for (;;) {
                            eat();
                            if (c == is_std) {
                                auto size = SM.includeStack.size();
                                for (;;) {
                                    char c = SM.raw_read(f);
                                    if (c == '\0') {
                                        break;
                                    }
                                    if (c == '\n' || c == '\r')
                                        break;
                                }
                                if (path.empty()) {
                                    pp_error("%s", "empty filename in #include");
                                    path.free();
                                }
                                else {
                                    path.make_eos();
                                    size = SM.includeStack.size();
                                    SM.addIncludeFile(path.str(), is_std == '>');
                                    if (SM.includeStack.size() == size) {
                                        pp_error(loc, "#include file not found: %R", StringRef(path.data(), path.length() - 1));
                                        path.free();
                                    } else { // TODO
                                        SM.beginInclude(context, theFD);
                                    }
                                }
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
                    default: pp_error(loc, "%s", "expect \"FILENAME\" or <FILENAME>"); goto BAD_RET;
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
                    SmallString<256> s;
                    for (; c != '\n' && c != '\0';) {
                        s.push_back(c), eat();
                    }
                    if (saved_tok == PPwarning)
                        warning(loc, "#warning: %R", s.str());
                    else
                        pp_error(loc, "#error: %R", s.str());
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
            case '\n':
                if (isPPMode) {
                    return isPPMode = false, TNewLine;
                }
                eat();
                continue;
            case 'u':
                eat();
                if (c == '"')
                    return lexStringLit(Prefix_u);
                if (c == '\'')
                    return lexCharLit(Prefix_u);
                if (c == '8') {
                    eat();
                    if (c != '"') {
                        if (c == '\'')
                            return lexCharLit(Prefix_u8);
                        return lexIdent("u8");
                    }
                    return lexStringLit(Prefix_u8);
                }
                return lexIdent("u");
            case 'U':
                eat();
                if (c != '"') {
                    if (c == '\'')
                        return lexCharLit(Prefix_U);
                    return lexIdent("U");
                }
                return lexStringLit(Prefix_U);
            case 'L':
                eat();
                if (c != '"') {
                    if (c == '\'')
                        return lexCharLit(Prefix_L);
                    return lexIdent("L");
                }
                return lexStringLit(Prefix_L);
            case '.':
                eat(); // first
                if (c == '.') {
                    eat(); // second
                    if (c != '.')
                        return TEllipsis2;
                    eat(); // third
                    return TEllipsis;
                }
                if (llvm::isDigit(c))
                    return lexPPNumberAfterDot();
                return TDot;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return lexPPNumber();
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
                return lexStringLit(Prefix_none);
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
            } // end switch
            eat();
        } // end for
    }  // end lex
    void checkMacro() {
        IdentRef name = tok.s;
        Token saved_tok;
        switch ((saved_tok = name->second.getToken())) {
        case PP__LINE__:
        case PP__COUNTER__: {
            // 4294967295 - the highest unsigned 32 bit value, require 10 chars
            xstring str = xstring::get_with_length(11);
            {
                uint32_t val = saved_tok == PP__LINE__ ? SM.getLine() : counter++;
                str[0] = '0' + val / 1000000000;
                str[1] = '0' + val / 100000000;
                str[2] = '0' + val / 10000000;
                str[3] = '0' + val / 1000000;
                str[4] = '0' + val / 100000;
                str[5] = '0' + val / 10000;
                str[6] = '0' + val / 1000;
                str[7] = '0' + val / 100;
                str[8] = '0' + val / 10;
                str[9] = '0' + val;
                str[10] = '\0';
            }
            tok = TokenV(ATokenVStrLit, TStringLit);
            tok.str = str;
            return;
        }
        case PP__DATE__:
        case PP__TIME__: {
            time_t now = time(nullptr);
            struct tm *t = localtime(&now);

            xstring str = xstring::get_with_capacity(32);
            if (saved_tok == PP__DATE__)
                // Mmm dd yyyy
                str.msize() = snprintf(str.data(), 32, "%s %2d %4d", months[t->tm_mon], t->tm_mday, t->tm_year + 1900);
            else
                // hh:mm:ss
                str.msize() = snprintf(str.data(), 32, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
            tok = TokenV(ATokenVStrLit, TStringLit);
            tok.str = str;
            return;
        }
        case PP__FILE__: {
            tok = TokenV(ATokenVStrLit, TStringLit);
            tok.str = xstring::get(SM.getFileName());
            return;
        }
        case PP_Pragma: {
            cpp();
            if (tok.tok != TLbracket)
                return expectLB(loc);
            cpp();
            if (tok.tok != TStringLit)
                return expect(loc, "expect string literal");
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
            PPMacro &m = it->second->m;
            switch (m.k) {
            case MOBJ: {
                if (m.tokens.size()) {
                    beginExpandMacro(it->second);
                    for (size_t i = m.tokens.size();i--; ) {
                        TokenV theTok = m.tokens[i]; // copy ctor
                        if (theTok.tok != TSpace) {
                            if (tok.tok >= kw_start) {
                                if (isMacroInUse(theTok.s)) {
                                    // https://gcc.gnu.org/onlinedocs/cpp/Self-Referential-Macros.html
                                    if (theTok.tok == PPIdent)
                                        theTok.tok = TIdentifier;
                                }
                            }
                            tokenq.push_back(theTok);
                        }
                    }
                    tokenq.push_back(SM.getLocTree());
                }
                return cpp();
            }
            case MFUNC: {
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
                    if (m.ivarargs ? args.size() < m.params.size() : args.size() != m.params.size()) {
                        if (args.size() == 1 && args.front().empty() && m.params.size() <= 1) {
                            if (m.params.size() == 0)
                                args.clear();
                            else
                                args.front().clear();
                        } else {
                            return (void)pp_error(loc, "macro %I expect %z arguments, %z provided", name, m.params.size(), args.size());
                        }
                    }
                    if (m.tokens.size()) {
                        beginExpandMacro(it->second);
                        for (size_t i = m.tokens.size();i--; ) {
                            TokenV theTok = m.tokens[i];
                            if (theTok.tok != TSpace) {
                                if (theTok.tok >= kw_start) {
                                    IdentRef s = theTok.s;
                                    for (unsigned j = 0; j < m.params.size(); j++) {
                                        if (s == m.params[j]) {
                                            for (const auto &it : args[j]) 
                                                tokenq.push_back(it);
                                            goto BREAK;
                                        }
                                    }
                                    if (isMacroInUse(s)) {
                                        if (theTok.tok == PPIdent)
                                            theTok.tok = TIdentifier;
                                    }
                                    tokenq.push_back(theTok);
                                } else {
                                    tokenq.push_back(theTok);
                                }
                                BREAK:;
                            }
                        }
                        tokenq.push_back(SM.getLocTree());
                    }
                    return cpp();
                }
                tokenq.push_back(tok);
                tok = saved_token;
                if (saved_token.tok == PPIdent)
                    tok.tok = TIdentifier;
                return;
            } // end cast
            default: llvm_unreachable("bad macro kind");
            } // end switch
        }     // end default
        }     // end switch
    }
};
