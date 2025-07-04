#pragma once
#include "lexer.h"

typedef int64_t Word;

typedef enum ASTNodeType {
    AST_NODE_EXPRESSION_STATEMENT,

    AST_NODE_BINARY,
    AST_NODE_UNARY,
    AST_NODE_LITERAL,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        // expression statement
        struct ASTNode* expression;

        // binary operation
        struct {
            struct ASTNode* left;
            TokenType op;
            struct ASTNode* right;
        } binary;

        // unary operation
        struct {
            TokenType op;
            struct ASTNode* right;
        } unary;

        // literal value
        Word literal;
    };
} ASTNode;

ASTNode* parser_parse(const char* file_path, TokenArray* token_array);
void parser_free_ast(ASTNode* root);
void parser_print_output(ASTNode* root, int indent);
