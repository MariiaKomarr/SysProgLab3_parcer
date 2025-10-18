#ifndef SCOPE_H
#define SCOPE_H
#include <stddef.h>

typedef struct {
    char* name;
    long value;
} Binding;

typedef struct Scope {
    Binding* arr;
    size_t count;
    struct Scope* parent;
} Scope;

Scope* scope_push(Scope* parent);
Scope* scope_pop(Scope* s);
int scope_set(Scope* s, const char* name, long value);
void scope_define(Scope* s, const char* name, long value);
int scope_get(Scope* s, const char* name, long* out);
void scope_free_all(Scope* s);

#endif
