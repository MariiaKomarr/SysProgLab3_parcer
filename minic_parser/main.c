#include <stdio.h>
#include <stdlib.h>   // system, exit
#include <string.h>
#include "ast.h"

int yyparse(void);
int yylex_destroy(void);
extern FILE* yyin;
extern Ast* root;

long eval_program(Ast* root);

/* Допоміжні функції */
static int write_dot(const char* path) {
    FILE* out = fopen(path, "w");
    if(!out) {
        perror("fopen(dot)");
        return 0;
    }
    ast_dot(root, out);
    fclose(out);
    return 1;
}

static int write_png_from_dot(const char* dot_path, const char* png_path) {
    /* Потрібен graphviz (утиліта `dot`) у PATH.
       MSYS2:  pacman -S graphviz
       Ubuntu: sudo apt-get install graphviz
       Windows (choco): choco install graphviz */
    char cmd[512];
#if defined(_WIN32)
    /* На Windows достатньо, щоб `dot.exe` був у PATH */
#endif
    snprintf(cmd, sizeof(cmd), "dot -Tpng \"%s\" -o \"%s\"", dot_path, png_path);
    int rc = system(cmd);
    return (rc == 0);
}

static void print_usage(const char* argv0) {
    fprintf(stderr,
            "Usage: %s <file.mc> [--run | --dot | --png]\n"
            "  --run : parse and execute program (print result)\n"
            "  --dot : write AST to tree.dot\n"
            "  --png : write AST to tree.dot and render tree.png via Graphviz\n",
            argv0);
}

int main(int argc, char** argv){
    if(argc < 2){
        print_usage(argv[0]);
        return 1;
    }

    const char* path = argv[1];
    const char* mode = (argc >= 3) ? argv[2] : NULL;

    FILE* f = fopen(path, "r");
    if(!f){
        perror("fopen(input)");
        return 1;
    }
    yyin = f;

    int rc = yyparse();
    fclose(f);
    yylex_destroy();

    if(rc != 0){
        fprintf(stderr, "Parse failed.\n");
        return 2;
    }

    if(mode && strcmp(mode, "--dot") == 0){
        if(write_dot("tree.dot")){
            fprintf(stdout, "✅ AST saved to tree.dot\n");
            return 0;
        } else {
            return 3;
        }
    }

    if(mode && strcmp(mode, "--png") == 0){
        if(!write_dot("tree.dot")) return 3;
        if(write_png_from_dot("tree.dot", "tree.png")){
            fprintf(stdout, "✅ AST saved to tree.dot and tree.png\n");
            return 0;
        } else {
            fprintf(stderr, "⚠️ Graphviz 'dot' not found or failed. You can render tree.dot manually.\n");
            return 4;
        }
    }

    if(mode && strcmp(mode, "--run") == 0){
        long v = eval_program(root);
        printf("%ld\n", v);
        return 0;
    }

    /* За замовчуванням — просто парсимо */
    printf("OK\n");
    return 0;
}
