#include <cstdint>
#include <cstdio>

union xint128_t {
    //       high --- low
    // index: 0, 1, 2, ...
    uint8_t m8[16];
    uint32_t m32[4];
    uint64_t m64[2];
    void dump() {
        printf("\n%s\n", "raw uint8_t data");
        for (size_t i = 0;i < 16;i++) {
            printf("%u\t", m8[i]);
        }
        printf("\n%s\n", "raw uint32_t data");
        for (size_t i = 0;i < 4;i++) {
            printf("%u\t", m32[i]);
        }
        printf("\n%s\n", "raw uint64_t data");
        for (size_t i = 0;i < 2;i++) {
            printf("%lu\t", m64[i]);
        }
        printf("\n");
    }
    static xint128_t get(uint64_t low = 0, uint64_t high = 0) {
        xint128_t res;
        res.m64[0] = low;
        res.m64[1] = high;
        return res;
    }
};
static bool uadd_overflow(xint128_t &res, uint64_t b) {
    if ((res.m64[1] += b) < b) {
        if ((++res.m64[0]) == 0)
            return true;
    }
    return false;
};
static bool uadd_overflow(xint128_t a, xint128_t b, xint128_t &res) {
    bool overflow = false;
    res.m64[0] = a.m64[0] + b.m64[0];
    res.m64[1] = a.m64[1] + b.m64[1];
    if (res.m64[1] < a.m64[1]) {
        if (++res.m64[0] == 0)
            overflow = true;
    }
    if (res.m64[0] < a.m64[0])
        overflow = true;
    return overflow;
};
static bool umul_overflow(xint128_t a, uint32_t b, xint128_t &res) {
    uint64_t i = b;
    bool overflow = false;
    uint64_t 
      z1 = (uint64_t)a.m32[0] * i,
      z2 = (uint64_t)a.m32[1] * i,
      z3 = (uint64_t)a.m32[2] * i,
      z4 = (uint64_t)a.m32[3] * i;
    if (z4 >> 32)
        overflow = true;
    res.m32[0] = z1;
    res.m32[1] = (z1 >> 32) + z2;
    res.m32[2] = (z2 >> 32) + z3;
    res.m32[3] = (z3 >> 32) + z4;
    if (res.m32[3] < z4)
        overflow = true;
    return overflow;
}
