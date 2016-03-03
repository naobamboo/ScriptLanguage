%{
#include <stdio.h>
#include <stdlib.h>
#include "language.tab.h"
#include "language.h"
%}

%%
-?[0-9]+ { yylval.n = atoi(yytext); return NUMBER; }
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")" { return yytext[0]; }
"println" { yylval.fn = B_print; return FUNC; }
[a-zA-Z][a-zA-Z0-9_]* { yylval.sym = lookup(yytext); return IDENT; }
\"[^\"]*\" { yylval.str = yytext; return STRING; }
[ \t] /* ignore white space */
"\n" { return EOL; }
. { yyerror("Mystery character %c\n", *yytext); }
%%
