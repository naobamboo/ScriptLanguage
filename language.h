enum nodetype {
	number = 1,
    factor,
	exp
} nodetype;

struct ast {
	int nodetype;
	char* contents;
	struct ast *l;
	struct ast *r;
};

struct numval {
	int nodetype;
	int number;
};

extern int yylineno;
void yyerror(char *s, ...);

int eval(struct ast* a);
struct ast* newast(int nodetype, char* contents, struct ast *l, struct ast *r);
struct ast* newnum(int n);
