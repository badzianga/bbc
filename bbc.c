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
    lexer_print_output(token_array);
    printf("----------------------------------------------------------------\n");
    
    ASTNode* ast = parser_parse(argv[1], &token_array);
    parser_print_output(ast, 0);
    printf("----------------------------------------------------------------\n");
    
    interpreter_interpret(ast);

    parser_free_ast(ast);
    lexer_free_tokens(&token_array);
    free(source);
    return 0;
}
