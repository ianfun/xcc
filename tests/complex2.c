// program to test Complex number support
// runtime complex arithmetic
_Complex double c_add(_Complex double a, _Complex double b) {
	return a + b;
}

_Complex double c_sub(_Complex double a, _Complex double b) {
        return a - b;
}

_Complex double c_mul(_Complex double a, _Complex double b) {
        return a * b;
}

_Complex double c_div(_Complex double a, _Complex double b) {
        return a / b;
}

_Complex double c_conj(_Complex double a) {
        return ~a;
}
_Complex double c_neg(_Complex double a) {
        return -a;
}

_Bool c_eq(_Complex double a, _Complex double b) {
        return a == b;
}
_Bool c_ne(_Complex double a, _Complex double b) {
        return a != b;
}
_Complex double eq_and_cast(_Complex double a, _Complex double b) {
        return a == b;
}
double get_real(_Complex double a) {
	return __real a;
}
double get_imag(_Complex double a) {
	return __extension__  __imag a;
}
double the_same(double a) {
	return __real a;
}
double zero(double a) {
	return __imag a;
}
