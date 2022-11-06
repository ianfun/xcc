#include <cstdint>
#include <cstdio>

struct xint128_t {
    //       high --- low
    // index: 0, 1, 2, 3
    uint32_t m32[4];
    void dump() const { printf("%13d\t%13d\t%13d\t%13d\n", m32[0], m32[1], m32[2], m32[3]); }
    operator bool() const { return m32[0] || m32[1] || m32[2] || m32[3]; }
    uint64_t high() const { return (static_cast<uint64_t>(m32[0]) << 32) | m32[1]; }
    void setHigh(uint64_t v) {
        m32[0] = static_cast<uint32_t>(v >> 32);
        m32[1] = static_cast<uint32_t>(v);
    }
    void setLow(uint64_t v) {
        m32[2] = static_cast<uint32_t>(v >> 32);
        m32[3] = static_cast<uint32_t>(v);
    }
    uint64_t low() const { return (static_cast<uint64_t>(m32[2]) << 32) | m32[3]; }
    xint128_t operator>>(unsigned shift) const {
        if (shift < 64) {
            return xint128_t::get((low() >> shift) | high() >> (64 - shift), high() >> shift);
        }
        return xint128_t::get(high() >> (shift - 64), 0);
    }
    static xint128_t getZero() {
        xint128_t res;
        res.setHigh(0);
        res.setLow(0);
        return res;
    }
    static xint128_t get(uint64_t low = 0, uint64_t high = 0) {
        xint128_t res;
        res.m32[3] = low;
        res.m32[2] = low >> 32;
        res.m32[1] = high;
        res.m32[0] = high >> 32;
        return res;
    }
};
static bool uadd_overflow(xint128_t &res, uint32_t b) {
    return ((res.m32[3] += b) < b) && (!++res.m32[2]) && (!++res.m32[1]) && (!++res.m32[0]);
};
static bool umul_overflow(xint128_t a, uint32_t b, xint128_t &res) {
    const uint64_t I = b;
    uint64_t carry;
    uint64_t z1 = a.m32[0] * I, z2 = a.m32[1] * I, z3 = a.m32[2] * I, z4 = a.m32[3] * I;
    res.m32[3] = z4;
    carry = (res.m32[2] = z3 + (z4 >> 32)) < z3;
    carry = (res.m32[1] = z2 + (z3 >> 32)) < z2;
    carry = (res.m32[0] = z1 + (z2 >> 32)) < z1;
    return carry;
}
