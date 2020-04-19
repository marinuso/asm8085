#ifndef __DIRECTIVES_H__
#define __DIRECTIVES_H__

#include "assembler.h"
#include "parser_types.h"

void no_asm_output(struct line *);

#define _DIR(dir) int dir_##dir(struct asmstate *);
#include "instructions.h" 

#endif
