enum nodetype {
	NODE_NUMBER = 1,
    NODE_STR,
	NODE_EXP,
    NODE_EXPLIST,
    NODE_SYMBOL,
    NODE_LET,
    NODE_FUNC,
    NODE_CALL,
    NODE_STMT,
    NODE_STMTS
};

enum symboltype {
    SYMBOL_NIL = 0,
    SYMBOL_NUMBER,
    SYMBOL_STR
};

enum expresstype {
    EXP_NUMBER = 1,
    EXP_STR,
    EXP_SYM
};

enum bifs {
    B_print = 1
};

struct symbol {
    enum symboltype stype;
    char* name;
    union _su {
        int i;
        char* s;
    } su;
};

typedef struct expression {
    enum expresstype etype;
    union _eu {
        int i;
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
	int number;
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

struct fncall {
    enum nodetype ntype;
    struct ast *l;
    enum bifs ftype;
};

struct ufncall {
    enum nodetype ntype;
    struct ast *l;
    struct symbol *sym;
};

struct stmt {
    enum nodetype ntype;
    struct ast *l;
};

extern int yylineno;
void yyerror(char *s, ...);

#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol * lookup(char* sym);

Expression eval(struct ast* a);
struct ast* newast(enum nodetype type, char* contents, struct ast *l, struct ast *r);
struct ast* newnum(int n);
struct ast* newref(struct symbol *sym);
struct ast* newasgn(struct symbol *sym, struct ast *v);
struct ast* newfunc(enum bifs ftype, struct ast *l);
struct ast* newcall(struct symbol *s,  struct ast *l);
struct ast* newstmt(struct ast *l);
