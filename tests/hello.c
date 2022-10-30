int putchar(int);
int getchar();
int puts(char*);
int printf(char*, ...);
int main() {
	int i = 0;
	while (i < 100)
		printf("%d\n", i++);
	return i;
}
