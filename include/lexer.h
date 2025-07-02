#pragma once
#include <stdint.h>

// TODO: add assignment operators
typedef enum e_token_type {
    TOKEN_EOF,             // EOF

    TOKEN_LEFT_PAREN,      // (
    TOKEN_RIGHT_PAREN,     // )
    TOKEN_LEFT_BRACE,      // {
    TOKEN_RIGHT_BRACE,     // }
    TOKEN_LEFT_BRACKET,    // [
    TOKEN_RIGHT_BRACKET,   // ]
    TOKEN_COMMA,           // ,
    TOKEN_DOT,             // .
    TOKEN_QUESTION_MARK,   // ?
    TOKEN_SEMICOLON,       // ;
    TOKEN_COLON,           // :

    TOKEN_SLASH,           // /
    TOKEN_ASTERISK,        // *
    TOKEN_PERCENT,         // %
    TOKEN_PLUS,            // +
    TOKEN_INCREMENT,       // ++
    TOKEN_MINUS,           // -
    TOKEN_DECREMENT,       // --
    TOKEN_NOT,             // !
    TOKEN_NOT_EQUAL,       // !=
    TOKEN_EQUAL,           // =
    TOKEN_EQUAL_EQUAL,     // ==
    TOKEN_GREATER,         // >
    TOKEN_GREATER_EQUAL,   // >=
    TOKEN_LESS,            // <
    TOKEN_LESS_EQUAL,      // <=
    TOKEN_BIT_AND,         // &
    TOKEN_AND,             // &&
    TOKEN_BIT_OR,          // |
    TOKEN_OR,              // ||

    TOKEN_IDENTIFIER,      // x
    TOKEN_STRING_LITERAL,  // ""
    TOKEN_WORD_LITERAL,    // 0

    TOKEN_AUTO,            // auto
    TOKEN_EXTRN,           // extrn
    TOKEN_IF,              // if
    TOKEN_ELSE,            // else
    TOKEN_SWITCH,          // switch
    TOKEN_CASE,            // case
    TOKEN_GOTO,            // goto
    TOKEN_WHILE,           // while
    TOKEN_RETURN,          // return

    TOKEN_ERROR
} TokenType;

typedef struct s_token {
    TokenType type;
    const char* value;
    int line;
    int length;
} Token;

typedef struct s_token_array {
    Token* tokens;
    int count;
    int capacity;
} TokenArray;

TokenArray lexer_lex(const char* source);
void lexer_free_tokens(TokenArray* token_array);
void lexer_print_output(TokenArray token_array);
