%option noyywrap

%{
#define _POSIX_C_SOURCE 200809L
#include "ast.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
%}

%%

"int"            { return T_INT; }
"if"             { return T_IF; }
"else"           { return T_ELSE; }
"while"          { return T_WHILE; }
"return"         { return T_RETURN; }

"=="             { return T_EQ; }
"!="             { return T_NE; }
"<="             { return T_LE; }
">="             { return T_GE; }
"&&"             { return T_AND; }
"||"             { return T_OR; }

[0-9]+                               { yylval.ival = strtol(yytext, NULL, 10); return T_NUM; }
[a-zA-Z_][a-zA-Z0-9_]*               { yylval.sval = strdup(yytext);          return T_ID;  }

"="              { return '='; }
"("              { return '('; }
")"              { return ')'; }
"{"              { return '{'; }
"}"              { return '}'; }
";"              { return ';'; }
","              { return ','; }
"!"              { return '!'; }
"+"              { return '+'; }
"-"              { return '-'; }
"*"              { return '*'; }
"/"              { return '/'; }
"%"              { return '%'; }
"<"              { return '<'; }
">"              { return '>'; }

[ \t\r\n]+                          { /* skip whitespace */ }

.                { fprintf(stderr, "Unknown char '%s'\n", yytext); }

%%
