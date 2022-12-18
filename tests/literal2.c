const char *hello_world = "Hello world!";
const char *world_hello = *&"!dlrow olleH";
const char *intern_1 = "intern";
const char *intern_2 = "intern";

typedef typeof("") string;
typedef typeof(u8"") utf8;
typedef typeof(u"") utf13;
typedef typeof(U"") utf32;
typedef typeof(L"") wide_string;

typedef typeof('A') c_int;
typedef typeof(+'A') c_int2;

typedef typeof(false) c_bool;
typedef typeof(true) c_bool2;

typedef typeof(nullptr) c_nullptr_t;
typedef typeof((void*)0) c_void_ptr;
