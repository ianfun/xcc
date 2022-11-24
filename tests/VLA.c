typedef unsigned long long size_t;
extern int use_array(void *ptr, size_t size);
extern int getchar();

void test(unsigned n) {
	for (unsigned i = 0;i < n;++i) {
		int c = getchar();
		typedef int VLATy[c];
BEGIN: ;
		VLATy array;
		switch (getchar()) {
			default: break;
			case 0: goto BEGIN;
			case 1: goto END;
		}
		use_array(array, c)
	}
END: ;
}

