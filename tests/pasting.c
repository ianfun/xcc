#define PASTE(a, b) a ## b

int test_1() {
    int ab = 10;
    return PASTE(a, b);
}

int test_2() {
    return PASTE(1, 0);
}

int test_3() {
    return 1 PASTE(<, <) 10;
}

#define N 1 ## 0

int test_4() {
    return N;
}
