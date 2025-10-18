#define _POSIX_C_SOURCE 200809L
#include "scope.h"
#include <stdlib.h>
#include <string.h>

Scope* scope_push(Scope* parent){
    Scope* s = (Scope*)calloc(1,sizeof(Scope));
    s->parent = parent;
    return s;
}
Scope* scope_pop(Scope* s){
    Scope* p = s ? s->parent : NULL;
    if(s){
        for(size_t i=0;i<s->count;i++) free(s->arr[i].name);
        free(s->arr);
        free(s);
    }
    return p;
}
static long* find(Scope* s, const char* name){
    for(size_t i=0;i<s->count;i++)
        if(strcmp(s->arr[i].name, name)==0) return &s->arr[i].value;
    return NULL;
}
int scope_set(Scope* s, const char* name, long value){
    for(Scope* cur=s; cur; cur=cur->parent){
        long* p = find(cur,name);
        if(p){ *p = value; return 1; }
    }
    return 0;
}
void scope_define(Scope* s, const char* name, long value){
    s->arr = (Binding*)realloc(s->arr, (s->count+1)*sizeof(Binding));
    s->arr[s->count].name = strdup(name);
    s->arr[s->count].value = value;
    s->count++;
}
int scope_get(Scope* s, const char* name, long* out){
    for(Scope* cur=s; cur; cur=cur->parent){
        long* p = find(cur,name);
        if(p){ *out = *p; return 1; }
    }
    return 0;
}
void scope_free_all(Scope* s){
    while(s) s = scope_pop(s);
}
