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

static unsigned
symhash(char *sym)
{
    unsigned int hash = 0;
    unsigned c;

    while(c == *sym++) hash = hash*9 ^c;

    return hash;
}

struct symbol *
lookup(char* sym)
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

struct ast*
newast(enum nodetype ntype, char* contents, struct ast *l, struct ast *r)
{
	struct ast *a = malloc(sizeof(struct ast));
	a->ntype = ntype;
	a->contents = contents;
	a->l = l;
	a->r = r;
	return a;
}

struct ast*
newnum(int n)
{
	struct numval *a = malloc(sizeof(struct numval));
	a->ntype = NODE_NUMBER;
	a->number = n;
	return (struct ast*)a;
}

struct ast*
newref(struct symbol *sym)
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

struct ast*
newasgn(struct symbol *sym, struct ast *v)
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

struct ast *
newfunc(enum bifs ftype,  struct ast *l)
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


struct ast*
newcall(struct symbol *sym, struct ast *l)
{
    struct ufncall *a = malloc(sizeof(struct ufncall));

    a->ntype = NODE_CALL;
    a->l = l;
    a->sym =sym;
    return (struct ast *)a;
}

struct ast*
newstmt(struct ast *l)
{
    struct stmt *a = malloc(sizeof(struct stmt));
    a->ntype = NODE_STMT;
    a->l = l;
    return (struct ast *)a;
}

static Expression
callbuiltin(struct fncall *f)
{
    enum bifs ftype = f->ftype;
    Expression exp = eval(f->l);

    switch(ftype) {
        case B_print:
            if(exp.etype == EXP_NUMBER) {
                printf("%d\n", exp.eu.i);
            } else if (exp.etype == EXP_STR) {
                printf("%s\n", exp.eu.s);
            }
            break;
    }
    return exp;
}

Expression
eval(struct ast *a)
{
    Expression exp;
    Expression left;
    Expression right;
    struct symbol *sym;
	switch(a->ntype) {
		case NODE_NUMBER:
            exp.etype = EXP_NUMBER;
            exp.eu.i = ((struct numval*)a)->number;
			break;
		case NODE_EXP:
			left = eval(a->l);
			right = eval(a->r);
            exp.etype = EXP_NUMBER;
			if (strcmp(a->contents, "+") == 0) {
                exp.eu.i =  left.eu.i + right.eu.i;
            }
			if (strcmp(a->contents, "-") == 0) {
                exp.eu.i =  left.eu.i - right.eu.i;
            }
			if (strcmp(a->contents, "*") == 0) {
                exp.eu.i =  left.eu.i * right.eu.i;
            }
			if (strcmp(a->contents, "/") == 0) {
                exp.eu.i =  left.eu.i / right.eu.i;
            }
			break;
        case NODE_STR:
            exp.etype = EXP_STR;
            size_t l = strlen(a->contents) - 4;
            exp.eu.s = calloc(1, l + 1);
            strncpy(exp.eu.s, a->contents + 1, l);
            exp.eu.s[l] = 0;
            break;
        case NODE_LET:
            exp = eval(((struct symasgn *)a)->v);
            if (exp.etype == EXP_NUMBER) {
                ((struct symasgn *)a)->sym->stype = SYMBOL_NUMBER;
                ((struct symasgn *)a)->sym->su.i = exp.eu.i;
            } else if (exp.etype == EXP_STR) {
                ((struct symasgn *)a)->sym->stype = SYMBOL_STR;
                ((struct symasgn *)a)->sym->su.s = exp.eu.s;
            } else {
            }
            break;
        case NODE_SYMBOL:
            sym = ((struct symref *)a)->sym;
            switch (sym->stype) {
                case SYMBOL_NUMBER:
                    exp.etype = EXP_NUMBER;
                    exp.eu.i = sym->su.i;
                    break;
                case SYMBOL_STR:
                    exp.etype = EXP_STR;
                    exp.eu.s = sym->su.s;
                    break;
                case SYMBOL_NIL:
                    break;
            }
            break;
        case NODE_EXPLIST:
            eval(a->l);
            exp = eval(a->r);
            break;
        case NODE_CALL:
            break;
        case NODE_FUNC:
            exp = callbuiltin((struct fncall *)a);
            break;
        case NODE_STMT:
            exp = eval(a->l);
            break;
        case NODE_STMTS:
            eval(a->l);
            exp = eval(a->r);
            break;
	}
	return exp;
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
