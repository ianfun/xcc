/*void bad() {
	void *a;
	goto *a;
}*/
void forever() {
L: ;
	void *p = &&L;
	void *p2;
	p2 = (void*)121;
	goto *p;	
}

