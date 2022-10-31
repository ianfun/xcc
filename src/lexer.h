/* lexer.cpp - C's lexer and preprocessor */

struct Parser;

struct Lexer: public DiagnosticHelper
{
    TokenV tok = TNul;
    bool want_expr = false;
    bool isPPMode = false;
    llvm::SmallVector<uint8_t, 4> ppstack;
    bool ok = true;
    char c = ' ';
    DenseMap<IdentRef, PPMacro> macros;
    Parser &parser;
    SourceMgr &SM;
    std::deque<TokenV> tokenq;
    llvm::SmallPtrSet<IdentRef, 10> expansion_list{};
    xstring lexIdnetBuffer = xstring::get_with_capacity(20);
    uint32_t counter = 0;
    Location loc;
    Evaluator evaluator;
    
    Lexer(SourceMgr &SM, Parser &parser, xcc_context &context)
    : DiagnosticHelper{context}, parser{parser}, SM{SM}, evaluator{context} {

    }
    Location getLoc() { return loc; }
    void updateLoc() { loc = SM.getLoc(); }
    void validUCN(Codepoint codepoint, Location loc) {
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
    void eat() {
        c = SM.skip_read();
    }
Codepoint lexHexChar(){
    Codepoint n = 0;
    for(;;){
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
    Location loc = getLoc();
    unsigned n = 0, i = 0;

    for(;;){
        unsigned char t = c;
        i++;
        if (llvm::isDigit(t))
            n = (n << 4) | (t - '0');
        else if (t >= 'a' && t <= 'f')
            n = (n << 4) | (t - 'a' + 10);
        else if (t >= 'A' && t <= 'F')
            n = (n << 4) | (t - 'A' + 10);
        else{
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
Codepoint lexEscape(){
    eat();
    switch (c) {
    case 'a': return eat(), 7;
    case 'b': return eat(), 8;
    case 'f': return eat(), 12;
    case 'n': return eat(), 10;
    case 'r': return eat(), 13;
    case 't': return eat(), 9;
    case 'v': return eat(), 11;
    case 'e': case 'E': return eat(), 27;
    case 'x': return eat(), lexHexChar();
    case 'u': return eat(), lexUChar(4);
    case 'U': return eat(), lexUChar(8);
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
    {
        auto n = (Codepoint)c - '0';
        eat(); // eat first
        if ((unsigned char)c >= '0' && (unsigned char)c <= '7') {
            n = (n << 3) | ((unsigned char)c - '0');
            eat(); // eat second
            if ((unsigned char)c >= '0' && (unsigned char)c <= '7'){
                n = (n << 3) | ((unsigned char)c - '0');
                eat(); // eat third
            }
        }
        return n;
    }    
    default: 
        {
            auto c2 = c;
            eat();
            return c2;
        }
    }
}
TokenV lexCharLit(enum ITag I = Iint) {
    TokenV theTok = TokenV(ATokenVChar, TCharLit);
    theTok.itag = I;
    eat(); // eat '
    if (c == '\\') {
        auto codepoint = lexEscape();
        if (codepoint > 0xFF) {
            warning(loc, "character constant too large");
        }
        theTok.i = codepoint;
    }
    else {
        theTok.i = (unsigned char)c;
        eat();
    }
    if (c != '\'')
        lex_error(loc, "missing terminating " lquote "'" rquote " character in character literal");
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
    for(;;) {
        unsigned n = 8;
        if (c == '\\'){
            eat();
            if (c == 'U'){
                R:
                eat();
                auto codepoint = lexUChar(n);
                cat_codepoint(lexIdnetBuffer, codepoint);
            } else if (c == 'u'){
                n = 4;
                goto R;
            }
            else {
                lex_error(loc, "%s", "stray '\\' in program");
                eat();
            }
        }
        else {
            if (!(isalnum(c) ||  c == '_' || (unsigned char)c & 128 || c == '$')){
                // if (validate_utf8((const unsigned char*)t.l.tok->s.data(),  t.l.tok->s.size()) != UTF8_ACCEPT){
                //     lex_error("invalid utf-8 identifier literal");
                // }
                break;
            }
            lexIdnetBuffer.push_back(c);
            eat();
        }
    }
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
        case 'E':
        {
            s.push_back(c);
            eat();
            if (c == '+' || c == '-')
                s.push_back(c), eat();
            return lexPPNumberEnd(s);
        }
        default:
        {
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
TokenV lexStringLit(uint8_t enc) {
    xstring str = xstring::get();
    eat(); // eat "
    TokenV theTok = TokenV(ATokenVStrLit, TStringLit);
    for(;;) {
        if (c == '\\')
            cat_codepoint(str, lexEscape());
        else if (c == '"'){
            eat();
            break;
        }
        else {
            if (c == '\0') {
                lex_error(loc, "missing terminating \" character");
                break;
            }
            str.push_back(c);
            eat();
        }
    }
    theTok.str = str;
    theTok.enc = enc;
//    if (!IsUTF8((const unsigned char*)str.data(), str.size())) {
//        lex_error("invalid utf-8 string literal");
//    }
    return theTok;
}
TokenV lex();

void beginExpandMacro(IdentRef a) {
    expansion_list.insert(a);
    tokenq.push_back(TokenV(ATokenIdent, PPMacroPop));
}
void endExpandMacro(IdentRef a) {
    expansion_list.erase(a);
}
bool isMacroInUse(IdentRef a) {
    return expansion_list.contains(a);
}
void checkMacro();
void cpp() {
    if (tokenq.empty())
        tok = lex();
    else
        tok = tokenq.front(), tokenq.pop_front();
    if (tok.tok >= kw_start && tok.tok != TIdentifier) {
        isPPMode = true;
        checkMacro();
        isPPMode = false;
        if (tok.tok == TNewLine)
            return cpp();
    } else if (tok.tok == PPMacroPop)
        return endExpandMacro(tok.s), cpp();
}
};

void Lexer::checkMacro() {
    IdentRef name = tok.s;
    Token saved_tok;
    switch ((saved_tok=name->second.getToken())) {
        case PP__LINE__:
        case PP__COUNTER__:
        {
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
        case PP__TIME__:
        {
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
        case PP__FILE__:
        {
            tok = TokenV(ATokenVStrLit, TStringLit);
            tok.str = xstring::get(SM.getFileName());
            return;
        }
        case PP_Pragma:
        {
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
        default: 
        {
            auto it = macros.find(name);
            if (it == macros.end()) {
                if (tok.tok == PPIdent) 
                    tok.tok = TIdentifier;
                return;
            }
            PPMacro &m = it->second;
            switch (it->second.k) {
                case MOBJ:
                {
                    if (m.tokens.size()) {
                        beginExpandMacro(name);
                        for (size_t i = 0;i < m.tokens.size();i++) {
                            TokenV theTok = m.tokens[i]; // copy ctor
                            if (theTok.tok != TSpace) {
                                if (tok.tok >= kw_start && isMacroInUse(theTok.s)) {
                                    // https://gcc.gnu.org/onlinedocs/cpp/Self-Referential-Macros.html
                                    dbgprint("skipping self-referential macro %s\n", theTok.s->getKey().data());
                                    if (theTok.tok == PPIdent)
                                        theTok.tok = TIdentifier;
                                }
                                tokenq.push_back(theTok);
                            }
                        }
                    }
                    return cpp();
                }
                case MFUNC:
                {
                    TokenV saved_token = tok; // copy ctor
                    cpp();
                    if (tok.tok == TLbracket) {
                        SmallVector<SmallVector<TokenV, 3>, 5> args;
                        args.push_back(SmallVector<TokenV, 3>());
                        cpp();
                        for(;;cpp()) {
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
                            }
                            else if (tok.tok == TSpace) {
                                if (args.back().size())
                                    args.back().push_back(tok);
                            }
                            else
                                args.back().push_back(tok);
                        }
                        for (size_t i = 0;i < args.size();++i) {
                            while (args[i].size() && args[i].back().tok == TSpace)
                                args[i].pop_back();
                        }
                        if (m.ivarargs ? args.size() < m.params.size() : args.size() != m.params.size()) {
                            pp_error(loc, "macro %I expect %z arguments, %z provided", name, m.params.size(), args.size());
                            return;
                        }
                        if (m.tokens.size()) {
                            beginExpandMacro(name);
                            for (size_t i = 0;i < m.tokens.size();i++) {
                                TokenV theTok = m.tokens[i]; // copy ctor
                                if (theTok.tok != TSpace) {
                                    if (theTok.tok >= kw_start) {
                                        size_t j;
                                        IdentRef s = theTok.s;
                                        for (j = 0;j < m.params.size();j++) {
                                            if (s == m.params[i]) {
                                                // xxx: llvm::SmallVector's append does better
                                                for (const auto it: args[i])
                                                    tokenq.push_back(it);
                                                goto BREAK;
                                            }
                                        }
                                        if (isMacroInUse(s)) {
                                            if (theTok.tok == PPIdent)
                                                theTok.tok = TIdentifier;
                                        }
                                        tokenq.push_back(theTok);
                                        BREAK: ;
                                    }
                                } else {
                                    tokenq.push_back(theTok);
                                }
                            }
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
        } // end default
    } // end switch
}

