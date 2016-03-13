#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "language.h"
#include "eval.h"

extern int yyparse(void);
extern FILE* yyin;

void yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	fprintf(stderr, "%d: error: ", yylineno);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
}

static unsigned symhash(char *sym)
{
	unsigned int hash = 0;
	unsigned c;

	while(c == *sym++) hash = hash*9 ^c;

	return hash;
}

struct symbol * lookup(char* sym)
{
	struct symbol *sp = &symtab[symhash(sym)%NHASH];
	int scount = NHASH;

	while(--scount >= 0) {
		if(sp->name && !strcmp(sp->name,  sym)) { return sp; }

		if(!sp->name) {
			sp->name = strdup(sym);
			sp->stype = SYMBOL_NIL;
			sp->su.i = 0;
			sp->su.s = NULL;
			return sp;
		}
		if(++sp >= symtab+NHASH) sp = symtab;
	}
	yyerror("symbol table overflow\n");
	abort();
}

struct ast* newast(enum nodetype ntype, char* contents, struct ast *l, struct ast *r)
{
	struct ast *a = malloc(sizeof(struct ast));
	a->ntype = ntype;
	if (ntype == NODE_STR) {
		size_t len = strlen(contents) - 2;
		char* str = calloc(1, len + 1);
		strncpy(str, contents + 1, len);
		str[len] = 0;
		a->contents = str;
	} else {
		a->contents = contents;
	}
	a->l = l;
	a->r = r;
	return a;
}

struct ast* newnum(char* s)
{
	struct numval *a = malloc(sizeof(struct numval));
	if (!strchr (s, '.')) {
		a->ntype = NODE_INT;
		a->nu.i = atoi(s);
	} else {
		a->ntype = NODE_FLOAT;
		a->nu.d = atof(s);
	}
	return (struct ast*)a;
}

struct ast* newref(struct symbol *sym)
{
	struct symref *a = malloc(sizeof(struct symref));

	if(!a) {
		yyerror("out of space");
		exit(0);
	}
	a->ntype = NODE_SYMBOL;
	a->sym = sym;
	return (struct ast *)a;
}

struct symlist * newsymlist(struct symbol *sym, struct symlist *next)
{
	struct symlist *sl = malloc(sizeof(struct symlist));

	if(!sl) {
		yyerror("out of space");
		exit(0);
	}
	sl->sym = sym;
	sl->next = next;
	return sl;
}


struct ast* newasgn(struct symbol *sym, struct ast *v)
{
	struct symasgn *a = malloc(sizeof(struct symasgn));

	if(!a) {
		yyerror("out of space");
		exit(0);
	}
	a->ntype = NODE_LET;
	a->sym = sym;
	a->v = v;
	return (struct ast *)a;
}

struct ast * newfunc(enum bifs ftype,  struct ast *l)
{
	struct fncall *a = malloc(sizeof(struct fncall));

	if(!a) {
		yyerror("out of space");
		exit(0);
	}
	a->ntype = NODE_FUNC;
	a->l = l;
	a->ftype = ftype;
	return (struct ast *)a;
}


struct ast* newcall(struct symbol *s, struct ast *l)
{
	struct ufncall *a = malloc(sizeof(struct ufncall));
	if(!a) {
		yyerror("out of space");
		exit(0);
	}

	a->ntype = NODE_CALL;
	a->l = l;
	a->s = s;
	return (struct ast *)a;
}

struct ast* newflow(enum nodetype ntype, struct ast *cond, struct ast *tl, struct ast *el)
{
	struct flow *a = malloc(sizeof(struct flow));
	if(!a) {
		yyerror("out of space");
		exit(0);
	}
	a->ntype = ntype;
	a->cond = cond;
	a->tl = tl;
	a->el = el;
	return (struct ast *)a;
}

void symlistfree(struct  symlist *sl)
{
	struct symlist *nsl;

	while(sl) {
		nsl = sl->next;
		free(sl);
		sl = nsl;
	}
}

void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
	if(name->syms) symlistfree(name->syms);
	if(name->func) treefree(name->func);
	name->syms = syms;
	name->func = func;
}

void treefree(struct ast *a)
{
	switch(a->ntype) {
        case NODE_EXP:
		case NODE_EXPLIST:
			treefree(a->r);
			break;
		case NODE_FUNC:
		case NODE_CALL:
			treefree(a->l);
			break;
        case NODE_INT:
        case NODE_FLOAT:
        case NODE_STR:
        case NODE_SYMBOL:
            break;
        case NODE_IF:
        case NODE_WHILE:
        case NODE_SELECT:
        case NODE_STMTS:
            treefree(a->l);
            treefree(a->r);
            break;
        case NODE_ELSE:
            treefree(a->r);
            break;
        case NODE_LET:
            free( ((struct symasgn *)a)->v);
            break;
	}
    free(a);
}

int main(int argc, char** argv)
{
    char* sourcefile = "./sample.txt";

    if (argc == 2) {
        sourcefile = argv[1];
    }

	if ((yyin = fopen(sourcefile, "r")) == NULL) {
		fprintf(stderr, "Can't open a input file\n");
		return 1;
	}
	return yyparse();
}
