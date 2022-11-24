// compile-time constant complex arithmetic

const _Complex double a = 2 + 4i; // a = 2 + 4i
const _Complex double b = 1 + 2i; // b = 1 + 2i

_Bool toBool1 = a; // true
_Bool toBool2 = b; // true
_Bool toBool3 = 1j; // true
_Bool toBool4 = 3 + 4j; // true
_Bool toBool5 = 0j; // false
_Bool toBool6 = 0 + 0j; // false

_Complex double A = a + b; // 3 + 6i
_Complex double B = a - b; // 1 + 2i
_Complex double C = a * b;
_Complex double D = a / b;

_Bool eq = a == b; // false
_Bool ne = a != b; // true

_Complex int toComplexInt = a;
_Complex unsigned toComplexUnsigned = b;

double toReal = a;
int toInt = b;

_Complex float toComplexFloat = a;
_Complex long double toComplexLongDouble = b;

_Complex double unaryNeg = -a;
_Complex double unaryConj = ~b;

_Complex int poison_pair = (_Complex int)0i / (_Complex int)0i;
_Complex double neg_nan_pair = (_Complex double)0i / (_Complex double)0i;


