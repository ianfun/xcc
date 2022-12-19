int getchar();

struct {

};

struct Foo {
   struct Bar {
      int s;
   };
};

int test(){
    struct Foo {
       int arr[getchar()];
       int next;
    };
    struct Foo a;
    a.next = 10;
    return a.next + sizeof(a);
}

// fields must have a constant size: 'variable length array in structure' extension will never be supported

