enum Command {
    NotACommand,
    Help,
    Quit
};
template <typename char_type> struct TinyCommandParser {
    static void fmt(const char *msg, ...) {
        va_list ap;
        va_start(ap, msg);
        vfprintf(stderr, msg, ap);
        va_end(ap);
    }
    static void raw(const char *msg) { fputs(msg, stderr); }
    void token() {
        while (pos < max && (s[pos] == ' ' || s[pos] == '\t')) {
        }
    }
    static inline int xisdigit(unsigned c) { return c >= '0' && c <= '9'; }
    static inline int xisalpha(unsigned c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '$';
    }
    TinyCommandParser(const char_type *s, size_t len) : s{s}, pos{0}, max{len} {};
    const char_type *s;
    size_t pos, max;
    void readIdent(std::basic_string<char_type> &result) {
        token();
        while (pos < max && xisalpha(s[pos])) {
            result += s[pos++];
        }
    }
    int isChar(char_type c) {
        token();
        return pos < max && s[pos] == c;
    }
    long long readNumber(bool &success) {
        bool neg = false;
        unsigned long long u = 0;

        token();
        success = false;
        if (max == pos)
            return 0;
        if (isdigit(s[pos])) {
READ:
            do {
                u = u * 10 + (unsigned long long)s[pos++] - '0';
            } while (pos < max && xisdigit(s[pos]));
            if (neg) {
                u = 0 - u;
            }
            return u;
        }
        if (s[pos] == '+') {
            goto READ;
        }
        if (s[pos] == '-') {
            neg = true;
            goto READ;
        }
        return 0;
    }
};
