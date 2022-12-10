void test(unsigned n) {
	{
		int vla[n];
L:;
	}
	goto L;
}

