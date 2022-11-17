_Complex double c_add(_Complex double a, _Complex double b) {
	return a + b;
}
_Complex double c_sub(_Complex double a, _Complex double b) {
        return a - b;
}
/*
_Complex double c_mul(_Complex double a, _Complex double b) {
        return a * b;
}*/
/*
_Complex double c_div(_Complex double a, _Complex double b) {
        return a / b;
}*/
_Complex double c_conj(_Complex double a) {
        return ~a;
}
_Complex double c_neg(_Complex double a) {
        return -a;
}
_Complex double c_eq(_Complex double a, _Complex double b) {
        return a == b;
}
_Complex double c_ne(_Complex double a, _Complex double b) {
        return a != b;
}

