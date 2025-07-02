#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

typedef struct s_parser {
    Token* tokens;
    Token* current;
    int count;
} Parser;

static Parser parser;

static int parse_expression();
static int parse_equality();
static int parse_comparison();
static int parse_term();
static int parse_factor();
static int parse_unary();
static int parse_primary();

// TODO: to delete
static int parse_line() {
    int result = parse_expression();
    if (parser.current->type != TOKEN_SEMICOLON) {
        fprintf(stderr, "error: expected ';' after expression\n");
        exit(1);
    }
    ++parser.current;
    return result;
}

static int parse_expression() {
    return parse_equality();
}

static int parse_equality() {
    int left = parse_comparison();

    TokenType operator = parser.current->type;
    while (operator == TOKEN_EQUAL_EQUAL || operator == TOKEN_NOT_EQUAL) {
        ++parser.current;
        int right = parse_comparison();
        if (operator == TOKEN_EQUAL_EQUAL) {
            left = (left == right);
        }
        else {
            left = (left != right);
        }
        operator = parser.current->type;
    }
    return left;
}

static int parse_comparison() {
    int left = parse_term();

    TokenType operator = parser.current->type;
    while (operator == TOKEN_GREATER || operator == TOKEN_GREATER_EQUAL || operator == TOKEN_LESS || operator == TOKEN_LESS_EQUAL) {
        ++parser.current;
        int right = parse_term();
        if (operator == TOKEN_GREATER) {
            left = (left > right);
        }
        else if (operator == TOKEN_GREATER_EQUAL) {
            left = (left >= right);
        }
        else if (operator == TOKEN_LESS) {
            left = (left < right);
        }
        else {
            left = (left <= right);
        }
        operator = parser.current->type;
    }
    return left;
}

static int parse_term() {
    int left = parse_factor();

    TokenType operator = parser.current->type;
    while (operator == TOKEN_PLUS || operator == TOKEN_MINUS) {
        ++parser.current;
        int right = parse_factor();
        if (operator == TOKEN_PLUS) {
            left += right;
        }
        else {
            left -= right;
        }
        operator = parser.current->type;
    }
    return left;
}

static int parse_factor() {
    int left = parse_unary();

    TokenType operator = parser.current->type;
    while (operator == TOKEN_SLASH || operator == TOKEN_ASTERISK || operator == TOKEN_PERCENT) {
        ++parser.current;
        int right = parse_unary();
        if (operator == TOKEN_SLASH) {
            left /= right;
        }
        else if (operator == TOKEN_ASTERISK) {
            left *= right;
        }
        else {
            left %= right;
        }
        operator = parser.current->type;
    }
    return left;
}

static int parse_unary() {
    TokenType operator = parser.current->type;
    if (operator == TOKEN_MINUS || operator == TOKEN_NOT) {
        ++parser.current;
        int right = parse_primary();
        if (operator == TOKEN_MINUS) {
            return -right;
        }
        return !right;
    }
    return parse_primary();
}

static int parse_primary() {
    if (parser.current->type == TOKEN_WORD_LITERAL) {
        int value = strtol(parser.current->value, NULL, 10);
        ++parser.current;
        return value;
    }
    if (parser.current->type == TOKEN_LEFT_PAREN) {
        ++parser.current;
        int inside = parse_expression();
        if (parser.current->type != TOKEN_RIGHT_PAREN) {
            fprintf(stderr, "error: expected closing parenthesis\n");
            exit(1);
        }
        ++parser.current;
        return inside;
    }
    fprintf(stderr, "error: invalid token: %.*s", parser.current->length, parser.current->value);
    exit(1);
}

int parser_parse(TokenArray* token_array) {
    parser.tokens = token_array->tokens;
    parser.count = token_array->count;
    parser.current = parser.tokens;

    return parse_line();
}
