#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"

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

static void execbuiltin(enum bifs ftype, Expression *exp, int nargs)
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

static Expression callbuiltin(struct fncall *f)
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

static Expression calluser(struct ufncall *f)
{
	struct symbol *fn = f->s;
	struct symlist *sl;
	struct ast *args = f->l;
	Expression exp;
	return exp;
}

static bool expToBool(Expression exp) {
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
