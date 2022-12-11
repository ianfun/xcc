typedef unsigned long long size_t;

typedef typeof_unqual(nullptr) nullptr_t; // declare nullptr_t as typedef nullptr_t
void *p = nullptr;

nullptr_t null_ptr; // declare null_ptr as nullptr_t

extern void *memcpy (
	void *__restrict __dest, 
	const void *__restrict __src,
        size_t __n
);

extern void *malloc(size_t size);
extern void free(void *Ptr);

void* test() {
	p = malloc(10);
	memcpy(p, "nullptr_t", 10);
	free(p);
	return nullptr;
}
