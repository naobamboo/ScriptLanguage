%{
#include <stdio.h>
#include <stdlib.h>
#include "language.tab.h"
#include "language.h"
%}

%%
"while" { return WHILE; }
"elsif" { return ELSIF; }
"if" { return IF; }
"else" { return ELSE; }
-?[0-9]+"."[0-9]+ { yylval.str = yytext; return NUMBER; }
-?[0-9]+ { yylval.str = yytext; return NUMBER; }
"println" { yylval.fn = B_print; return FUNC; }
"\n"
[a-zA-Z][a-zA-Z0-9_]* { yylval.sym = lookup(yytext); return IDENT; }
\"(\\\\.|[^\"])*\" { yylval.str = yytext; return STRING; }
"//"[^\n]*\n /* ignore one line comment */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"{" |
"}" |
"(" |
")" { return yytext[0]; }
[ \t] /* ignore white space */
. { yyerror("Mystery character %c\n", *yytext); }
%%
