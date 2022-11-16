# C++ cmopiler
CC=clang++

# include files
CXXINCS=-I/usr/include/c++/9 -I/usr/include/x86_64-linux-gnu/c++/9 -I/usr/lib/gcc/x86_64-linux-gnu/9/include

# LLVM C++ flags
CXXFLAGS=-I/usr/lib/llvm-15/include -std=c++14 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

# change this to -c if you want to skip linking
# Flags to link with LLVM libary
LDFLAGS=-L/usr/lib/llvm-15/lib -lLLVM-15 -lz -ldl -lpthread -pthread -ltinfo
# zlib, dlopen/dlsym/dlclose, pthread, Terminfo

# Flags to link with LLD libary
# assume you build llvm-project/LLD and `make install` in /usr/local directory
LLDFLAGS= \
-L/usr/local/lib \
-llldCOFF \
-llldCommon \
-llldELF \
-llldMachO \
-llldMinGW \
-llldWasm

# debug on/off
DEBUG=-g -Wextra -Wno-unused-parameter -Wno-c++20-extensions

# XCC driver with LLD disabled
mainNoLLD:
	$(CC) main.cpp $(CXXINCS) $(CXXFLAGS) -o xcc $(DEBUG) $(LDFLAGS) $(LLDFLAGS)

# XCC driver with LLD enabled
main:
	$(CC) -DCC_HAS_LLD main.cpp $(CXXINCS) $(CXXFLAGS) $(DEBUG) -o xcc $(LLDFLAGS)

# test code generation
testCodeGen:
	$(CC) $(CXXINCS) testCodeGen.cpp $(DEBUG) $(CXXFLAGS) -o codegen $(LDFLAGS)

# test C preprocessor
testCpp:
	$(CC) $(CXXINCS) testCpp.cpp $(DEBUG) $(CXXFLAGS) -o cpp $(LDFLAGS)

# test C lexer
testLexer:
	$(CC) $(CXXINCS) testLexer.cpp $(DEBUG) $(CXXFLAGS) -o lexer $(LDFLAGS)

# test C parser
testParser:
	$(CC) $(CXXINCS) testParser.cpp $(DEBUG) $(CXXFLAGS) -o parser $(LDFLAGS)

# test LLVM's Just-In-Time compilation
testJit:
	$(CC) $(CXXINCS) testJit.cpp $(DEBUG) $(CXXFLAGS) -o jit $(LDFLAGS)

# make all, you may run with -j to make it faster
all: testLexer testCpp testParser testCodeGen testJit main mainNoLLD

# clean outputs
clean:
	rm -f lexer cpp parser codegen jit xcc *.out *.o *.ll


