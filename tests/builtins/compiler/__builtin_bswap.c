int do_swap(unsigned a) {
    return __builtin_bswap16(a);
}
int b = __builtin_bswap32(0xFF);
int c = __builtin_bswap64(1);
