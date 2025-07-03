#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

typedef struct s_parser {
    const char* file_path;
    Token* tokens;
    Token* current;
    int count;
} Parser;

static Parser parser;

static ASTNode* make_node_binary(ASTNode* left, TokenType op, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NODE_BINARY;
    node->binary.left = left;
    node->binary.op = op;
    node->binary.right = right;
    return node;
}

static ASTNode* make_node_unary(TokenType op, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NODE_UNARY;
    node->unary.op = op;
    node->unary.right = right;
    return node;
}

static ASTNode* make_node_literal(Word value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NODE_LITERAL;
    node->literal = value;
    return node;
}

static ASTNode* parse_expression();
static ASTNode* parse_equality();
static ASTNode* parse_comparison();
static ASTNode* parse_term();
static ASTNode* parse_factor();
static ASTNode* parse_unary();
static ASTNode* parse_primary();

// TODO: to delete
static ASTNode* parse_line() {
    ASTNode* result = parse_expression();
    if (parser.current->type != TOKEN_SEMICOLON) {
        fprintf(stderr, "%s:%d: error: expected ';' after expression\n", parser.file_path, parser.current->line);
        exit(1);
    }
    ++parser.current;
    return result;
}

static ASTNode* parse_expression() {
    return parse_equality();
}

static ASTNode* parse_equality() {
    ASTNode* left = parse_comparison();

    TokenType op = parser.current->type;
    while (op == TOKEN_EQUAL_EQUAL || op == TOKEN_NOT_EQUAL) {
        ++parser.current;
        ASTNode* right = parse_comparison();
        left = make_node_binary(left, op, right);
        op = parser.current->type;
    }
    return left;
}

static ASTNode* parse_comparison() {
    ASTNode* left = parse_term();

    TokenType op = parser.current->type;
    while (op == TOKEN_GREATER || op == TOKEN_GREATER_EQUAL || op == TOKEN_LESS || op == TOKEN_LESS_EQUAL) {
        ++parser.current;
        ASTNode* right = parse_term();
        left = make_node_binary(left, op, right);
        op = parser.current->type;
    }
    return left;
}

static ASTNode* parse_term() {
    ASTNode* left = parse_factor();

    TokenType op = parser.current->type;
    while (op == TOKEN_PLUS || op == TOKEN_MINUS) {
        ++parser.current;
        ASTNode* right = parse_factor();
        left = make_node_binary(left, op, right);
        op = parser.current->type;
    }
    return left;
}

static ASTNode* parse_factor() {
    ASTNode* left = parse_unary();

    TokenType op = parser.current->type;
    while (op == TOKEN_SLASH || op == TOKEN_ASTERISK || op == TOKEN_PERCENT) {
        ++parser.current;
        ASTNode* right = parse_unary();
        left = make_node_binary(left, op, right);
        op = parser.current->type;
    }
    return left;
}

static ASTNode* parse_unary() {
    TokenType op = parser.current->type;
    if (op == TOKEN_MINUS || op == TOKEN_NOT) {
        ++parser.current;
        ASTNode* right = parse_primary();
        return make_node_unary(op, right);
    }
    return parse_primary();
}

static ASTNode* parse_primary() {
    if (parser.current->type == TOKEN_WORD_LITERAL) {
        Word value = strtoll(parser.current->value, NULL, 10);
        ++parser.current;
        return make_node_literal(value);
    }
    if (parser.current->type == TOKEN_LEFT_PAREN) {
        ++parser.current;
        ASTNode* inside = parse_expression();
        if (parser.current->type != TOKEN_RIGHT_PAREN) {
            fprintf(stderr, "%s:%d: error: expected closing parenthesis\n", parser.file_path, parser.current->line);
            exit(1);
        }
        ++parser.current;
        return inside;
    }
    fprintf(
        stderr, "%s:%d: error: invalid token: %.*s\n",    
        parser.file_path,
        parser.current->line,    
        parser.current->length,    
        parser.current->value
    );
    exit(1);
}

ASTNode* parser_parse(const char* file_path, TokenArray* token_array) {
    parser.file_path = file_path;
    parser.tokens = token_array->tokens;
    parser.count = token_array->count;
    parser.current = parser.tokens;

    return parse_line();
}

void parser_free_ast(ASTNode* root) {
    switch (root->type) {
        case AST_NODE_BINARY: {
            parser_free_ast(root->binary.left);
            parser_free_ast(root->binary.right);
        } break;
        case AST_NODE_UNARY: {
            parser_free_ast(root->unary.right);
        } break;
        case AST_NODE_LITERAL:
        default:
            break;
    }
    free (root);
}

void parser_print_output(ASTNode* root, int indent) {
    // TODO: temporary, needed for operators
    static const char* token_string[] = {
        "/",
        "*",
        "\045",
        "+",
        "++",
        "-",
        "--",
        "!",
        "!=",
        "=",
        "==",
        ">",
        ">=",
        "<",
        "<=",
        "&",
        "&&",
        "|",
        "||",
    };

    for (int i = 0; i < indent; ++i) printf("  ");

    switch (root->type) {
        case AST_NODE_BINARY: {
            printf("Binary: '%s'\n", token_string[root->binary.op - TOKEN_SLASH]);
            parser_print_output(root->binary.left, indent + 1);
            parser_print_output(root->binary.right, indent + 1);
        } break;
        case AST_NODE_UNARY: {
            printf("Unary: '%s'\n", token_string[root->unary.op - TOKEN_SLASH]);
            parser_print_output(root->unary.right, indent + 1);
        } break;
        case AST_NODE_LITERAL: {
            printf("Literal: %ld\n", root->literal);
        } break;
        default: {
            fprintf(stderr, "%s:%d: error: unknown AST node: %d\n", parser.file_path, parser.current->line, root->type);
        } break;
    }
}
