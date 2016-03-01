#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "language.h"

extern int yyparse(void);
extern FILE* yyin;

void
yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	fprintf(stderr, "%d: error: ", yylineno);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
}

struct ast*
newast(int nodetype, char* contents, struct ast *l, struct ast *r)
{
	struct ast *a = malloc(sizeof(struct ast));
	a->nodetype =nodetype;
	a->contents = contents;
	a->l = l;
	a->r = r;
	return a;
}

struct ast* 
newnum(int n)
{
	struct numval *a = malloc(sizeof(struct numval));
	a->nodetype = number;
	a->number = n;
	return (struct ast*)a;
}

int
eval(struct ast *a)
{
	int v;
	int left;
	int right;
	switch(a->nodetype) {
		case number:
			v = ((struct numval*)a)->number;
			break;
		case exp:
			left = eval(a->l);
			right = eval(a->r);
			if (strcmp(a->contents, "+") == 0) { v = left + right; }
			if (strcmp(a->contents, "-") == 0) { v = left - right; }
			if (strcmp(a->contents, "*") == 0) { v = left * right; }
			if (strcmp(a->contents, "/") == 0) { v = left / right; }
			break;
	}
	return v;
}

int
main()
{
	if ((yyin = fopen("./input.txt", "r")) == NULL) {
		fprintf(stderr, "Can't open a input file\n");
		return 1;
	}
	return yyparse();
}
