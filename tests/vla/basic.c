void* vla(int N) {
	int array[N];
	return array + 1;
}
void * vla2(int N) {
	int array[N];
	return &array + 20;
}
void *array() {
	int arr[5];
	return arr + 1;
}
void *array2() {
	int arr[5];
	return &arr + 1;
}
unsigned long long vla3(int N) {
	int array[N];
	return sizeof(array);
}
unsigned index(unsigned a) {
	int vla[a];
	return vla[a - 1];
}
unsigned long long diff(unsigned a) {
	int vla[a];
	return (&vla + 1) - &vla;
}
