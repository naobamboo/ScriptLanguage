%{
#include <stdio.h>
#include <string.h>
#include "language.h"

extern int yylex(void);

int yywrap()
{
	return 1;
}
%}

%union {
	struct ast* a;
	int n;
}

%token <n> NUMBER
%token EOL
%type <a> exp

%left '+' '-'
%left '*' '/'

%%
lines
	: line
	| lines line
	;

line
	: EOL
	| exp EOL { printf("%d\n", eval($1)); }
	;

exp
	: NUMBER { $$ = newnum($1); }
	| '(' exp ')' { $$ = $2; }
	| exp '+' exp { $$ = newast(exp, "+", $1, $3); }
	| exp '-' exp { $$ = newast(exp, "-", $1, $3); }
	| exp '*' exp { $$ = newast(exp, "*", $1, $3); }
	| exp '/' exp { $$ = newast(exp, "/", $1, $3); }
	;
%%
