/* asm8085 (C) 2019 Marinus Oosters */


#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include "dirstack.h"
#include "util.h"
#include "expression.h"
#include "parser.h"
#include "macro.h"

#define MAX_INCLUDES 1024
#define RES_STACK_SIZE 8192
#define MAX_PATHLINE_SIZE (PATH_MAX + 128) 

#define INCLUDE_PRE "\tpushd\t\"%s\"\t; --- Including: %s"
#define INCLUDE_POST "\tpopd\t\t; --- End of include: %s"



// Assembler state
struct asmstate {
    struct maclist *macros;  // Holds the macros
    struct varspace *knowns; // Holds the known values, as values
    struct varspace *unknowns; // Holds the unknown values, pointers to lines where they are defined
    
    struct line *prev_line; // Holds a pointer to the previous line seen
    struct line *cur_line; // Holds a pointer to the current line 
    
    int n_includes; // count how many includes we've had
};
    
    
// Initialize assembler state
struct asmstate *init_asmstate();

// Free assembler state
void free_asmstate(struct asmstate *state);

// Do some sanity checks 
char sanity_checks(const struct line *line);

// Assemble lines
struct line *asm_lines(struct asmstate *state, struct line *lines);

// Assemble a file
struct line *assemble(struct asmstate *state, const char *filename);

// Evaluate all remaining expressions, and fill in the results
int complete(struct asmstate *state, struct line *lines);

#include "directives.h"
#include "opcodes.h"


#endif