%{
#include <stdio.h>
#include <string.h>
#include "language.h"
#include "eval.h"
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
	struct symlist *sl;
	int fn;
}

%token <str> NUMBER
%token <sym> IDENT
%token <str> STRING
%token <fn> FUNC
%token WHILE
%token IF
%token ELSE
%token ELSIF
%token DECLARE
%type <a> factor
%type <a> exp explist
%type <sl> symlist
%type <a> let call declare  if else elsif elsiflist elselist ifstmt
%type <a> stmt stmts
%type <a> program


%left '+' '-'
%left '*' '/'
%right '='

%%
program
	: stmts { eval($1); }
	;
stmts 
	: stmt
	| stmt stmts { $$ = newast(NODE_STMTS, NULL,  $1, $2); }
	;
stmt
	: let 
	| call
	| declare
	| WHILE '(' exp ')' '{' stmts '}' { $$ = newast(NODE_WHILE, NULL, $3, $6); }
	| ifstmt
	;
let
	: IDENT '=' exp ';'{ $$ = newasgn($1, $3); }
	;
call
	: IDENT '(' explist ')' ';' { $$ = newcall($1, $3); }
	| FUNC '(' explist ')' ';' { $$ = newfunc($1, $3); }
	;
declare
	: DECLARE IDENT '(' symlist ')' '{' stmts '}' { dodef($2, $4, $7); }
	;
ifstmt
	: if
	| if elselist { $$ = newast(NODE_SELECT, NULL, $1, $2); } 
	;
elselist
	: elsiflist
	| else
	| elsiflist else { $$ = newast(NODE_SELECT, NULL, $1, $2); }
	;
elsiflist
	: elsif
	| elsif elsiflist { $$ = newast(NODE_SELECT, NULL, $1, $2); }
	;
elsif
	: ELSIF '(' exp ')' '{' stmts '}' { $$ = newast(NODE_IF, NULL, $3, $6); }
	;
else
	: ELSE '{' stmts '}' { $$ = newast(NODE_ELSE, NULL, NULL, $3); }
	;
if
	: IF '(' exp ')' '{' stmts '}' { $$ = newast(NODE_IF, NULL, $3, $6); }
	;
symlist
	: /* empty */
	| IDENT { $$ = newsymlist($1, NULL); }
	| IDENT ',' symlist { $$ = newsymlist($1, $3); }
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
