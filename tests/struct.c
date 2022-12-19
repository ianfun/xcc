struct Empty {};

struct Single {
	int data;
};

struct Pair {
	int first;
	int second;
};

struct Outer {
	int a;
	struct {
		int b;
		struct {
			int c;
			struct { int data; int data2; } arr[50];
		};
		int d;
	};
	int e;
};

/*
// test global constant initialization
struct Empty a = {};
struct Single b = {1};
struct Single c = {.data = 1};
struct Pair d = {.first = 1, 2};
struct Pair e = {1, 2};
struct Pair f = {.first = 1, .second = 2};
struct Outer g = {};
struct Outer h = {.a = 1, .b = 2, .c = 3, .arr[0].data = 4, .arr[1] = {5}, .arr[2 ... 20].data = 6, .arr[5 ... 30].data2 = 7, .d = 8, .e = 9};
*/

// test local initialization
void test() {
	struct Empty a = {};
	struct Single b = {1};
	struct Single c = {.data = 1};
	struct Pair d = {.first = 1, 2};
	struct Pair e = {1, 2};
	struct Pair f = {.first = 1, .second = 2};
	struct Outer g = {};
	struct Outer h = {.a = 10, .b = 11., .c = 12, .d = 13, .e = 14};
/*
	struct Outer i = {.a = 1, .b = 2, .c = 3, .arr[0].data = 4, .arr[1] = {5}, .arr[2 ... 20].data = 6, .arr[5 ... 30].data2 = 7, .d = 8, .e = 9};
	*/
}

