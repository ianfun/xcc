unsigned long long do_strlen(const char *s) {
    return __builtin_strlen(s);
}

unsigned long long a = __builtin_strlen("123"); // 3
