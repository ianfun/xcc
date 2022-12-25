int do_popcount(unsigned a) {
    return __builtin_popcount(a);
}

int a = __builtin_popcount(0xFF);
