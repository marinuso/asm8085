#ifndef __OPCODES_H__
#define __OPCODES_H__

#include "parser.h"
#include "assembler.h"
#include "expression.h"
#include "util.h"

#define _OP(name, _) int op_##name(struct asmstate *);
#include "instructions.h"

#endif