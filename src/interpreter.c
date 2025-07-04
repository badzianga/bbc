#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

#define INTERPRET_EXPRESSION_STATEMENT(node) interpreter_interpret((node)->expression)
#define INTERPRET_PRINT_STATEMENT(node) printf("Printed: %ld\n", interpreter_interpret((node)->expression))
#define INTERPRET_BINARY(node, op) interpreter_interpret((node)->binary.left) op interpreter_interpret((node)->binary.right)
#define INTERPRET_UNARY(node, op) op interpreter_interpret((node)->unary.right)
#define INTERPRET_LITERAL(node) (node)->literal

Word interpreter_interpret(ASTNode* root) {
    switch (root->type) {
        case AST_NODE_EXPRESSION_STATEMENT: {
            return INTERPRET_EXPRESSION_STATEMENT(root);
        }
        case AST_NODE_PRINT_STATEMENT: {
            return INTERPRET_PRINT_STATEMENT(root);
        }
        case AST_NODE_BINARY: {
            switch (root->binary.op) {
                case TOKEN_SLASH:
                    return INTERPRET_BINARY(root, /);
                case TOKEN_ASTERISK:
                    return INTERPRET_BINARY(root, *);
                case TOKEN_PERCENT:
                    return INTERPRET_BINARY(root, %);
                case TOKEN_PLUS:
                    return INTERPRET_BINARY(root, +);
                case TOKEN_MINUS:
                    return INTERPRET_BINARY(root, -);
                case TOKEN_EQUAL_EQUAL:
                    return INTERPRET_BINARY(root, ==);
                case TOKEN_NOT_EQUAL:
                    return INTERPRET_BINARY(root, !=);
                case TOKEN_GREATER:
                    return INTERPRET_BINARY(root, >);
                case TOKEN_GREATER_EQUAL:
                    return INTERPRET_BINARY(root, >=);
                case TOKEN_LESS:
                    return INTERPRET_BINARY(root, <);
                case TOKEN_LESS_EQUAL:
                    return INTERPRET_BINARY(root, <=);
                default: {
                    fprintf(stderr, "error: invalid token in binary operation: %d\n", root->binary.op);
                    exit(1);
                }
            }
        }
        case AST_NODE_UNARY: {
            switch (root->unary.op) {
                case TOKEN_MINUS:
                    return INTERPRET_UNARY(root, -);
                case TOKEN_NOT:
                    return INTERPRET_UNARY(root, !);
                default: {
                    fprintf(stderr, "error: invalid token in unary operation: %d\n", root->unary.op);
                    exit(1);
                }
            }
        }
        case AST_NODE_LITERAL: {
            return INTERPRET_LITERAL(root); 
        }
        default: {
            fprintf(stderr, "error: unknown AST node to interpret: %d\n", root->type);
            exit(1);
        } break;
    }
    return 0;
}
