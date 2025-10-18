#define _POSIX_C_SOURCE 200809L
#include "ast.h"
#include <stdlib.h>
#include <string.h>

static Ast* make_list_node() {
    Ast* n = ast_make(AST_LIST, 0, 0);
    n->as.list.items = NULL;
    n->as.list.count = 0;
    return n;
}

Ast* ast_make(AstKind k, int line, int col){
    Ast* n = (Ast*)calloc(1, sizeof(Ast));
    n->kind = k;
    n->line = line;
    n->col = col;
    return n;
}

Ast* ast_program(Ast* decls){
    Ast* n = ast_make(AST_PROGRAM, 0, 0);
    if (decls && decls->kind == AST_LIST) n->as.list = decls->as.list;
    else {
        n->as.list.items = NULL;
        n->as.list.count = 0;
        if (decls) {
            n->as.list.items = (Ast**)calloc(1, sizeof(Ast*));
            n->as.list.items[0] = decls;
            n->as.list.count = 1;
        }
    }
    return n;
}

Ast* ast_list_append(Ast* list_or_null, Ast* item){
    Ast* lst = list_or_null;
    if (!lst) lst = make_list_node();
    size_t newc = lst->as.list.count + 1;
    lst->as.list.items = (Ast**)realloc(lst->as.list.items, newc * sizeof(Ast*));
    lst->as.list.items[lst->as.list.count++] = item;
    return lst;
}

Ast* ast_var_decl(char* name, Ast* init, int line, int col){
    Ast* n = ast_make(AST_VAR_DECL, line, col);
    n->as.vardecl.name = strdup(name);
    n->as.vardecl.init = init;
    return n;
}

Ast* ast_func_def(char* name, Ast* params, Ast* body, int line, int col){
    Ast* n = ast_make(AST_FUNC_DEF, line, col);
    n->as.func.name = strdup(name);
    n->as.func.params = params;
    n->as.func.body = body;
    return n;
}

Ast* ast_exprstmt(Ast* e, int line, int col){
    Ast* n = ast_make(AST_EXPRSTMT, line, col);
    n->as.ret.expr = e;
    return n;
}

Ast* ast_if(Ast* c, Ast* t, Ast* e, int line, int col){
    Ast* n = ast_make(AST_IF, line, col);
    n->as.ifs.cond = c; n->as.ifs.then_br = t; n->as.ifs.else_br = e;
    return n;
}

Ast* ast_while(Ast* c, Ast* b, int line, int col){
    Ast* n = ast_make(AST_WHILE, line, col);
    n->as.whiles.cond = c; n->as.whiles.body = b;
    return n;
}

Ast* ast_return(Ast* e, int line, int col){
    Ast* n = ast_make(AST_RETURN, line, col);
    n->as.ret.expr = e;
    return n;
}

Ast* ast_bin(OpCode op, Ast* a, Ast* b, int line, int col){
    Ast* n = ast_make(AST_BIN, line, col);
    n->as.bin.op = op; n->as.bin.a = a; n->as.bin.b = b;
    return n;
}

Ast* ast_un(OpCode op, Ast* a, int line, int col){
    Ast* n = ast_make(AST_UN, line, col);
    n->as.un.op = op; n->as.un.a = a;
    return n;
}

Ast* ast_num(long v, int line, int col){
    Ast* n = ast_make(AST_NUM, line, col);
    n->as.num.value = v;
    return n;
}

Ast* ast_id(char* name, int line, int col){
    Ast* n = ast_make(AST_ID, line, col);
    n->as.id.name = strdup(name);
    return n;
}

Ast* ast_assign(char* name, Ast* rhs, int line, int col){
    Ast* n = ast_make(AST_ASSIGN, line, col);
    n->as.assign.name = strdup(name);
    n->as.assign.rhs = rhs;
    return n;
}

static void dot_node(FILE* f, Ast* n, int id){
    if(!n){ fprintf(f, "  n%d [label=\"(null)\"]\n", id); return; }
    const char* label="?";
    char buf[64];
    switch(n->kind){
        case AST_PROGRAM: label="PROGRAM"; break;
        case AST_COMPOUND: label="COMPOUND"; break;
        case AST_EXPRSTMT: label="EXPRSTMT"; break;
        case AST_VAR_DECL: label="VAR_DECL"; break;
        case AST_FUNC_DEF: label="FUNC_DEF"; break;
        case AST_IF: label="IF"; break;
        case AST_WHILE: label="WHILE"; break;
        case AST_RETURN: label="RETURN"; break;
        case AST_BIN: label="BIN"; break;
        case AST_UN: label="UN"; break;
        case AST_NUM: snprintf(buf,sizeof(buf),"NUM %ld", n->as.num.value); label=buf; break;
        case AST_ID: label = n->as.id.name; break;
        case AST_ASSIGN: label="ASSIGN"; break;
        case AST_LIST: label="LIST"; break;
    }
    fprintf(f, "  n%d [shape=box,label=\"%s\"]\n", id, label);
}
static int dot_emit(FILE* f, Ast* n, int id){
    if(!n){ dot_node(f,n,id); return id; }
    dot_node(f,n,id);
    int cur=id;
    switch(n->kind){
        case AST_PROGRAM:
        case AST_COMPOUND:
        case AST_LIST: {
            for(size_t i=0;i<n->as.list.count;i++){
                fprintf(f,"  n%d -> n%d\n",cur,++id);
                id = dot_emit(f, n->as.list.items[i], id);
            }
        }break;
        case AST_VAR_DECL:
            fprintf(f,"  n%d -> n%d\n",cur,++id);
            id = dot_emit(f, n->as.vardecl.init, id);
            break;
        case AST_FUNC_DEF:
            fprintf(f,"  n%d -> n%d\n",cur,++id);
            id = dot_emit(f, n->as.func.params, id);
            fprintf(f,"  n%d -> n%d\n",cur,++id);
            id = dot_emit(f, n->as.func.body, id);
            break;
        case AST_IF:
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.ifs.cond, id);
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.ifs.then_br, id);
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.ifs.else_br, id);
            break;
        case AST_WHILE:
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.whiles.cond, id);
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.whiles.body, id);
            break;
        case AST_RETURN:
        case AST_EXPRSTMT:
            fprintf(f,"  n%d -> n%d\n",cur,++id);
            id = dot_emit(f, n->as.ret.expr, id);
            break;
        case AST_BIN:
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.bin.a, id);
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.bin.b, id);
            break;
        case AST_UN:
            fprintf(f,"  n%d -> n%d\n",cur,++id); id = dot_emit(f, n->as.un.a, id);
            break;
        case AST_NUM:
        case AST_ID:
        case AST_ASSIGN:
            if(n->kind==AST_ASSIGN){
                fprintf(f,"  n%d -> n%d\n",cur,++id);
                id = dot_emit(f, n->as.assign.rhs, id);
            }
            break;
    }
    return id;
}
void ast_dot(Ast* n, FILE* out){
    fprintf(out, "digraph AST {\n");
    dot_emit(out, n, 0);
    fprintf(out, "}\n");
}
