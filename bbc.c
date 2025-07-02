#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "utils.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <input.b>\n", argv[0]);
        fprintf(stderr, "error: input file not specified\n");
        exit(1);
    }

    char* source = file_read(argv[1]);

    TokenArray token_array = lexer_lex(source);

    int result = parser_parse(&token_array);
    printf("Result: %d\n", result);

    lexer_free_tokens(&token_array);
    free(source);
    return 0;
}
