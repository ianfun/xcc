_Bool old = 0;
_Bool new_style = false;
_Bool true_constant = true;

_Bool and_bool(bool a, bool b) {
	return a & b;
}
_Bool or_bool(bool a, bool b) {
	return a | b;
}
int bool_as_int(bool a) {
	return a;
}
float bool_as_float(bool a) {
	return a;
}
_Bool bool_from_int(int a) {
	return a;
}
int warn_bool_in_switch(_Bool a) {
	switch (a) {
		case true:  return 1;
		case false: return 0;
	}
}

