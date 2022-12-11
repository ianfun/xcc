// Program to test Imaginary number support in C.
// C23 Annex G - IEC 60559-compatible complex arithmetic

_Imaginary double a = 10j;
// @a =  global double 1.000000e+01
_Bool zero = 0j;
// @zero = global i1 false
_Bool non_zero = 1j;
// @non_zero = global i1 true

_Imaginary double imaginary_add1(_Imaginary double a, _Imaginary double b) {
	// %3 = fadd double %0, %1
	// ret double %3
	return a + b;
}
_Complex double imaginary_add2(_Imaginary double real, double imag) {
	// %3 = insertvalue { double, double } poison, double %0, 0
	// %4 = insertvalue { double, double } %3, double %1, 1
  	// ret { double, double } %4
	return real + imag;
}
_Complex double imaginary_add3(_Imaginary double real, double imag) {
	// %3 = insertvalue { double, double } poison, double %1, 0
	// %4 = insertvalue { double, double } %3, double %0, 1
	// ret { double, double } %4
        return imag + real;
}
_Imaginary double imaginary_sub1(_Imaginary double a, _Imaginary double b) {
	// %3 = fsub double %0, %1
	// ret double %3
        return a - b;
}
_Complex double imaginary_sub2(_Imaginary double real, double imag) {
	// %3 = fneg double %0
	// %4 = insertvalue { double, double } poison, double %3, 0
	// %5 = insertvalue { double, double } %4, double %1, 1
	// ret { double, double } %5
        return real - imag;
}
_Complex double imaginary_sub3(double real, double _Imaginary imag) {
	// %3 = fneg double %1
	// %4 = insertvalue { double, double } poison, double %3, 0
	// %5 = insertvalue { double, double } %4, double %0, 1
	// ret { double, double } %5
        return imag - real;
}
double imaginary_mul1(_Imaginary double a, _Imaginary double b) {
	return a * b;
	// %3 = fneg double %0
  	// %4 = fmul double %3, %1
  	// ret double %4
}
_Imaginary double imaginary_mul2(double a, _Imaginary double b) {
        return a * b; // %3 = fmul double %0, %1
        // ret double %3
}
_Imaginary double imaginary_mul3(_Imaginary double a, double b) {
        return a * b; // %3 = fmul double %1, %1
        // ret double %3
}
double imaginary_div1(_Imaginary double a, _Imaginary double b) {
        return a / b; // %3 = fdiv double %0, %1
  	// ret double %3
}
_Imaginary double imaginary_div2(_Imaginary double a, double b) {
        return a / b; // %3 = fdiv double %0, %1
        // ret double %3
}
_Imaginary double imaginary_div3(double a, _Imaginary double b) {
        return a / b; // %3 = fdiv double %0, %1
        // ret double %3
}
double get_complex_imaginary(_Complex double a) {
	// %.fca.0.extract = extractvalue { double, double } %0, 0
	// ret double %.fca.0.extract
	return a;
}
_Complex double complex_from_imaginary(_Imaginary double a) {
	// %2 = insertvalue { double, double } { double 0.000000e+00, double poison }, double %0, 1
	// ret { double, double } %2
	return a;
}
_Imaginary float imaginary_cast(_Imaginary int a) {
	// %2 = sitofp i32 %0 to float
	// ret float %2
	return a;
}
_Complex double complex_cast(_Imaginary float a) {
	// %2 = fpext float %0 to double
	// %3 = insertvalue { double, double } { double 0.000000e+00, double poison }, double %2, 1
	// ret { double, double } %3
	return a;
}
_Bool imaginary_to_bool(_Imaginary double a) {
	// %2 = fcmp one double %0, 0.000000e+00
	// ret i1 %2
	return a;
}
_Bool imaginary_eq(_Imaginary double a, _Imaginary double b) {
	// %3 = fcmp oeq double %0, %1
	// ret i1 %3
	return a == b;
}
double always_zero(_Imaginary double a) {
	// ret double 0.000000e+00
	return a;
}
_Imaginary double always_zero2(double a) {
	// ret double 0.000000e+00
	return a;
}
