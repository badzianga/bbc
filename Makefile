all: bbc

bbc: bbc.c include/lexer.h src/lexer.c include/utils.h src/utils.c include/parser.h src/parser.c include/interpreter.h src/interpreter.c
	gcc -Iinclude -ggdb bbc.c src/lexer.c src/parser.c src/utils.c src/interpreter.c -o bbc

clean:
	rm -fr bbc

.PHONY: all clean
