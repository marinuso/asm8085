/* asm8085 (C) 2019 Marinus Oosters 
 *
 * Implementations of functions and operators for expressions */

#include "expr_fns.h"

int eval_keyword(int kwdnum, int input) {
    int k=0;
    #define _KWD(kwd) if(k==kwdnum) return evalkwd_##kwd (input); else k++;
    #include "operators.h"
    
    FATAL_ERROR("keyword does not exist #%d",kwdnum);
}

int eval_operator(int opnum, int *inputs) {
    int o=0;
    #define _OPR(op, name, prcd, argsn) if(o==opnum) return evalop_##name (inputs); else o++;
    #include "operators.h"
    
    FATAL_ERROR("operator does not exist #%d",opnum);
}

// keywords
int evalkwd_high(int input) {return ( ((unsigned) input) & 0xFF00) >> 8;}
int evalkwd_low(int input)  {return ( ((unsigned) input) & 0x00FF);}

// operators 

#define UNOP(name, op)  int evalop_##name (int *inputs) { return op *inputs; }
#define BINOP(name, op) int evalop_##name (int *inputs) { return inputs[0] op inputs[1]; }

BINOP(NE, !=)
UNOP(BOOL_NOT, !)
UNOP(BIT_NOT, ~)
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