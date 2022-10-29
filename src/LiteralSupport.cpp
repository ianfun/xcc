unsigned Lexer::read_pp_float(Location loc, const unsigned char *s, size_t len, double &o, size_t i, uintmax_t base, double f) {
    if (len == i)
        return lex_error(loc, "expect more digits after '.'"), 0;
    double e = 1.0;
    for(;;i++) {
        uintmax_t c = s[i];
        if (base == 16) {
            e /= 16.0;
            if (isdigit(c))
                f += (c - '0') * e;
            else if (c >= 'a' && c <= 'f')
                f += (c - 'a' + 10) * e;
            else if (c >= 'A' && c <= 'F')
                f += (c - 'A' + 10) * e;
            else
                break;
        } else {
            if (!isdigit(c))
                break;
            e /= 10.0;
            f += (c - '0') * e;
        }
        if (i == len)
            return o = f, 2;
    }
    char c = s[i];
    if (c == 'P' || c == 'p' || c == 'E' || c == 'e') {
        if (++i == len)
            return lex_error(loc, "expect exponent digits"), 0;        
        bool negate = false;
        if (s[i] == '-')
            negate = true, ++i;
        else if (s[i] == '+')
            ++i;
        if (i == len || !isdigit(s[i]))
            return parse_error(loc, "expect exponent digits"), 0;
        uintmax_t exp;
        for (exp=0;i != len && isdigit(s[i]);++i)
            exp = exp * 10 + (uintmax_t)s[i] - '0';
        if (negate)
            exp = 0 - exp;
        f *= pow(base == 10 ? 10.0 : 2.0, (double)exp);
    }
    unsigned res = 2;
    if (i != len) {
        unsigned c = s[i];
        switch (c) {
            case 'f':
                res = 3;
                break;
            case 'F':
                break;
            case 'L':
                break;
            case 'l':
                break;
            // TODO: complex float
            case 'i':
            case 'I':
                break;
            default:
                lex_error(loc, "invalid float suffix %C", c);
                return 0;
        }
    }
    o = f;
    return res;
}
// attempt to paser pp-number to int
// return value
//    0: <failed>
//    1: int
//    2: double
//    3: float
//    4: long
//    5: long long
//    6: unsigned long
//    7: unsigned long long
//    8: unsigned int

unsigned Lexer::read_pp_number(Location loc, xstring str, double &f, uintmax_t &n) {
    uintmax_t base = 10;
    uintmax_t num;
    size_t i = 0, len = str.size();
    auto s = (const unsigned char*)str.data();

    assert(len && "empty string is invalid number!");

    uintmax_t front = *s;
    if (front == '0') {
        if (len == 1)
            return 1 + (n = 0);
        uintmax_t c = s[1];
        switch (c) {
            case 'b': case 'B': 
                base = 2, i = 2;
                break;
            case 'x': case 'X':
                base = 16, i = 2;
                break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                i = 2;
                for (uintmax_t num = c - '0';;i++) {
                    uintmax_t c = s[i];
                    if (i == len)
                        return n = num, 1;
                    if (c < '0' || c > '7')
                        return lex_error(loc, "bad octal literal"), 0;
                    num = (num << 3) | (c - '0');
                }
                break;
            case '.':
                return read_pp_float(loc, s, len, f, 2, 10, 0.0);
            default:
                return lex_error(loc, "invalid number prefix: %C", c), 0;
        }
    }
    else if (front == '.')
        return read_pp_float(loc, s, len, f, 1, 10);
    else 
        assert(isdigit(front));

    for(num=0;i != len;++i) {
        uintmax_t c = s[i];
        if (base == 10){
            if (!isdigit(c))
                break;
            num = num * 10 + c - '0';
        }
        else if (base == 2)
            if (c == '0' || c == '1')
                num = num << 1 | (c - '0');
            else
                break;
        else // base == 16
            if (isdigit(c))
                num = (num << 4) | (c - '0' + 0);
            else if (c >= 'a' && c <= 'f')
                num = (num << 4) | (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F')
                num = (num << 4) | (c - 'A' + 10);
            else
                break;
    }

    if (i == len)
      return n = num, 1;
    else if (base == 2)
        return lex_error(loc, "binary number literal has no suffix: %C ", s[i]), 0;
    if (s[i] == '.')
        return read_pp_float(loc, s, len, f, i+1, base, num);
    unsigned result = 1;
    switch (s[i]) {
        case 'E': case 'e': case 'P': case 'p':
        {
            result = 2;
            if (++i == len)
                return lex_error(loc, "expect exponents"), 0;
            bool negate = false;
            if (s[i] == '-')
                negate = true, ++i;
            else if (s[i] == '+')
                ++i;
            else if (!isdigit(s[i]))
                return lex_error(loc, "expect exponent digits"), 0;
            uintmax_t exps;
            for (exps=0;isdigit(s[i]);)
                exps = exps * 10 + (uintmax_t)s[i] - '0';
            if (negate)
                exps = 0 - exps;
            f *= pow(base == 10 ? 10.0 : 2.0, (double)exps);
        }
        default:
            break;
    }
    for(;i < len;++i) {
        switch (s[i]) {
            case 'F': case 'f':
                if (result != 2 && result != 3)
                    return lex_error(loc, "invalid integer suffix: %C", s[i]), 0;
                result = 3;
                break;
            case 'L': case 'l':
                if (result != 2 && result != 3)
                    break;
                switch (result) {
                    case 1: // int => long
                        result = 4;
                    case 4: // long => long long
                        result = 5;
                    case 8: // unsigned int => unsigned long
                        result = 6;
                    case 6: // unsigned long => unsigned long long
                        result = 7;
                    default:
                        return lex_error(loc, "duplicate 'L' suffix in integer constant"), 0;
                }
                break;
            case 'U': case 'u':
                if (result == 2 || result == 3)
                    return lex_error(loc, "invalid float suffix: %C", s[i]), 0;
                switch (result) {
                    case 1:
                        result = 8;
                    case 4:
                        result = 6;
                    case 5:
                        result = 7;
                    default:
                        return lex_error(loc, "duplicate 'U' suffix in integer constant"), 0;
                }
                break;
            default:
                return lex_error(loc, result == 2 || result == 3 ? "invalid float suffix: %C" : "invalid integer suffix: %C", s[i]), 0;
        }
    }
    n = num;
    return result;
}
