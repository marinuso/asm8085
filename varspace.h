/* asm8085 (C) 2019 Marinus Oosters
 * 
 * varspace.h: keep track of labels and variables
 */
 
#ifndef __VARSPACE_H__
#define __VARSPACE_H__

#include "util.h"
#include <stdlib.h>

struct variable {
    char *name;
    int value;

    struct variable *next, *prev;
};

struct varspace {
    
    char *cur_base; // Current base name, if any
    struct variable *variables;

};

// Allocate a variable space
struct varspace *alloc_varspace();

// Free a variable space and all its associated variables.
void *free_varspace(struct varspace *);

// Get the value of a variable. Returns false if it doesn't exist. 
// The base is added if the name starts with a period.
char get_var(const struct varspace *, const char *name, int *value);

// Set the value of a variable. The base is added if the name starts with a period.
void set_var(struct varspace *, const char *name, int value);

// Delete a variable. Returns false if it didn't exist.
char del_var(struct varspace *, const char *name);

// Set the current base name
void set_base(struct varspace *, const char *base);

#endif