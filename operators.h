/* asm8085 (C) 2019 Marinus Oosters 
 *
 * operators.h
 * This file lists all the expression operators and keywords in one place, as well as their precedence.
 * Note that the parser tries them in order, so they must be in length order
 * (e.g. if '<' were to precede '<<' then '<<' would be parsed as '<' '<').
 */
 
 
#ifndef _OPR
#define _OPR(op, name, prcd, argsn)
#endif

#ifndef _KWD 
#define _KWD(kwd)
#endif

// keywords
_KWD(high)
_KWD(low)

// operator,name,precedence,valence
_OPR(!=,NE,       4,2)
_OPR(!, BOOL_NOT, 9,1)
_OPR(~, BIT_NOT,  9,1)
_OPR(*, MUL,      8,2)
_OPR(/, DIV,      8,2)
_OPR(%, MOD,      8,2)
_OPR(+, ADD,      7,2)
_OPR(-, SUB,      7,2)
_OPR(<<,SHL,      6,2)
_OPR(>>,SHR,      6,2)
_OPR(<=,LE,       5,2)
_OPR(>=,GE,       5,2)
_OPR(<, LT,       5,2)
_OPR(>, GT,       5,2)
_OPR(==,EQ,       4,2)
_OPR(&&,BOOL_AND, 2,2)
_OPR(||,BOOL_OR,  2,2)
_OPR(&, AND,      3,2)
_OPR(^, XOR,      3,2)
_OPR(|, OR,       3,2)


#undef _OPR
#undef _KWD
