#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "language.h"

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

	struct ast*
newnum(char* s)
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
	if(!a) {
		yyerror("out of space");
		exit(0);
	}

	a->ntype = NODE_CALL;
	a->l = l;
	a->sym =sym;
	return (struct ast *)a;
}

	struct ast*
newflow(enum nodetype ntype, struct ast *cond, struct ast *tl, struct ast *el)
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

	static void
execbuiltin(enum bifs ftype, Expression *exp, int nargs)
{
	switch(ftype) {
		case B_print:
			for(int i = 0; i < nargs; i++) {
				switch (exp[i].etype) {
					case EXP_INT:
						printf("%d", exp[i].eu.i);
						break;
					case EXP_FLOAT:
						printf("%f", exp[i].eu.d);
						break;
					case EXP_STR:
						printf("%s", exp[i].eu.s);
						break;
					case EXP_NIL:
						break;
				}
			}
			printf("\n");
			break;
		default:
			break;
	}
}

	static Expression
callbuiltin(struct fncall *f)
{
	enum bifs ftype = f->ftype;
	struct ast *args = f->l;
	Expression ret = { EXP_NIL };
	Expression *exp2;
	int  nargs = 0;
	int i = 0;

	while(1) {
		if(args->ntype == NODE_EXPLIST) {
			args = args->r;
			++nargs;
		} else {
			++nargs;
			break;
		}
	}

	Expression exp[nargs];
	if (nargs > 0) {
		args = f->l;
		while(1) {
			if(args->ntype == NODE_EXPLIST) {
				exp[i] = eval(args->l);
				args = args->r;
				i++;
			} else {
				exp[i] = eval(args);
				break;
			}
		}
	}
	execbuiltin(ftype, exp, nargs);
	return ret;
}

	static Expression
calluser(struct ufncall *f)
{
	Expression exp;
	return exp;
}

static bool
expToBool(Expression exp) {
	bool judge = true;
	switch(exp.etype) {
		case EXP_NIL:
			judge = false;
			break;
		case EXP_INT:
			if (exp.eu.i <= 0) judge = false;
			break;
		case EXP_FLOAT:
			if (exp.eu.d <= 0) judge = false;
			break;
		case EXP_STR:
			if (exp.eu.s == NULL) judge = false;
			break;
		default:
			break;
	}
	return judge;
}

Expression eval(struct ast *a)
{
	Expression exp;
	Expression left;
	Expression right;
	Expression cond;
	struct symbol *sym;
	switch(a->ntype) {
		case NODE_INT:
			exp.etype = EXP_INT;
			exp.eu.i = ((struct numval*)a)->nu.i;
			break;
		case NODE_FLOAT:
			exp.etype = EXP_FLOAT;
			exp.eu.d = ((struct numval*)a)->nu.d;
			break;
		case NODE_EXP:
			left = eval(a->l);
			right = eval(a->r);
			if (left.etype == EXP_INT && right.etype == EXP_INT) {
				exp.etype = EXP_INT;
				if (strcmp(a->contents, "+") == 0) {
					exp.eu.i = left.eu.i + right.eu.i;
				}
				if (strcmp(a->contents, "-") == 0) {
					exp.eu.i = left.eu.i - right.eu.i;
				}
				if (strcmp(a->contents, "*") == 0) {
					exp.eu.i = left.eu.i * right.eu.i;
				}
				if (strcmp(a->contents, "/") == 0) {
					exp.eu.i = left.eu.i / right.eu.i;
				}
			} else if (left.etype == EXP_INT && right.etype == EXP_FLOAT) {
				exp.etype = EXP_FLOAT;
				if (strcmp(a->contents, "+") == 0) {
					exp.eu.d = (double)left.eu.i + right.eu.d;
				}
				if (strcmp(a->contents, "-") == 0) {
					exp.eu.d = (double)left.eu.i - right.eu.d;
				}
				if (strcmp(a->contents, "*") == 0) {
					exp.eu.d = (double)left.eu.i * right.eu.d;
				}
				if (strcmp(a->contents, "/") == 0) {
					exp.eu.d = (double)left.eu.i / right.eu.d;
				}
			} else if (left.etype == EXP_FLOAT && right.etype == EXP_INT) {
				exp.etype = EXP_FLOAT;
				if (strcmp(a->contents, "+") == 0) {
					exp.eu.d = left.eu.d + (double)right.eu.i;
				}
				if (strcmp(a->contents, "-") == 0) {
					exp.eu.d = left.eu.d - (double)right.eu.i;
				}
				if (strcmp(a->contents, "*") == 0) {
					exp.eu.d = left.eu.d * (double)right.eu.i;
				}
				if (strcmp(a->contents, "/") == 0) {
					exp.eu.d = left.eu.d / (double)right.eu.i;
				}
			} else if (left.etype == EXP_FLOAT && right.etype == EXP_FLOAT) {
				exp.etype = EXP_FLOAT;
				if (strcmp(a->contents, "+") == 0) {
					exp.eu.d = left.eu.d + right.eu.d;
				}
				if (strcmp(a->contents, "-") == 0) {
					exp.eu.d = left.eu.d - right.eu.d;
				}
				if (strcmp(a->contents, "*") == 0) {
					exp.eu.d = left.eu.d * right.eu.d;
				}
				if (strcmp(a->contents, "/") == 0) {
					exp.eu.d = left.eu.d / right.eu.d;
				}
			}
			break;
		case NODE_STR:
			exp.etype = EXP_STR;
			exp.eu.s = a->contents;
			break;
		case NODE_LET:
			exp = eval(((struct symasgn *)a)->v);
			if (exp.etype == EXP_INT) {
				((struct symasgn *)a)->sym->stype = SYMBOL_INT;
				((struct symasgn *)a)->sym->su.i = exp.eu.i;
			} else if (exp.etype == EXP_FLOAT) {
				((struct symasgn *)a)->sym->stype = SYMBOL_FLOAT;
				((struct symasgn *)a)->sym->su.d = exp.eu.d;
			} else if (exp.etype == EXP_STR) {
				((struct symasgn *)a)->sym->stype = SYMBOL_STR;
				((struct symasgn *)a)->sym->su.s = exp.eu.s;
			} else {
			}
			break;
		case NODE_SYMBOL:
			sym = ((struct symref *)a)->sym;
			switch (sym->stype) {
				case SYMBOL_INT:
					exp.etype = EXP_INT;
					exp.eu.i = sym->su.i;
					break;
				case SYMBOL_FLOAT:
					exp.etype = EXP_FLOAT;
					exp.eu.d = sym->su.d;
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
		case NODE_FUNC:
			exp = callbuiltin((struct fncall *)a);
			break;
		case NODE_CALL:
			exp = calluser((struct ufncall *)a);
			break;
		case NODE_STMTS:
			eval(a->l);
			exp = eval(a->r);
			break;
		case NODE_WHILE:
			while (expToBool(eval(a->l))) {
				exp = eval(a->r);
			}
			break;
		case NODE_IF:
			exp = eval(a->l);
			if (expToBool(exp)) {
				eval(a->r);
			}
			break;
		case NODE_ELSE:
			exp = eval(a->r);
			break;
		case NODE_SELECT:
			exp = eval(a->l);
			if (!expToBool(exp)) {
				exp = eval(a->r);
			} 
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
