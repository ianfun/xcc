// program to test imaginary numbers specfied in 
// C23 Annex G - IEC 60559-compatible complex arithmetic
/*
_Imaginary double a = 10j;
_Bool zero = 0j;
_Bool non_zero = 1j;

_Imaginary double imaginary_add1(_Imaginary double a, _Imaginary double b) {
	return a + b;
}
_Complex double imaginary_add2(_Imaginary double real, double imag) {
	return real + imag;
}
_Complex double imaginary_add3(_Imaginary double real, double imag) {
        return imag + real;
}
_Imaginary double imaginary_sub1(_Imaginary double a, _Imaginary double b) {
        return a - b;
}
_Complex double imaginary_sub2(_Imaginary double real, double imag) {
        return real - imag;
}
_Complex double imaginary_sub3(double real, double _Imaginary imag) {
        return imag - real;
}*/
double imaginary_mul1(_Imaginary double a, _Imaginary double b) {
	return a * b;
}/*
_Imaginary double imaginary_mul2(double a, _Imaginary double b) {
        return a * b;
}
_Imaginary double imaginary_mul3(_Imaginary double a, double b) {
        return a * b;
}
_Imaginary double imaginary_div1(_Imaginary double a, _Imaginary double b) {
        return a / b;
}
_Imaginary double imaginary_div2(double a, double b) {
        return a / b;
}
_Imaginary double imaginary_div3(_Imaginary double a, _Imaginary double b) {
        return a / b;
}
_Imaginary double get_complex_imaginary(_Complex double a) {
	return a;
}
_Complex double complex_from_imaginary(_Imaginary double a) {
	return a;
}
_Imaginary float imaginary_cast(_Imaginary int a) {
	return a;
}
_Complex double complex_cast(_Imaginary float a) {
	return a;
}
_Bool imaginary_to_bool(_Imaginary double a) {
	return a;
}
_Bool imaginary_eq(_Imaginary double a, _Imaginary double b) {
	return a == b;
}
double always_zero(_Imaginary double a) {
	return a;
}
_Imaginary double always_zero2(double a) {
	return a;
}
*/