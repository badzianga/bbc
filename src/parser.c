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
    if (parser.current->type == TOKEN_AUTO) {
        ++parser.current;
        if (parser.current->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "%s:%d: error: expected identifier name after `auto`\n", parser.file_path, parser.current->line);
            exit(1);
        }
        // TODO: free this memory
        char* name = strndup(parser.current->value, parser.current->length);
        ++parser.current;
        if (parser.current->type != TOKEN_SEMICOLON) {
            fprintf(stderr, "%s:%d: error: expected ';' after expression\n", parser.file_path, parser.current->line);
            exit(1);
        }
        ++parser.current;
        return make_node_variable_declaration(name);
    }

    return parse_statement();
}

static ASTNode* parse_statement() {
    ASTNode* expression = parse_expression();
    if (parser.current->type != TOKEN_SEMICOLON) {
        fprintf(stderr, "%s:%d: error: expected ';' after expression\n", parser.file_path, parser.current->line);
        exit(1);
    }
    ++parser.current;

    return make_node_expression_statement(expression);
}

static ASTNode* parse_expression() {
    return parse_assignment();
}

static ASTNode* parse_assignment() {
    // TODO: should it really be parsed like this?
    ASTNode* expression = parse_equality();

    if (parser.current->type == TOKEN_EQUAL) {
        ++parser.current;
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
    if (parser.current->type == TOKEN_IDENTIFIER) {
        char* name = strndup(parser.current->value, parser.current->length);
        ++parser.current;
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
