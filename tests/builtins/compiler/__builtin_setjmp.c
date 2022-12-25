extern int puts(const char *);

typedef unsigned long long intptr_t;

intptr_t buf[5];

void run() {
    // call void @llvm.eh.sjlj.longjmp(ptr @buf)
    return __builtin_longjmp((void**)&buf, 1);
}

int main() {
    // %2 = call ptr @llvm.frameaddress.p0(i32 0)
    // store ptr %2, ptr @buf, align 8
    // %3 = call ptr @llvm.stacksave()
    // store ptr %3, ptr getelementptr inbounds (ptr, ptr @buf, i32 2), align 8
    // @llvm.eh.sjlj.setjmp(ptr @buf)
    switch (__builtin_setjmp((void**)&buf)) { 
    case 0:
        puts("jumping");
        run();
    case 1:
        puts("jump back");
        break;
    default:
        puts("Error: __builtin_setjmp return unknown code");
        break;
    }
}
