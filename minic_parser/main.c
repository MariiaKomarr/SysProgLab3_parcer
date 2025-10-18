#include <stdio.h>
#include <string.h>
#include "ast.h"

int yyparse(void);
int yylex_destroy(void);
extern FILE* yyin;
extern Ast* root;

long eval_program(Ast* root);

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file.mc> [--run|--dot]\n", argv[0]);
        return 1;
    }
    const char* path = argv[1];
    FILE* f = fopen(path, "r");
    if(!f){ perror("fopen"); return 1; }
    yyin = f;
    int rc = yyparse();
    fclose(f);
    if(rc != 0){
        fprintf(stderr, "Parse failed.\n");
        return 2;
    }
    if(argc >= 3 && strcmp(argv[2], "--dot")==0){
        ast_dot(root, stdout);
        return 0;
    }
    if(argc >= 3 && strcmp(argv[2], "--run")==0){
        long v = eval_program(root);
        printf("%ld\n", v);
        return 0;
    }
    printf("OK\n");
    return 0;
}
