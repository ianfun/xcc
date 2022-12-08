#define DECL static int xxx = 0;

DECL

#define DECL2(name) static int name = 0;

DECL2(foo)

DECL2(bar)
