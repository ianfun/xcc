int test() {
	return 0;
	return 123;
}
extern void _Noreturn exit(int status);

int test2() 
{
	exit(0);
	return 0;
}
