CC=clang++

FLAGS=`llvm-config --cxxflags --ldflags --libs` -g

testCodeGen:
	$(CC) testCodeGen.cpp $(FLAGS) -o codegen

testCpp:
	$(CC) testCpp.cpp $(FLAGS) -o cpp

testLexer:
	$(CC) testLexer.cpp $(FLAGS) -o lexer

testParser:
	$(CC) testParser.cpp $(FLAGS) -o parser

testJit:
	$(CC) testJit.cpp $(FLAGS) -o jit

all: testLexer testCpp testParser testCodeGen testJit

clean:
	rm lexer cpp parser codegen jit


