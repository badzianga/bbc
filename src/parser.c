#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"

typedef struct s_parser {
    const char* file_path;
    Token* tokens;
    Token* current;
    int count;
} Parser;

static Parser parser;

static void consume_expected(TokenType token, const char* error_if_fail) {
    if (parser.current->type != token) {
        fprintf(stderr, "%s:%d: error: %s\n", parser.file_path, parser.current->line, error_if_fail);
        exit(1);
    }
    ++parser.current;
}

static bool match(int argc, ...) {
    TokenType token = parser.current->type;
    va_list argv;
    va_start(argv, argc);
    for (int i = 0; i < argc; ++i) {
        if (token == va_arg(argv, TokenType)) {
            va_end(argv);
            ++parser.current;
            return true;
        }
    }
    va_end(argv);
    return false;
}

inline static Token* previous() {
    return parser.current - 1;
}

static ASTNode* make_node_program() {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    node->type = AST_NODE_PROGRAM;
    return node;
}

static ASTNode* make_node_expression_statement(ASTNode* expression) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NODE_EXPRESSION_STATEMENT;
    node->expression = expression;
    return node;
}

static ASTNode* make_node_variable_declaration(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NODE_VARIABLE_DECLARATION;
    node->name = name;
    return node;
}

static ASTNode* make_node_block() {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    node->type = AST_NODE_BLOCK;
    return node;
}

// static ASTNode* make_node_assignment(char* name, ASTNode* value) {
//     ASTNode* node = malloc(sizeof(ASTNode));
//     node->type = AST_NODE_ASSIGNMENT;
//     node->assignment.name = name;
//     node->assignment.value = value;
//     return node;
// }

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

static ASTNode* make_node_variable(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NODE_VARIABLE;
    node->name = name;
    return node;
}

static ASTNode* parse_program();
static ASTNode* parse_declaration();
static ASTNode* parse_statement();
static ASTNode* parse_block();

static ASTNode* parse_expression();
static ASTNode* parse_assignment();
static ASTNode* parse_equality();
static ASTNode* parse_comparison();
static ASTNode* parse_term();
static ASTNode* parse_factor();
static ASTNode* parse_unary();
static ASTNode* parse_primary();

static ASTNode* parse_program() {
    ASTNode* node = make_node_program();
    while (parser.current->type != TOKEN_EOF) {
        if (node->program.capacity < node->program.count + 1) {
            int old_capacity = node->program.capacity;
            node->program.capacity = GROW_CAPACITY(old_capacity);
            node->program.statements = GROW_ARRAY(ASTNode*, node->program.statements, old_capacity, node->program.capacity);
        }
        node->program.statements[node->program.count++] = parse_declaration();
    }
    return node;
}

static ASTNode* parse_declaration() {
    if (match(1, TOKEN_AUTO)) {
        consume_expected(TOKEN_IDENTIFIER, "expected identifier name after 'auto'\n");
        char* name = strndup(previous()->value, previous()->length);
        consume_expected(TOKEN_SEMICOLON, "expected ';' after expression");
        return make_node_variable_declaration(name);
    }

    return parse_statement();
}

static ASTNode* parse_statement() {
    if (match(1, TOKEN_LEFT_BRACE)) {
        ASTNode* node = parse_block();
        consume_expected(TOKEN_RIGHT_BRACE, "expected '}' after block");
        return node;
    }

    ASTNode* expression = parse_expression();
    consume_expected(TOKEN_SEMICOLON, "expected ';' after expression");
    return make_node_expression_statement(expression);
}

static ASTNode* parse_block() {
    ASTNode* node = make_node_block();
    while (parser.current->type != TOKEN_RIGHT_BRACE && parser.current->type != TOKEN_EOF) {
        if (node->block.capacity < node->block.count + 1) {
            int old_capacity = node->block.capacity;
            node->block.capacity = GROW_CAPACITY(old_capacity);
            node->block.statements = GROW_ARRAY(ASTNode*, node->block.statements, old_capacity, node->block.capacity);
        }
        node->block.statements[node->block.count++] = parse_declaration();
    }
    return node;
}

static ASTNode* parse_expression() {
    return parse_assignment();
}

static ASTNode* parse_assignment() {
    ASTNode* expression = parse_equality();

    if (match(1, TOKEN_EQUAL)) {
        ASTNode* value = parse_assignment();

        if (expression->type == AST_NODE_VARIABLE) {
            char* name = expression->name;
            expression->type = AST_NODE_ASSIGNMENT;
            expression->assignment.name = name;
            expression->assignment.value = value;
            return expression;
        }
        fprintf(stderr, "error: invalid assignment target\n");
        exit(1);
    }

    return expression;
}

static ASTNode* parse_equality() {
    ASTNode* left = parse_comparison();

    while (match(2, TOKEN_EQUAL_EQUAL, TOKEN_NOT_EQUAL)) {
        TokenType op = previous()->type;
        ASTNode* right = parse_comparison();
        left = make_node_binary(left, op, right);
    }
    return left;
}

static ASTNode* parse_comparison() {
    ASTNode* left = parse_term();

    while (match(4, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL)) {
        TokenType op = previous()->type;
        ASTNode* right = parse_term();
        left = make_node_binary(left, op, right);
    }
    return left;
}

static ASTNode* parse_term() {
    ASTNode* left = parse_factor();

    while (match(2, TOKEN_PLUS, TOKEN_MINUS)) {
        TokenType op = previous()->type;
        ASTNode* right = parse_factor();
        left = make_node_binary(left, op, right);
    }
    return left;
}

static ASTNode* parse_factor() {
    ASTNode* left = parse_unary();

    while (match(3, TOKEN_SLASH, TOKEN_ASTERISK, TOKEN_PERCENT)) {
        TokenType op = previous()->type;
        ASTNode* right = parse_unary();
        left = make_node_binary(left, op, right);
    }
    return left;
}

static ASTNode* parse_unary() {
    if (match(2, TOKEN_MINUS, TOKEN_NOT)) {
        TokenType op = previous()->type;
        ASTNode* right = parse_primary();
        return make_node_unary(op, right);
    }
    return parse_primary();
}

static ASTNode* parse_primary() {
    if (match(1, TOKEN_WORD_LITERAL)) {
        Word value = strtoll(previous()->value, NULL, 10);
        return make_node_literal(value);
    }
    if (match(1, TOKEN_LEFT_PAREN)) {
        ASTNode* inside = parse_expression();
        consume_expected(TOKEN_RIGHT_PAREN, "expected closing parenthesis");
        return inside;
    }
    if (match(1, TOKEN_IDENTIFIER)) {
        char* name = strndup(previous()->value, previous()->length);
        return make_node_variable(name);
    }
    fprintf(
        stderr, "%s:%d: error: invalid token: '%.*s'\n",    
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

    return parse_program();
}

void parser_free_ast(ASTNode* root) {
    switch (root->type) {
        case AST_NODE_PROGRAM: {
            for (int i = 0; i < root->program.count; ++i) {
                parser_free_ast(root->program.statements[i]);
            }
            free(root->program.statements);
        } break;
        case AST_NODE_BLOCK: {
            for (int i = 0; i < root->block.count; ++i) {
                parser_free_ast(root->block.statements[i]);
            }
            free(root->block.statements);
        } break;
        case AST_NODE_EXPRESSION_STATEMENT: {
            parser_free_ast(root->expression);
        } break;
        case AST_NODE_VARIABLE_DECLARATION: {
            free(root->name);
        } break;
        case AST_NODE_ASSIGNMENT: {
            free(root->assignment.name);
            parser_free_ast(root->assignment.value);
        } break;
        case AST_NODE_BINARY: {
            parser_free_ast(root->binary.left);
            parser_free_ast(root->binary.right);
        } break;
        case AST_NODE_UNARY: {
            parser_free_ast(root->unary.right);
        } break;
        case AST_NODE_LITERAL: break;
        case AST_NODE_VARIABLE: {
            free(root->name);
        } break;
        default: {
            fprintf(stderr, "error: unknown AST node to free: %d\n", root->type);
        } break;
    }
    free (root);
}

void parser_print_output(ASTNode* root, int indent) {
    for (int i = 0; i < indent; ++i) printf("  ");

    switch (root->type) {
        case AST_NODE_PROGRAM: {
            printf("Program:\n");
            for (int i = 0; i < root->program.count; ++i) {
                parser_print_output(root->program.statements[i], indent + 1);                
            }
        } break;
        case AST_NODE_BLOCK: {
            printf("Block:\n");
            for (int i = 0; i < root->block.count; ++i) {
                parser_print_output(root->block.statements[i], indent + 1);                
            }
        } break;
        case AST_NODE_EXPRESSION_STATEMENT: {
            printf("ExpressionStatement:\n");
            parser_print_output(root->expression, indent + 1);
        } break;
        case AST_NODE_VARIABLE_DECLARATION: {
            printf("VariableDeclaration: %s\n", root->name);
        } break;
        case AST_NODE_ASSIGNMENT: {
            printf("Assignment: %s\n", root->assignment.name);
            parser_print_output(root->assignment.value, indent + 1);
        } break;
        case AST_NODE_BINARY: {
            printf("Binary: '%s'\n", token_as_cstr(root->binary.op));
            parser_print_output(root->binary.left, indent + 1);
            parser_print_output(root->binary.right, indent + 1);
        } break;
        case AST_NODE_UNARY: {
            printf("Unary: '%s'\n", token_as_cstr(root->unary.op));
            parser_print_output(root->unary.right, indent + 1);
        } break;
        case AST_NODE_LITERAL: {
            printf("Literal: %ld\n", root->literal);
        } break;
        case AST_NODE_VARIABLE: {
            printf("Variable: %s\n", root->name);
        } break;
        default: {
            fprintf(stderr, "%s:%d: error: unknown AST node: %d\n", parser.file_path, parser.current->line, root->type);
            exit(1);
        } break;
    }
}
