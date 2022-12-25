#define DO(X) \
double do_##X(double a) { \
    return __builtin_##X(a) + __builtin_##X##l(a) + __builtin_##X##f(a);\
}

DO(sin)
DO(cos)
DO(tan)
DO(exp)

double one = __builtin_cos(0.0);
double zero = __builtin_tan(0.0);
double three = __builtin_log2(8);
double four = __builtin_sqrt(__builtin_pow(5, 2) - __builtin_pow(3, 2));
double my_nan = __builtin_nan("");
double my_inf = __builtin_inf();
_Complex double C = __builtin_complex(__builtin_log10(1e4), __builtin_log10(1e5));

_Complex double do_csqrt(_Complex double a) {
    return __builtin_csqrt(a);
}
double do_real(_Complex double a) {
    return __builtin_creal(a) + __builtin_cimag(a);
}
double do_cabs(_Complex double a) {
    return __builtin_cabs(a);
}
