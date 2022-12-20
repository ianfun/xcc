struct { unsigned : 1; unsigned char x: 7;  } A;
struct { unsigned : 1; unsigned short x: 10;  } B;
struct { unsigned : 1; unsigned int x: 30;  } C;
struct { unsigned : 1; unsigned long long x: 60;  } D;

/*

get bit-field:
	%ptr = getelementptr ...
	%a = load %ptr
	%b = lshr a, (padding bits) - move left/right for other bits
	%c = and b, (all ones value, width=bit field width - padding bits) - clear old part except bit field part
	ret %c

*/

unsigned char get_A() { return A.x; }
unsigned short get_B() { return B.x; }
unsigned int get_C() { return C.x; }
unsigned long long get_D() { return D.x; }

/*

set bit-field(%new_value):
	%ptr = getelementptr ...
	%old_value = load %ptr
	%a = and %new_value, (all ones value, width=bit field width) - clear high bits
	%b = shl %a, (padding bits) - move left/right for other bits
	%c = and %old_value, (all ones value, width=padding bits) - Save the old value except bit field part
	%d = or %b, %c - merge them
	store %d, %ptr - store back new value

*/

void set_A(unsigned char x) { A.x = x; }
void set_B(unsigned short x) { B.x = x; }
void set_C(unsigned int x) { C.x = x; }
void set_D(unsigned long long x) { D.x = x; }

