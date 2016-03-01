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
[ \t] /* ignore white space */
"\n" { return EOL; }
. { yyerror("Mystery character %c\n", *yytext); }
%%
