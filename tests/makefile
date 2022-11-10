CC=clang++

FLAGS=-I/usr/lib/llvm-15/include -std=c++14   -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-15/lib -lLLVM-15
MOREFLAGS=-g -lz -ldl -lpthread -pthread -ltinfo
# debug, zlib, dlopen/dlsym/dlclose, pthread, Terminfo

# assume you build llvm-projeect/LLD and `make install` in /usr/local directory, you may need to change
LLDFLAGS= \
-L/usr/local/lib \
-llldCOFF \
-llldCommon \
-llldELF \
-llldMachO \
-llldMinGW \
-llldWasm

mainNoLLD:
	$(CC) main.cpp $(FLAGS) -o xcc -g

main:
	$(CC) -DCC_HAS_LLD  main.cpp $(FLAGS) -o xcc $(MOREFLAGS) $(LLDFLAGS)

testCodeGen:
	$(CC) testCodeGen.cpp $(FLAGS) -o codegen $(MOREFLAGS)

testCpp:
	$(CC) testCpp.cpp $(FLAGS) -o cpp $(MOREFLAGS)

testLexer:
	$(CC) testLexer.cpp $(FLAGS) -o lexer $(MOREFLAGS)

testParser:
	$(CC) testParser.cpp $(FLAGS) -o parser $(MOREFLAGS)

testJit:
	$(CC) testJit.cpp $(FLAGS) -o jit $(MOREFLAGS)

all: testLexer testCpp testParser testCodeGen testJit

clean:
	rm lexer cpp parser codegen jit xcc


