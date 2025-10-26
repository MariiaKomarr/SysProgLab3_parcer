%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "util.h"

Ast* root = NULL;

void yyerror(const char* s);
int yylex(void);
extern int yylineno;
%}

%define parse.error "verbose"

%union {
    long ival;
    char* sval;
    Ast* node;
}

/* tokens */
%token T_INT T_IF T_ELSE T_WHILE T_RETURN
%token <sval> T_ID
%token <ival> T_NUM

%token T_EQ T_NE T_LE T_GE T_AND T_OR

/* precedence */
%left T_OR
%left T_AND
%nonassoc T_EQ T_NE
%nonassoc '<' '>' T_LE T_GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UPLUS UMINUS
%right '='
%nonassoc IFX
%nonassoc T_ELSE

%type <node> program decl_list decl var_decl func_def param_list_opt param_list params compound stmt_list_opt stmt expr_stmt if_stmt while_stmt return_stmt expr assignment logic_or logic_and equality rel add mul unary primary

%printer { fprintf(yyoutput, "%s", $$); } <sval>

%%

program : decl_list               { root = ast_program($1); }
        ;

decl_list
        : decl_list decl          { $$ = ast_list_append($1, $2); }
        | decl                    { $$ = ast_list_append(NULL, $1); }
        ;

decl    : var_decl                { $$ = $1; }
        | func_def                { $$ = $1; }
        ;

var_decl: T_INT T_ID ';'                          { $$ = ast_var_decl($2, NULL, 0, 0); }
        | T_INT T_ID '=' expr ';'                 { $$ = ast_var_decl($2, $4,   0, 0); }
        ;

func_def: T_INT T_ID '(' param_list_opt ')' compound
        { $$ = ast_func_def($2, $4, $6, 0, 0); }
        ;

param_list_opt
        : /* empty */             { $$ = NULL; }
        | param_list              { $$ = $1; }
        ;

param_list
        : params                  { $$ = $1; }
        ;

params  : T_INT T_ID                             { $$ = ast_list_append(NULL, ast_var_decl($2,NULL,0,0)); }
        | params ',' T_INT T_ID                  { $$ = ast_list_append($1,    ast_var_decl($4,NULL,0,0)); }
        ;

compound: '{' stmt_list_opt '}'   {
            if(!$2){
                $$ = ast_make(AST_COMPOUND, 0, 0);
            } else {
                $$ = $2;
                $$->kind = AST_COMPOUND;
            }
        }
        ;

stmt_list_opt
        : /* empty */             { $$ = NULL; }
        | stmt_list_opt stmt      { $$ = ast_list_append($1, $2); }
        ;

stmt    : expr_stmt               { $$ = $1; }
        | if_stmt                 { $$ = $1; }
        | while_stmt              { $$ = $1; }
        | return_stmt             { $$ = $1; }
        | compound                { $$ = $1; }
        | var_decl                { $$ = $1; }
        ;

expr_stmt
        : expr ';'                { $$ = ast_exprstmt($1, 0, 0); }
        | ';'                     { $$ = ast_exprstmt(NULL, 0, 0); }
        ;

if_stmt : T_IF '(' expr ')' stmt %prec IFX      { $$ = ast_if($3, $5, NULL, 0, 0); }
        | T_IF '(' expr ')' stmt T_ELSE stmt    { $$ = ast_if($3, $5, $7,   0, 0); }
        ;

while_stmt
        : T_WHILE '(' expr ')' stmt             { $$ = ast_while($3, $5, 0, 0); }
        ;

return_stmt
        : T_RETURN expr ';'                     { $$ = ast_return($2, 0, 0); }
        ;

expr    : assignment               { $$ = $1; }
        ;

assignment
        : T_ID '=' assignment      { $$ = ast_assign($1, $3, 0, 0); }
        | logic_or                 { $$ = $1; }
        ;

logic_or
        : logic_or T_OR logic_and  { $$ = ast_bin(OP_OR,  $1, $3, 0, 0); }
        | logic_and                { $$ = $1; }
        ;

logic_and
        : logic_and T_AND equality { $$ = ast_bin(OP_AND, $1, $3, 0, 0); }
        | equality                 { $$ = $1; }
        ;

equality
        : equality T_EQ rel        { $$ = ast_bin(OP_EQ,  $1, $3, 0, 0); }
        | equality T_NE rel        { $$ = ast_bin(OP_NE,  $1, $3, 0, 0); }
        | rel                      { $$ = $1; }
        ;

rel     : rel '<' add             { $$ = ast_bin(OP_LT, $1, $3, 0, 0); }
        | rel '>' add             { $$ = ast_bin(OP_GT, $1, $3, 0, 0); }
        | rel T_LE add            { $$ = ast_bin(OP_LE, $1, $3, 0, 0); }
        | rel T_GE add            { $$ = ast_bin(OP_GE, $1, $3, 0, 0); }
        | add                     { $$ = $1; }
        ;

add     : add '+' mul             { $$ = ast_bin(OP_ADD, $1, $3, 0, 0); }
        | add '-' mul             { $$ = ast_bin(OP_SUB, $1, $3, 0, 0); }
        | mul                     { $$ = $1; }
        ;

mul     : mul '*' unary           { $$ = ast_bin(OP_MUL, $1, $3, 0, 0); }
        | mul '/' unary           { $$ = ast_bin(OP_DIV, $1, $3, 0, 0); }
        | mul '%' unary           { $$ = ast_bin(OP_MOD, $1, $3, 0, 0); }
        | unary                   { $$ = $1; }
        ;

unary   : '+' unary %prec UPLUS   { $$ = ast_un(OP_POS, $2, 0, 0); }
        | '-' unary %prec UMINUS  { $$ = ast_un(OP_NEG, $2, 0, 0); }
        | '!' unary               { $$ = ast_un(OP_NOT, $2, 0, 0); }
        | primary                 { $$ = $1; }
        ;

primary : T_NUM                   { $$ = ast_num($1, 0, 0); }
        | T_ID                    { $$ = ast_id($1, 0, 0); }
        | '(' expr ')'            { $$ = $2; }
        ;

%%

void yyerror(const char* s){
    fprintf(stderr, "Parse error: %s\n", s);
}
