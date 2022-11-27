void *int_to_ptr_small(char c) {
	return (void*)c;
}
char ptr_to_int_small(void *p) {
	return (char)p;
}
int puts(const char *str);

void bad_argument(unsigned char *p) {
	puts(p);
}
