#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils.h"

typedef struct s_lexer {
    const char* start;
    const char* current;
    int line;
} Lexer;

static Lexer lexer;

inline static bool lexer_is_at_end() {
    return *lexer.current == '\0';
}

inline static char lexer_peek() {
    return *lexer.current;
}

inline static char lexer_advance() {
    if (*lexer.current == '\n') {
        lexer.line++;
    }
    return *lexer.current++;
}

inline static bool lexer_advance_if(char expected) {
    if (lexer_is_at_end()) return false;
    if (*lexer.current != expected) return false;
    ++lexer.current;
    return true; 
}

inline static Token lexer_make_token(TokenType type) {
    return (Token) {
        .type = type,
        .value = lexer.start,
        .line = lexer.line,
        .length = (int)(lexer.current - lexer.start)
    };
}

inline static Token lexer_make_error_token(const char* message) {
    return (Token) {
        .type = TOKEN_ERROR,
        .value = message,
        .line = lexer.line,
        .length = strlen(message)
    };
}

static void lexer_skip_whitespace() {
    for (;;) {
        char c = lexer_peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                lexer_advance();
                break;
            default:
                return;
        }
        // TODO: handle comments
    }
}

static Token lexer_read_identifier() {
    while (isalnum(lexer_peek()) || lexer_peek() == '_') {
        lexer_advance();
    }

    // TODO: optimize this
    const char* keywords[] = {
        "auto", "extrn",
        "if", "else",
        "switch", "case",
        "goto", "while",
        "return",
    };
    const int keywords_amount = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < keywords_amount; ++i) {
        int length = (int)(lexer.current - lexer.start);
        int keyword_length = strlen(keywords[i]);

        if (length != keyword_length) continue;

        bool might_be_keyword = true;
        for (int j = 0; j < length; ++j) {
            if (*(lexer.start + j) != keywords[i][j]) {
                might_be_keyword = false;
                break;
            }
        }
        if (might_be_keyword) {
            return lexer_make_token(TOKEN_AUTO + i);
        }
    }
    return lexer_make_token(TOKEN_IDENTIFIER);
}

static Token lexer_read_word() {
    while (isdigit(lexer_peek())) {
        lexer_advance();
    }
    return lexer_make_token(TOKEN_WORD_LITERAL);
}

static Token lexer_read_string() {
    // TODO: \" is not handled

    lexer.start = lexer.current;

    while (!lexer_is_at_end() && lexer_peek() != '"') {
        lexer_advance();
    }

    if (lexer_peek() != '"') {
        return lexer_make_error_token("Unterminated string");
    }

    Token token = lexer_make_token(TOKEN_STRING_LITERAL);

    lexer_advance();  // skip ending quote

    return token;
}

static Token lexer_next_token() {
    lexer_skip_whitespace();
    lexer.start = lexer.current;
    
    if (lexer_is_at_end()) {
        return lexer_make_token(TOKEN_EOF);
    }

    char c = lexer_advance();
    switch (c) {
        case '(':
            return lexer_make_token(TOKEN_LEFT_PAREN);
        case ')':
            return lexer_make_token(TOKEN_RIGHT_PAREN);
        case '{':
            return lexer_make_token(TOKEN_LEFT_BRACE);
        case '}':
            return lexer_make_token(TOKEN_RIGHT_BRACE);
        case '[':
            return lexer_make_token(TOKEN_LEFT_BRACKET);
        case ']':
            return lexer_make_token(TOKEN_RIGHT_BRACKET);
        case '.':
            return lexer_make_token(TOKEN_DOT);
        case '?':
            return lexer_make_token(TOKEN_QUESTION_MARK);
        case ';':
            return lexer_make_token(TOKEN_SEMICOLON);
        case ':':
            return lexer_make_token(TOKEN_COLON);
        case '/':
            return lexer_make_token(TOKEN_SLASH);
        case '*':
            return lexer_make_token(TOKEN_ASTERISK);
        case '%':
            return lexer_make_token(TOKEN_PERCENT);
        case '+':
            return lexer_advance_if('+') ? lexer_make_token(TOKEN_INCREMENT) : lexer_make_token(TOKEN_PLUS);
        case '-':
            return lexer_advance_if('-') ? lexer_make_token(TOKEN_DECREMENT) : lexer_make_token(TOKEN_MINUS);
        case '!':
            return lexer_advance_if('=') ? lexer_make_token(TOKEN_NOT_EQUAL) : lexer_make_token(TOKEN_NOT);
        case '=':
            return lexer_advance_if('=') ? lexer_make_token(TOKEN_EQUAL_EQUAL) : lexer_make_token(TOKEN_EQUAL);
        case '>':
            return lexer_advance_if('=') ? lexer_make_token(TOKEN_GREATER_EQUAL) : lexer_make_token(TOKEN_GREATER);
        case '<':
            return lexer_advance_if('=') ? lexer_make_token(TOKEN_LESS_EQUAL) : lexer_make_token(TOKEN_LESS);
        case '&':
            return lexer_advance_if('&') ? lexer_make_token(TOKEN_AND) : lexer_make_token(TOKEN_BIT_AND);
        case '|':
            return lexer_advance_if('|') ? lexer_make_token(TOKEN_OR) : lexer_make_token(TOKEN_BIT_OR);
        case '"':
            return lexer_read_string();
        default:
            break;
    }
    
    if (isdigit(c)) {
        return lexer_read_word();
    }
    else {
        return lexer_read_identifier();
    }

    return lexer_make_error_token("Unknown token");
}

TokenArray lexer_lex(const char* source) {
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;

    TokenArray array = { 0 };

    while (!lexer_is_at_end()) {
        if (array.capacity < array.count + 1) {
            int old_capacity = array.capacity;
            array.capacity = GROW_CAPACITY(old_capacity);
            array.tokens = GROW_ARRAY(Token, array.tokens, old_capacity, array.capacity);
        }
        array.tokens[array.count++] = lexer_next_token();
    }
    return array;
}

void lexer_free_tokens(TokenArray *token_array) {
    free(token_array->tokens);
    
    token_array->tokens = NULL;
    token_array->count = 0;
    token_array->capacity = 0;
}

void lexer_print_output(TokenArray output) {
    static const char* token_string[] = {
        "EOF",
    
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        ",",
        ".",
        "?",
        ";",
        ":",
    
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
        
        "IDENTIFIER",
        "STRING_LITERAL",
        "WORD_LITERAL",
        
        "auto",
        "extrn",
        "if",
        "else",
        "switch",
        "case",
        "goto",
        "while",
        "return",
    
        "ERROR"
    };

    for (int i = 0; i < output.count; ++i) {
        const Token* token = &output.tokens[i];

        TokenType type = token->type;

        if (type == TOKEN_IDENTIFIER || type == TOKEN_WORD_LITERAL || type == TOKEN_STRING_LITERAL) {
            printf(
                "Line: %d,\ttoken: %s,\tvalue: %.*s\n",
                token->line,
                token_string[type],
                token->length,
                token->value
            );
        }
        else {
            printf("Line: %d,\ttoken: %s\n", token->line, token_string[type]);
        }

        if (type == TOKEN_EOF) break;
    }
}
