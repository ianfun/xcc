static __inline int add(int a, int b) {
	return a + b;
}
double get_complex_real(__complex double a) {
	return __real a;
}
double get_complex_real2(_Complex double a) {
        return __extension__ __real__ a;
}


__const unsigned A = __alignof__(int);
__const__ unsigned B = alignof(int);
__volatile unsigned C = __alignof(int);

asm("\t");
__asm__("\t");
asm("\t");

__typeof__(int) D;
__typeof__(+((char)'C')) E;

static_assert(1, "This should not print.");

__signed__ int *__restrict Ptr;
