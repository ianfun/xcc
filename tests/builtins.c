#pragma

_Pragma("");

const char *a = __func__; // ""
const char *b = __FUNCTION__; // ""
const char *c = __PRETTY_FUNCTION__; // "top level"
const char *e = __builtin_FUNCTION(); // ""

const char *compile_time() {
	return __TIME__;
}
const char *compile_date() {
	return __DATE__;
}
unsigned compile_line() {
	return __LINE__;
}
unsigned compile_line2() {
	return __builtin_LINE();
}
const char *compile_function() {
	return __func__;
}
const char *compile_function2() {
	return __FUNCTION__;
}
const char *compile_function3() {
	return __builtin_FUNCTION();
}
const char *compile_function_pretty() {
	return __PRETTY_FUNCTION__;
}
unsigned counter0() {
	return __COUNTER__;
}
unsigned counter1() {
	return __COUNTER__;
}
unsigned counter2() {
	return __COUNTER__;
}
unsigned counter3() {
	return __COUNTER__;
}
unsigned counter4() {
	return __COUNTER__;
}
unsigned counter5() {
	return __COUNTER__;
}
unsigned counter6() {
	return __COUNTER__;
}
unsigned counter7() {
	return __COUNTER__;
}
unsigned counter8() {
	return __COUNTER__;
}
