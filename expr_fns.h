/* asm8085 (C) 2019 Marinus Oosters 
 *
 * Implementations of functions and operators for expressions */

#ifndef __EXPR_FNS_H__
#define __EXPR_FNS_H__

#include "util.h"

int eval_keyword(int kwdnum, int input);
int eval_operator(int opnum, int *inputs);

#define _KWD(kwd) int evalkwd_##kwd (int);
#define _OPR(op,name,prcd,argsn) int evalop_##name (int*);
#include "operators.h"

#endif
