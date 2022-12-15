# sudo apt install llvm-15 liblld-15 liblld-15-dev
LLVM_OBJ_ROOT=/usr/lib/llvm-15/build/Debug+Asserts/lib
LLVM_INCLUDE=-I/usr/lib/llvm-15/build/Debug+Asserts/include
CXX_INCS=-I/usr/lib/gcc/x86_64-linux-gnu/9/include -I/usr/local/include
# LLD is optional
LLD_ROOT=/usr/local/lib
# C++ cmopiler
CC=clang++
# LLVM C++ flags
CXXFLAGS=$(LLVM_INCLUDE) -std=c++14 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
# change this to -c if you want to skip linking
# Flags to link with LLVM libary
LDFLAGS='-L$(LLVM_OBJ_ROOT)' -lLLVM-15 -lz -ldl -lpthread -pthread -ltinfo
# zlib, dlopen/dlsym/dlclose, pthread, Terminfo

# Flags to link with LLD libary
# assume you build llvm-project/LLD and `make install` in /usr/local directory
LLDFLAGS_STATIC= \
-L$(LLD_ROOT) \
-lz -ldl -lpthread -pthread -ltinfo \
/usr/local/lib/*.a

# debug on/off
DEBUG=-g -Wextra -Wno-unused-parameter -Wno-c++20-extensions # -fsanitize=address

# XCC driver with LLD disabled
mainNoLLD:
	$(CC) main.cpp $(CXXINCS) $(CXXFLAGS) -o xcc $(DEBUG) $(LDFLAGS)

# XCC driver with LLD enabled
main:
	$(CC) -DCC_HAS_LLD main.cpp $(CXXINCS) $(CXXFLAGS) $(DEBUG) -o xcc $(LLDFLAGS_STATIC)

# test code generation
testCodeGen:
	$(CC) $(CXXINCS) testCodeGen.cpp $(DEBUG) $(CXXFLAGS) -o codegen $(LDFLAGS)

# test C preprocessor
testCpp:
	$(CC) $(CXXINCS) testCpp.cpp $(DEBUG) $(CXXFLAGS) -o cpp $(LDFLAGS)

# test C lexer
testLexer:
	$(CC) $(CXXINCS) testLexer.cpp $(DEBUG) $(CXXFLAGS) -o lexer $(LDFLAGS)

# test CType
testCType:
	$(CC) $(CXXINCS) testCType.cpp $(DEBUG) $(CXXFLAGS) -o ctype $(LDFLAGS)
	echo "> running test program"
	./ctype

# test C parser
testParser:
	$(CC) $(CXXINCS) testParser.cpp $(DEBUG) $(CXXFLAGS) -o parser $(LDFLAGS)

# test Interpreter
testInter:
	$(CC) $(CXXINCS) testInterpreter.cpp $(DEBUG) $(CXXFLAGS) -o interpreter $(LDFLAGS)

# test printing Graph to dot file
testGraph:
	$(CC) $(CXXINCS) testGraph.cpp $(DEBUG) $(CXXFLAGS) -o graph $(LDFLAGS)

# test LLVM's Just-In-Time compilation
testJit:
	$(CC) $(CXXINCS) testJit.cpp $(DEBUG) $(CXXFLAGS) -o jit $(LDFLAGS)

# make all, you may run with -j to make it faster
all: testLexer testCpp testParser testCodeGen testJit main mainNoLLD testInter

runTest: testCpp testCodeGen
	./codegen complex.c
	./codegen imaginary.c
	./codegen bool.c
	./codegen complex2.c
	./codegen xc.c
	./cpp trigraphs.c
	clang example.ll
	./a.out hello.c

# clean outputs
clean:
	rm -f lexer cpp parser codegen jit xcc *.out *.o *.ll

