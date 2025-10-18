#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdio.h>

typedef enum {
    AST_PROGRAM,
    AST_COMPOUND,
    AST_EXPRSTMT,
    AST_VAR_DECL,
    AST_FUNC_DEF,
    AST_IF,
    AST_WHILE,
    AST_RETURN,
    AST_BIN,
    AST_UN,
    AST_NUM,
    AST_ID,
    AST_ASSIGN,
    AST_LIST
} AstKind;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_LT, OP_GT, OP_LE, OP_GE,
    OP_EQ, OP_NE,
    OP_AND, OP_OR,
    OP_POS, OP_NEG, OP_NOT
} OpCode;

typedef struct Ast Ast;

typedef struct {
    Ast** items;
    size_t count;
} AstList;

struct Ast {
    AstKind kind;
    int line, col;
    union {
        struct { Ast** items; size_t count; } list;
        struct { char* name; Ast* init; } vardecl;
        struct { char* name; Ast* params; Ast* body; } func;
        struct { Ast* cond; Ast* then_br; Ast* else_br; } ifs;
        struct { Ast* cond; Ast* body; } whiles;
        struct { Ast* expr; } ret;
        struct { OpCode op; Ast* a; Ast* b; } bin;
        struct { OpCode op; Ast* a; } un;
        struct { long value; } num;
        struct { char* name; } id;
        struct { char* name; Ast* rhs; } assign;
    } as;
};

Ast* ast_make(AstKind k, int line, int col);
Ast* ast_program(Ast* decls);
Ast* ast_list_append(Ast* list_or_null, Ast* item);
Ast* ast_var_decl(char* name, Ast* init, int line, int col);
Ast* ast_func_def(char* name, Ast* params, Ast* body, int line, int col);
Ast* ast_exprstmt(Ast* e, int line, int col);
Ast* ast_if(Ast* c, Ast* t, Ast* e, int line, int col);
Ast* ast_while(Ast* c, Ast* b, int line, int col);
Ast* ast_return(Ast* e, int line, int col);
Ast* ast_bin(OpCode op, Ast* a, Ast* b, int line, int col);
Ast* ast_un(OpCode op, Ast* a, int line, int col);
Ast* ast_num(long v, int line, int col);
Ast* ast_id(char* name, int line, int col);
Ast* ast_assign(char* name, Ast* rhs, int line, int col);

void ast_dot(Ast* n, FILE* out);

#endif
