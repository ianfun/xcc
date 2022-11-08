CC=clang++

FLAGS=`llvm-config --cxxflags --ldflags --libs`
MOREFLAGS='-g'

main:
	$(CC) main.cpp $(FLAGS) -o xcc $(MOREFLAGS)

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


