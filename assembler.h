/* asm8085 (C) 2019 Marinus Oosters */


#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include "util.h"
#include "expression.h"
#include "parser.h"

#define MAX_INCLUDES 1024
#define RES_STACK_SIZE 8192


// Do some sanity checks 
char sanity_checks(const struct line *line);


#endif