/* asm8085 (C) 2019-20 Marinus Oosters 
 *
 * Implementations of functions and operators for expressions */

#include "expr_fns.h"

intptr_t eval_keyword(int kwdnum, intptr_t input) {
    int k=0;
    #define _KWD(kwd) if(k==kwdnum) return evalkwd_##kwd (input); else k++;
    #include "operators.h"
    
    FATAL_ERROR("keyword does not exist #%d",kwdnum);
}

intptr_t eval_operator(int opnum, intptr_t *inputs) {
    int o=0;
    #define _OPR(op, name, prcd, argsn) if(o==opnum) return evalop_##name (inputs); else o++;
    #include "operators.h"
    
    FATAL_ERROR("operator does not exist #%d",opnum);
}

// keywords
intptr_t evalkwd_high(intptr_t input) {return ( ((unsigned) input) & 0xFF00) >> 8;}
intptr_t evalkwd_low(intptr_t input)  {return ( ((unsigned) input) & 0x00FF);}

// operators 

#define UNOP(name, op)  intptr_t evalop_##name (intptr_t *inputs) { return op *inputs; }
#define BINOP(name, op) intptr_t evalop_##name (intptr_t *inputs) { return inputs[0] op inputs[1]; }

BINOP(NE, !=)
UNOP(BOOL_NOT, !)
UNOP(BIT_NOT, ~)
UNOP(NEG, -)
BINOP(MUL, *)
BINOP(DIV, /)
BINOP(MOD, %)
BINOP(ADD, +)
BINOP(SUB, -)
BINOP(SHL, <<)
BINOP(SHR, >>)
BINOP(LE, <=)
BINOP(GE, >=)
BINOP(LT, <)
BINOP(GT, >)
BINOP(EQ, ==)
BINOP(BOOL_AND, &&)
BINOP(BOOL_OR, ||)
BINOP(AND, &)
BINOP(XOR, ^)
BINOP(OR, |)