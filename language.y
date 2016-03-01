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
	int n;
}

%token <n> NUMBER
%token EOL
%type <n> exp

%left '+' '-'
%left '*' '/'

%%
lines
	: line
	| lines line
	;

line
	: EOL 
	| exp EOL { printf("%d\n", $1); }
	;

exp
	: NUMBER
	| '(' exp ')' { $$ = $2; }
	| exp '+' exp { $$ = $1 + $3; }
	| exp '-' exp { $$ = $1 - $3; }
	| exp '*' exp { $$ = $1 * $3; }
	| exp '/' exp { $$ = $1 / $3; }
	;
%%
