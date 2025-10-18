#define _POSIX_C_SOURCE 200809L
#include "ast.h"
#include "scope.h"
#include <stdio.h>
#include <string.h>

static long eval_expr(Ast* e, Scope* s);

static void eval_stmt(Ast* st, Scope** s, int* did_return, long* retv){
    if(!st) return;
    switch(st->kind){
        case AST_EXPRSTMT:
            (void)eval_expr(st->as.ret.expr, *s);
            break;
        case AST_VAR_DECL: {
            long v = 0;
            if(st->as.vardecl.init) v = eval_expr(st->as.vardecl.init, *s);
            scope_define(*s, st->as.vardecl.name, v);
        }break;
        case AST_COMPOUND: {
            *s = scope_push(*s);
            for(size_t i=0;i<st->as.list.count;i++){
                eval_stmt(st->as.list.items[i], s, did_return, retv);
                if(*did_return) break;
            }
            *s = scope_pop(*s);
        }break;
        case AST_IF: {
            long c = eval_expr(st->as.ifs.cond, *s);
            if(c) eval_stmt(st->as.ifs.then_br, s, did_return, retv);
            else  eval_stmt(st->as.ifs.else_br, s, did_return, retv);
        }break;
        case AST_WHILE: {
            while(eval_expr(st->as.whiles.cond, *s)){
                eval_stmt(st->as.whiles.body, s, did_return, retv);
                if(*did_return) break;
            }
        }break;
        case AST_RETURN: {
            *retv = eval_expr(st->as.ret.expr, *s);
            *did_return = 1;
        }break;
        default:
            break;
    }
}

static long to_bool(long x){ return x!=0; }

static long eval_expr(Ast* e, Scope* s){
    if(!e) return 0;
    switch(e->kind){
        case AST_NUM: return e->as.num.value;
        case AST_ID: {
            long v=0;
            if(!scope_get(s, e->as.id.name, &v)){
                fprintf(stderr,"Undefined variable '%s'\n", e->as.id.name);
            }
            return v;
        }
        case AST_ASSIGN: {
            long v = eval_expr(e->as.assign.rhs, s);
            if(!scope_set(s, e->as.assign.name, v)){
                scope_define(s, e->as.assign.name, v);
            }
            return v;
        }
        case AST_UN: {
            long a = eval_expr(e->as.un.a, s);
            switch(e->as.un.op){
                case OP_POS: return +a;
                case OP_NEG: return -a;
                case OP_NOT: return !to_bool(a);
                default: return 0;
            }
        }
        case AST_BIN: {
            long A = eval_expr(e->as.bin.a, s);
            long B = eval_expr(e->as.bin.b, s);
            switch(e->as.bin.op){
                case OP_ADD: return A+B;
                case OP_SUB: return A-B;
                case OP_MUL: return A*B;
                case OP_DIV: return B? A/B : 0;
                case OP_MOD: return B? A%B : 0;
                case OP_LT:  return A<B;
                case OP_GT:  return A>B;
                case OP_LE:  return A<=B;
                case OP_GE:  return A>=B;
                case OP_EQ:  return A==B;
                case OP_NE:  return A!=B;
                case OP_AND: return to_bool(A) && to_bool(B);
                case OP_OR:  return to_bool(A) || to_bool(B);
                default: return 0;
            }
        }
        default: return 0;
    }
}

long eval_program(Ast* root){
    Scope* g = scope_push(NULL);
    long ret = 0;
    int did_return = 0;
    if(root && root->kind==AST_PROGRAM){
        for(size_t i=0;i<root->as.list.count;i++){
            Ast* d = root->as.list.items[i];
            if(d->kind==AST_FUNC_DEF && d->as.func.name && strcmp(d->as.func.name,"main")==0){
                Scope* s = g;
                eval_stmt(d->as.func.body, &s, &did_return, &ret);
                break;
            } else if(d->kind==AST_VAR_DECL){
                long v = 0;
                if(d->as.vardecl.init) v = eval_expr(d->as.vardecl.init, g);
                scope_define(g, d->as.vardecl.name, v);
            }
        }
    }
    scope_free_all(g);
    return ret;
}
