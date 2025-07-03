#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
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

    // TODO: move token_array from main to parser
    TokenArray token_array = lexer_lex(source);
    ASTNode* ast = parser_parse(argv[1], &token_array);
    Word result = interpreter_interpret(ast);

    lexer_print_output(token_array);
    printf("----------------------------------------------------------------\n");
    parser_print_output(ast, 0);
    printf("----------------------------------------------------------------\n");
    printf("Result: %ld\n", result);

    parser_free_ast(ast);
    lexer_free_tokens(&token_array);
    free(source);
    return 0;
}
