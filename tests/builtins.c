int puts(const char*);
int printf(const char*, ...);


const char *a = __func__; // ""
const char *b = __FUNCTION__; // ""
const char *c = __PRETTY_FUNCTION__; // "top level"
const char *e = __builtin_FUNCTION(); // ""

void example() {
	puts(__func__);
	puts(__FUNCTION__);
	puts(__builtin_FUNCTION());
	puts(__builtin_LINE());
	puts(__builtin_FILE());
	puts(__LINE__);
	puts(__FILE__);
	puts(__TIME__);
	puts(__DATE__);
}


