CC=clang++

#FLAGS=`llvm-config --cxxflags --ldflags --libs`
FLAGS=`/usr/local/llvm-config --cxxflags --system-libs --ldflags --libs all`
MOREFLAGS=-g
#  -lz -ldl -lpthread -pthread -ltinfo
# debug, zlib, dlopen/dlsym/dlclose, pthread, Terminfo

# assume you build llvm-projeect/LLD and `make install` in /usr/local directory, you may need to change
LLDFLAGS=-I /usr/local/include/ /usr/local/lib/liblldCOFF.a /usr/local/lib/liblldELF.a /usr/local/lib/liblldELF.a /usr/local/lib/liblldMinGW.a /usr/local/lib/liblldWasm.a /usr/local/lib/liblldMachO.a /usr/local/lib/liblldCommon.a

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


