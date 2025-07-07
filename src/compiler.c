#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include "parser.h"

typedef struct {
    const char* name;
    Word offset;
} AutoVar;

// TODO: change to hash map
static AutoVar vars[128];
static size_t vars_index = 0;
static size_t vars_offset = 0;
static int pushed_on_stack = 0;

static AutoVar* find_auto_var(const char* name) {
    for (size_t i = 0; i < vars_index; ++i) {
        if (strcmp(name, vars[i].name) == 0) {
            return &vars[i];
        }
    }
    return NULL;
}

static void compile(ASTNode* root, FILE* file) {
    switch (root->type) {
        case AST_NODE_EXPRESSION_STATEMENT: {
            compile(root->expression, file);
        } break;
        case AST_NODE_VARIABLE_DECLARATION: {
            AutoVar* existing_var = find_auto_var(root->name);
            if (existing_var != NULL) {
                fprintf(stderr, "error: identifier '%s' already declared\n", root->name);
                fclose(file);
                exit(1);
            }
            vars_offset += sizeof(Word);
            vars[vars_index++] = (AutoVar) { .name = root->name, .offset = vars_offset };
            fprintf(file, "\t;---var_decl---\n");
            fprintf(file, "\tsub rsp, %zu\n", sizeof(Word));
        } break;
        case AST_NODE_ASSIGNMENT: {
            compile(root->assignment.value, file);
            AutoVar* var = find_auto_var(root->assignment.name);
            if (var == NULL) {
                fprintf(stderr, "error: undeclared identifier '%s'\n", root->name);
                fclose(file);
                exit(1);
            }
            // TODO: expressions like a = b = 10 don't work (10 is not on the stack anymore,
            // however, this value is still in rax register, so it should be used)
            fprintf(file, "\t;---assign---\n");
            fprintf(file, "\tpop rax\n"); --pushed_on_stack;
            fprintf(file, "\tmov QWORD [rbp-%zu], rax\n", var->offset);
        } break;
        case AST_NODE_BINARY: {
            compile(root->binary.left, file);
            compile(root->binary.right, file);
            fprintf(file, "\t;---binary---\n");
            fprintf(file, "\tpop rbx\n"); --pushed_on_stack;
            fprintf(file, "\tpop rax\n"); --pushed_on_stack;
            switch (root->binary.op) {
                case TOKEN_SLASH: {
                    fprintf(file, "\t;---div---\n");
                    fprintf(file, "\tidiv rbx\n");
                    fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
                } break;
                case TOKEN_ASTERISK: {
                    fprintf(file, "\t;---mul---\n");
                    fprintf(file, "\timul rax, rbx\n");
                    fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
                } break;
                case TOKEN_PERCENT: {
                    fprintf(file, "\t;---mod---\n");
                    fprintf(file, "\tidiv rbx\n");
                    fprintf(file, "\tpush rdx\n"); ++pushed_on_stack;
                } break;
                case TOKEN_PLUS: {
                    fprintf(file, "\t;---add---\n");
                    fprintf(file, "\tadd rax, rbx\n");
                    fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
                } break;
                case TOKEN_MINUS: {
                    fprintf(file, "\t;---sub---\n");
                    fprintf(file, "\tsub rax, rbx\n");
                    fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
                } break;
                case TOKEN_NOT_EQUAL: {
                    // TODO: not implemented
                } break;
                case TOKEN_EQUAL_EQUAL: {
                    // TODO: not implemented
                } break;
                case TOKEN_GREATER: {
                    // TODO: not implemented
                } break;
                case TOKEN_GREATER_EQUAL: {
                    // TODO: not implemented
                } break;
                case TOKEN_LESS: {
                    // TODO: not implemented
                } break;
                case TOKEN_LESS_EQUAL: {
                    // TODO: not implemented
                } break;
                default: {
                    fprintf(
                        stderr,
                        "error: invalid operator in binary operation: %s\n",
                        token_as_cstr(root->binary.op)
                    );
                    exit(1);
                } break;
            }
        } break;
        case AST_NODE_UNARY: {
            compile(root->unary.right, file);
            fprintf(file, "\t;---unary---\n");
            fprintf(file, "\tpop rax\n");  --pushed_on_stack;

            switch (root->unary.op) {
                case TOKEN_MINUS: {
                    fprintf(file, "\t;---negate---\n");
                    fprintf(file, "\tneg rax\n");
                    fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
                } break;
                case TOKEN_NOT: {
                    // TODO: not implemented
                } break;
                default: {
                    fprintf(
                        stderr,
                        "error: invalid operator in unary operation: %s\n",
                        token_as_cstr(root->unary.op)
                    );
                    exit(1);
                }
            }
        } break;
        case AST_NODE_LITERAL: {
            fprintf(file, "\t;---literal---\n");
            fprintf(file, "\tmov rax, %ld\n", root->literal);
            fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
        } break;
        case AST_NODE_VARIABLE: {
            AutoVar* var = find_auto_var(root->name);
            if (var == NULL) {
                fprintf(stderr, "error: undeclared identifier '%s'\n", root->name);
                fclose(file);
                exit(1);
            }
            fprintf(file, "\t;---var---\n");
            fprintf(file, "\tmov rax, [rbp-%zu]\n", var->offset);
            fprintf(file, "\tpush rax\n"); ++pushed_on_stack;
        } break;
        default: break;
    }
}

void compiler_compile(ASTNode* program, const char* filename) {
    if (program->type != AST_NODE_PROGRAM) {
        fprintf(stderr, "error: AST node for compiler is not a program\n");
        exit(1);
    }

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "error: failed to open file: %s\n", filename);
        exit(1);
    }

    fprintf(file, "format ELF64\n");
    fprintf(file, "section \".text\" executable\n");

    // TODO: temporary, bbc doesn't support functions for now
    fprintf(file, ";---func (NOT SUPPORTED)---\n");
    fprintf(file, "public main\n");
    fprintf(file, "main:\n");

    fprintf(file, "\t;---func start (NOT SUPPORTED)---\n");
    fprintf(file, "\tpush rbp\n");
    fprintf(file, "\tmov rbp, rsp\n");

    for (int i = 0; i < program->program.count; ++i) {
        compile(program->program.statements[i], file);
    }

    // clear unused pushed values from the stack
    while (pushed_on_stack > 0) {
        fprintf(file, "\tpop rbx\n"); --pushed_on_stack;
    }

    fprintf(file, "\t;---function end (NOT SUPPORTED)---\n");
    fprintf(file, "\tadd rsp, %zu\n", vars_offset);
    fprintf(file, "\tpop rbp\n");
    fprintf(file, "\tmov rax, 0\n");
    fprintf(file, "\tret\n");

    fclose(file);
}
