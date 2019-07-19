/* asm8085 (C) 2019 Marinus Oosters */


#ifndef __MACRO_H__
#define __MACRO_H__

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "parser.h"

#define MACRO_ARG_MAX 16
#define EXPANSION_TEMPLATE "_%s_%d_"
#define EXPANSION_ID_MAX_LEN 128
#define MAX_EXPAND_LENGTH 2048

// Macro
struct macro {
    char *name;
    
    unsigned int expansions;
    struct line *header;
    struct line *body;
};

// Keep track of macros
struct maclist {
    struct macro *macro;
    struct maclist *next;
};


// Free data structures
void free_macro(struct macro *);
void free_maclist(struct maclist *);

// Expand a macro, given the invocation on the given line
struct line *expand_macro(struct line *invocation, struct maclist *macros);

// Read a macro definition, removing the definition from the given line list
struct macro *define_macro(const struct line *definition, const struct line **endm);

// Find a macro defintion
struct macro *find_macro(const char *name, const struct maclist *macros);

// Add macro definition to list
struct maclist *add_macro(struct macro *macro, struct maclist *macros);

// Find an endm, and report errors
const struct line *find_endm(const struct line *, char *error);

#endif 