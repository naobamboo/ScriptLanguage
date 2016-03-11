#ifndef __SCRIPTLANGUAGE_LANGUAGE__
#define __SCRIPTLANGUAGE_LANGUAGE__

enum nodetype {
	NODE_INT = 1,
	NODE_FLOAT,
	NODE_STR,
	NODE_EXP,
	NODE_EXPLIST,
	NODE_SYMBOL,
	NODE_LET,
	NODE_FUNC,
	NODE_CALL,
	NODE_WHILE,
	NODE_IF,
	NODE_ELSE,
	NODE_SELECT,
	NODE_STMTS
};

enum symboltype {
	SYMBOL_NIL = 0,
	SYMBOL_INT,
	SYMBOL_FLOAT,
	SYMBOL_STR
};

enum expresstype {
	EXP_NIL = 1,
	EXP_INT,
	EXP_FLOAT,
	EXP_STR
};

enum bifs {
    B_print = 1
};

struct symbol {
    enum symboltype stype;
    char* name;
    union _su {
        int i;
        double d;
        char* s;
    } su;
	struct ast * func;
	struct symlist *syms;
};

struct symlist {
	struct symbol *sym;
	struct symlist *next;
};

typedef struct expression {
    enum expresstype etype;
    union _eu {
        int i;
				double d;
        char* s;
        struct symbol *sym;
    } eu;
} Expression;

struct ast {
	enum nodetype ntype;
	char* contents;
	struct ast *l;
	struct ast *r;
};

struct numval {
	enum nodetype ntype;
	union _nu {
		int i;
		double d;
	} nu;
};

struct symasgn {
    enum nodetype ntype;
    struct symbol *sym;
    struct ast *v; /* value */
};

struct symref {
    enum nodetype ntype;
    struct symbol *sym;
};

struct flow {
	enum nodetype ntype;
	struct ast *cond;
	struct ast *tl;
	struct ast *el;
};

struct fncall {
    enum nodetype ntype;
    struct ast *l;
    enum bifs ftype;
};

struct ufncall {
    enum nodetype ntype;
    struct ast *l;
    struct symbol *s;
};

extern int yylineno;
void yyerror(char *s, ...);

#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol * lookup(char* sym);

struct ast *newast(enum nodetype type, char* contents, struct ast *l, struct ast *r);
struct ast *newnum(char *s);
struct ast *newref(struct symbol *sym);
struct ast *newasgn(struct symbol *sym, struct ast *v);
struct ast *newfunc(enum bifs ftype, struct ast *l);
struct ast *newcall(struct symbol *s,  struct ast *l);
struct ast *newflow(enum nodetype ntype, struct ast *cond, struct ast *tl, struct ast *el);

void dodef(struct symbol *name, struct symlist *syms, struct ast *func);
struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);
void treefree(struct ast *);

#endif
