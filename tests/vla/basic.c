void* vla(int N) {
	int array[N];
	return array + 1;
}
void * vla2(int N) {
	int array[N];
	return &array + 1;
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
