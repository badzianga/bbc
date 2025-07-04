#pragma once
#include "lexer.h"

typedef int64_t Word;

typedef enum ASTNodeType {
    AST_NODE_PROGRAM,
    AST_NODE_EXPRESSION_STATEMENT,
    AST_NODE_PRINT_STATEMENT,  // TODO: TEMPORARY
    AST_NODE_VARIABLE_DECLARATION,

    AST_NODE_ASSIGNMENT,
    AST_NODE_BINARY,
    AST_NODE_UNARY,
    AST_NODE_LITERAL,
    AST_NODE_VARIABLE,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        // program
        struct {
            struct ASTNode** statements;
            int count;
            int capacity;
        } program;

        // expression
        struct ASTNode* expression;

        // variable and declaration
        char* name;

        // assignment
        struct {
            char* name;
            struct ASTNode* value;
        } assignment;

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
