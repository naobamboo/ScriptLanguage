#ifndef __SCRIPTLANGUAGE_EVAL__
#define __SCRIPTLANGUAGE_EVAL__

#include <stdbool.h>
#include "language.h"

Expression eval(struct ast* a);
static Expression callbuiltin(struct fncall *f);
static void execbuiltin(enum bifs ftype, Expression *exp, int nargs);
static Expression calluser(struct ufncall *f);
static bool expToBool(Expression exp); 

#endif 
