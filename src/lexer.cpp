#define TOK2(tc, ac, at, bt) \
case tc: \
    eat(); \
    if (c == ac) \
        return eat(), at; \
    return bt;

#define TOK3(tc, ac, at, bc, bt, d) \
case tc: \
    eat(); \
    if (c == ac) \
        return eat(), at; \
    if (c == bc) \
        return eat(), bt; \
    return d;

#define TOK4(tc, ac, at, bc, bt, cc, ct, d) \
case tc: \
    eat(); \
    switch (c) { \
        case ac: \
            return eat(), at; \
        case bc: \
            return eat(), bt; \
        case cc: \
            return eat(), ct; \
        default: \
            return d; \
    }

#define TOK5(tc, ac, at, bc, bt, cc, ct, dc, dt, d) \
case tc: \
    eat(); \
    switch (c) { \
        case ac: \
            return eat(), at; \
        case bc: \
            return eat(), bt; \
        case cc: \
            return eat(), ct; \
        case dc: \
            return eat(), dt; \
        default: \
            return d; \
    }

TokenV Lexer::lex() {
    // Tokenize
    for(;;){
        if (c == '\0') {
            return isPPMode = false, TEOF;
        }
        if (isCSkip(c)) {
            for(;;){
                eat();
                if (!isCSkip(c))
                    break;
            }
            if (isPPMode && !want_expr) {
                return TSpace;
            }
            continue;
        }
        updateLoc();
        if (c == '#') {
            goto RUN;
BAD_RET:
            return isPPMode = false, lex();
RUN:
            if (isPPMode) {
                eat();
                if (c == '#')
                    return eat(), PPSharpSharp;
                return PPSharp;
            }
            isPPMode = true;
            eat();
            tok = lex(); // directive
            if (tok.tok < kw_start) {
                pp_error(loc, "%s", "expect identifier");
                goto BAD_RET;
            }

            
            IdentRef saved_name = tok.s;
            Token saved_tok = saved_name->second.getToken();
            if (saved_tok != PPinclude) {
                do {
                    tok = lex();
                } while (tok.tok == TSpace);
            }
            switch (saved_tok) {
                case PPdefine:
                {
                    PPMacro m = PPMacro();
                    IdentRef name;
    
                    if (tok.tok < kw_start) {
                        pp_error(loc, "%s", "expect identifier");
                        goto BAD_RET;
                    }
                    name = tok.s;
                    if (tok.tok >= PP__LINE__ && tok.tok <= PP_Pragma)
                        warning(loc, "redefining builtin macro %I", name);
                    tok = lex();
                    if (tok.tok == TLbracket)
                        m.k = MFUNC;
                    while (tok.tok == TSpace)
                        tok = lex();
                    if (m.k == MFUNC) {
                        for(;;) {
                            tok = lex();
                            if (tok.tok == PPIdent) {
                                m.params.push_back(tok.s);
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
                                m.ivarargs = true;
                                tok = lex();
                                if (tok.tok != TRbracket) {
                                    parse_error(loc, "%s", "')' expected");
                                    goto BAD_RET;
                                }
                                continue;
                            }
                            if (tok.s->second.getToken() == PP__VA_ARGS__) {
                                m.ivarargs = true;
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
                                parse_error(loc, "unexpected token: %C", c);
                        }
                        tok = lex(); // eat ')'
                        while (tok.tok == TSpace)
                            tok = lex();
                    }
                    while (isPPMode) {
                        m.tokens.push_back(tok), tok = lex();
                    }
                    if (m.tokens.size())
                        while (m.tokens.back().tok == TSpace)
                            m.tokens.pop_back();
                    bool ok = true;
                    if (m.tokens.size()) {
                        if (m.tokens.front().tok == PPSharpSharp)
                            parse_error(loc, "%s", "'##' cannot appear at start of macro expansion"), ok = false;
                        if (m.tokens.size() >= 2)
                            if (m.tokens.back().tok == PPSharpSharp)
                                parse_error(loc, "'##' cannot appear at end of macro expansion"), ok = false;
                    }
                    if (ok) {
                        const auto it = macros.find(name);
                        if (it != macros.end())
                            if (!m.equals(it->second))
                                warning(loc, "macro %I redefined", name);
                        macros[name] = m;
                    }
                } break;
                case Kif:
                {
                    Expr e;
    
                    want_expr = true;
                    e = parser.constant_expression();
                    want_expr = false;
                    ok = true;
                    // TODO
                    dbgprint("#if: %s\n", ok ? "true" : "false");
                    ppstack.push_back(ok ? 1 : 0);
                    (void)e;
                } break;
                case PPifdef: case PPifndef:
                {
                    if (tok.tok < kw_start) {
                        isPPMode = false;
                        pp_error(loc, "expect identifier");
                        goto BAD_RET;
                    }
                    ok = macros.find(tok.s) != macros.end();
                    if (saved_tok == PPundef)
                        ok = !ok;
                    dbgprint(saved_tok == PPundef ? "#ifdef: %s\n" : "#ifndef: %s\n", ok ? "true" : "false");
                    ppstack.push_back(ok ? 1 : 0);
                } break;
                case Kelse:
                {
                    if (ppstack.empty()){
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
                }break;
                case PPelif:
                {
                    if (ppstack.empty()) {
                        pp_error(loc, "%s", "no matching #if");
                        break;
                    }
                    if (ppstack.back() & 2) {
                        pp_error(loc, "%s", "#elif after #else");
                        break;
                    }
                    if (!ok) {
                        Location cloc = getLoc();
                        Expr e;
                        want_expr = true;
                        e = parser.constant_expression();
                        want_expr = false;
                        ok = false;
                        if (!e)
                            pp_error(loc, "%s", "expect constant_expression");
                        else {
                            ok = force_eval(e, cloc);
                            dbgprint("#if: %s\n", ok ? "true" : "false");
                        }
                    }
                    else
                        ok = false;
                }break;
                case PPendif:
                {
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
                }break;
                case PPinclude:
                {
                    while (c == ' ')
                        eat();
                    char is_std = '\"';
                    xstring path = xstring::get();
    
                    switch (c) {
                        case '"':
                        STD_INCLUDE:
                            for(;;) {
                                eat();
                                if (c == is_std) {
                                    eat();
                                    break;
                                }
                                if (c == '\0' || c == '\n') {
                                    pp_error(loc, "%s", "unexpected EOF, expect path or '\"'");
                                    goto BAD_RET;
                                }
                                path.push_back(c);
                            }
                            path.make_eos();
                            dbgprint("#including file %s\n", path.data());
                            if (!SM.addIncludeFile(path.str(), is_std == '>'))
                                pp_error("#include file not found: %R", StringRef(path.data(), path.length() - 1));
                            path.free();
                            break;
                        case '<':
                            is_std = '>';
                            goto STD_INCLUDE;
                        default:
                            pp_error(loc, "%s", "expect \"FILENAME\" or <FILENAME>");
                            goto BAD_RET;
                    }
                } break;
                case PPline:
                {
                    uint32_t line = 0;
                    if (!llvm::isDigit(c)) {
                        pp_error(loc, "%s", "expect digits (positive line number)");
                        break;
                    }
                    do 
                        line =  line * 10 + (unsigned char)c;
                    while (eat(), llvm::isDigit(c));
                    SM.setLine(line);
                    while (isCSkip(c))
                        eat();
                    if (c == '"') {
                        xstring str = xstring::get(SM.getFileName());
                        for(;;) {
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
                case PPundef:
                {
                    if (tok.tok < kw_start)
                        pp_error(loc, "%s", "macro name should be a identifier");
                    else {
#if CC_DEBUG
                        macros.erase(tok.s);
#else
                        dbgprint("#undef: ", macros.erase(tok.s) ? "found" : "not found");
#endif
                    }
                }break;
                case PPpragma:
                {
                    llvm::SmallVector<TokenV> pragmas;
                    while (isPPMode) {
                        if (tok.tok != TSpace)
                            pragmas.push_back(tok);
                        tok = lex();
                    }
                    // TODO: pragma
                } break;
                case PPerror: case PPwarning:
                {
                    SmallString<256> s;
                    for(;c != '\n' && c != '\0';) {
                        s.push_back(c), eat();
                    }
                    if (saved_tok == PPwarning)
                        warning(loc, "#warning: %R", s.str());
                    else
                        pp_error(loc, "#error: %R", s.str());
                } break;
                case TNewLine:
                    continue;
                default:
                    pp_error(loc, "invalid preprocessing directive: %I", saved_name);
            }
            while (tok.tok != TNewLine && tok.tok != TEOF) 
                tok = lex();
            isPPMode = false;
            continue;
        }
        if (!isPPMode && !ok) {
            char lastc = c;
            for(;;) {
                if (c == '\0')
                    return isPPMode = false, TEOF;
                lastc = c;
                eat();
                if (lastc == '\n' && c == '#')
                    goto RUN;
            }
        }
        switch (c) {
            case '\n': if (isPPMode) { return isPPMode=false, TNewLine; } eat(); continue;
            case 'u':
                eat();
                if (c == '"')
                    return lexStringLit(16);
                if (c == '\'')
                    return lexCharLit(Ilong);
                if (c == '8') {
                    eat();
                    if (c != '"') {
                        if (c == '\'')
                            return lexCharLit();
                        return lexIdent("u8");
                    }
                    return lexStringLit(8);
                }
                return lexIdent("u");
            case 'U':
                eat();
                if (c != '"') {
                    if (c == '\'')
                        return lexCharLit(Iulong);
                    return lexIdent("U");
                }
                return lexStringLit(32);
            case 'L':
                eat();
                if (c != '"') {
                    if (c == '\'')
                        return lexCharLit(Ilonglong);
                    return lexIdent("L");
                }
                return lexStringLit(16);
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
            case '0': case '1': case '2': case '3': case '4': case '5': 
            case '6': case '7': case '8': case '9':  return lexPPNumber();
            case '\'':return lexCharLit();
            case '(': eat(); return TLbracket;
            case ')': eat(); return TRbracket;
            case '~': eat(); return TBash;
            case '?': eat(); return TQuestionMark;
            case '{': eat(); return TLcurlyBracket;
            case '}': eat(); return TRcurlyBracket;
            case ',': eat(); return TComma; 
            case '[': eat(); return TLSquareBrackets;
            case ']': eat(); return TRSquareBrackets; 
            case ';': eat(); return TSemicolon;
            case '@': eat(); return TMouse;
            case '"': return lexStringLit(8);
            TOK2('*', '=', TAsignMul, TMul)
            TOK2('=', '=', TEq, TAssign)
            TOK2('^', '=', TAsignBitXor, TXor)
            TOK2('/', '=', TAsignDiv, TSlash)
            TOK2('!', '=', TNe, TNot)
            TOK2(':', '>', TRSquareBrackets, TColon)
            TOK3('+', '+', TAddAdd, '=', TAsignAdd, TAdd)
            TOK3('>', '=', TGe, '>', Tshr, TGt)
            TOK3('|', '=', TAsignBitOr, '|', TLogicalOr, TBitOr)
            TOK3('&', '=', TAsignBitAnd, '&', TLogicalAnd, TBitAnd)
            TOK4('%', '=', TAsignRem, '>', TRcurlyBracket, ':', TBash, TPercent)
            TOK4('-', '-', TSubSub, '=', TAsignSub, '>', TArrow, TDash)
            TOK5('<', '<', Tshl, '=', TLe, ':', TLSquareBrackets, '%', TLcurlyBracket, TLt)
            default: 
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c == '\\' || (unsigned char)c >= 128)
                    return lexIdent();
                warning(loc, "stray %C(ascii %u) in program", c, (unsigned)(unsigned char)c);
                break;
        } // end switch
        eat();
    } // end for
} // end lex
