float half_add(unsigned a) {
	volatile __fp16 fp = a;
	fp++;
	fp = -fp;
	fp *= fp;
	fp /= fp;
	return fp;
}

