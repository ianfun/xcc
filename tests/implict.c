int puts(const char*);
int getchar(void);

char* example() {
	char *ptr = getchar();
	puts(getchar());
	int c = &example;
	return (unsigned char*)0;
}

void const_Cast() {
	const int a = 0;
	int *p = &a; // drops const !
	p = &getchar;
}

