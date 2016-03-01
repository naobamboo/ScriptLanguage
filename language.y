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
%type <a> factor
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

factor
	: '(' exp ')' { $$ = $2; }
	| NUMBER { $$ = newnum($1); }
	;
exp
	: factor '+' factor { $$ = newast(factor, "+", $1, $3); }
	| factor '-' factor { $$ = newast(factor, "-", $1, $3); }
	| factor '*' factor { $$ = newast(factor, "*", $1, $3); }
	| factor '/' factor { $$ = newast(factor, "/", $1, $3); }
	;
%%
