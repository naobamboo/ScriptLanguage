%{
#include <stdio.h>
#include <string.h>
#include "language.h"
#define YYERROR_VERBOSE 1
#define YYDEBUG 1

extern int yylex(void);

int yywrap()
{
	return 1;
}
%}

%union {
	char* str;
	struct ast *a;
	struct symbol *sym;
	int fn;
}

%token <str> NUMBER
%token <sym> IDENT
%token <str> STRING
%token <fn> FUNC
%token EOL
%type <a> factor exp explist let call stmt stmts

%left '+' '-'
%left '*' '/'
%right '='

%%
stmts
	: stmt
	| stmt stmts
	;
stmt
	: let EOL { eval($1) }
	| call EOL { eval($1) }
	;
let
	: IDENT '=' exp ';'{ $$ = newasgn($1, $3); }
	;
call
	: IDENT '(' explist ')' ';' { $$ = newcall($1, $3); }
	| FUNC '(' explist ')' ';' { $$ = newfunc($1, $3); }
	;
explist
	: exp ',' explist { $$ = newast(NODE_EXPLIST, NULL,  $1, $3); }
	| exp
	;
exp
	: factor
	| factor '+' factor { $$ = newast(NODE_EXP, "+", $1, $3); }
	| factor '-' factor { $$ = newast(NODE_EXP, "-", $1, $3); }
	| factor '*' factor { $$ = newast(NODE_EXP, "*", $1, $3); }
	| factor '/' factor { $$ = newast(NODE_EXP, "/", $1, $3); }
	;
factor
	: '(' exp ')' { $$ = $2; }
	| NUMBER { $$ = newnum($1); }
	| IDENT { $$ = newref($1); }
	| STRING { $$ = newast(NODE_STR, $1, NULL, NULL); }
	;
%%
