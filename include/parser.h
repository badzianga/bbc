#pragma once
#include "lexer.h"

typedef int64_t Word;

typedef enum ASTNodeType {
    AST_NODE_BINARY,
    AST_NODE_UNARY,
    AST_NODE_LITERAL,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        struct {
            struct ASTNode* left;
            TokenType op;
            struct ASTNode* right;
        } binary;

        struct {
            TokenType op;
            struct ASTNode* right;
        } unary;

        Word literal;
    };
} ASTNode;

ASTNode* parser_parse(TokenArray* token_array);
void parser_free_ast(ASTNode* root);
void parser_print_output(ASTNode* root, int indent);
