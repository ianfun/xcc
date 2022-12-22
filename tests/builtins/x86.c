void my_pause() {
	// llvm.x86.sse2.pause
	return __builtin_ia32_pause();
}
void my_ptwrite(unsigned a) {
	// llvm.x86.ptwrite32.i32
	return __builtin_ia32_ptwrite32(a);
}

