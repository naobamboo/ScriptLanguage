#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "language.h"

extern int yyparse(void);

void
yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	fprintf(stderr, "%d: error: ", yylineno);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
}

int
main()
{
	printf("> ");
	return yyparse();
}
