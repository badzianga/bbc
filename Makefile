bbc: bbc.c include/lexer.h src/lexer.c include/utils.h src/utils.c
	gcc -Iinclude bbc.c src/lexer.c src/utils.c -o bbc
