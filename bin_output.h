/* asm8085 (C) 2020 Marinus Oosters */

#ifndef __BIN_OUTPUT_H__

#include <stdio.h>
#include "parser_types.h"
#include "util.h"


// Given a list of assembled lines, make binary output. 
// It is assume that the given buffer is at least 64K.
// Returns the amount of bytes written. 
size_t make_binary(const struct line *lines, unsigned char *buf);

#endif 