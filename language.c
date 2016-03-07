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

static void
execbuiltin(enum bifs ftype, Expression exp)
{
    switch(ftype) {
        case B_print:
            switch (exp.etype) {
                case EXP_INT:
                    printf("%d", exp.eu.i);
                    break;
                case EXP_FLOAT:
                    printf("%f", exp.eu.d);
										break;
                case EXP_STR:
                    printf("%s", exp.eu.s);
                    break;
                case EXP_NIL:
                    break;
            }
            printf("\n");
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
    Expression exp;
    int i = 0;

    while(1) {
        if(args->ntype == NODE_EXPLIST) {
            exp = eval(args->l);
            execbuiltin(ftype, exp);
            args = args->r;
        } else {
            exp = eval(args);
            execbuiltin(ftype, exp);
            break;
        }
    }
    return ret;
}

static Expression
calluser(struct ufncall *f)
{
    Expression exp;
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
            size_t l = strlen(a->contents) - 4;
            exp.eu.s = calloc(1, l + 1);
            strncpy(exp.eu.s, a->contents + 1, l);
            exp.eu.s[l] = 0;
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
