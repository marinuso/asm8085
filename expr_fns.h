/* asm8085 (C) 2019-20 Marinus Oosters 
 *
 * Implementations of functions and operators for expressions */

#ifndef __EXPR_FNS_H__
#define __EXPR_FNS_H__

#include "util.h"

intptr_t eval_keyword(int kwdnum, intptr_t input);
intptr_t eval_operator(int opnum, intptr_t *inputs);

#define _KWD(kwd) intptr_t evalkwd_##kwd (intptr_t);
#define _OPR(op,name,prcd,argsn) intptr_t evalop_##name (intptr_t *);
#include "operators.h"

#endif
