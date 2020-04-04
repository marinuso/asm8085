/* asm8085 (C) 2019-20 Marinus Oosters */

#include "varspace.h"

// Free a variable
void free_var(struct variable *v) {
    if (v == NULL) return;
    free(v->name);
    free(v);
}

// Add the base to the name if necessary
char *add_base(const struct varspace *vs, const char *name) {
    char *newname;
    
    // no . = no base
    if (name[0] != '.') return copy_string(name);
    if (vs->cur_base == NULL) return copy_string(name+1);
    
    size_t baselen = strlen(vs->cur_base); 
    size_t namelen = strlen(name);
    size_t bufsize = baselen + namelen + 1;
    newname = malloc(bufsize);
    if (newname == NULL) FATAL_ERROR("failed to allocate space for name");
    
    memcpy(newname, vs->cur_base, baselen);
    memcpy(newname+baselen, name, namelen);
    newname[bufsize-1] = '\0';
    return newname;
}


// Allocate space for a variable
struct variable *alloc_var(const struct varspace *vs, const char *name) {
    struct variable *v = calloc(1, sizeof(struct variable));
    if (v == NULL) FATAL_ERROR("failed to allocate space for variable");
    v->name = add_base(vs, name);
    return v;
}


// Find a variable, if it exists.
struct variable *find_var(const struct varspace *vs, const char *name) {
    
    struct variable *v = vs->variables;
    char *newname = add_base(vs, name);
    
    while (v != NULL) {
        if (!strcmp(v->name, newname)) break;
        v = v->next;
    }
    
    free(newname);
    return v;
}

// Allocate a variable space
struct varspace *alloc_varspace() {
    struct varspace *vs = calloc(1, sizeof(struct varspace));
    if (vs == NULL) FATAL_ERROR("failed to allocate space for list of variables");
    vs->isref = FALSE;
    return vs;
}

// Free a variable space and all its associated variables.
void free_varspace(struct varspace *vs) {
    // Make sure we're not freeing a temporary reference
    if (vs->isref) FATAL_ERROR("tried to free temporary reference to varspace");

    // Free all the variables first
    struct variable *var, *next = vs->variables;
    while (next != NULL) {
        var = next;
        next = var->next;
        free_var(var);
    }
    
    // Free the base name if we have one
    free(vs->cur_base);
    
    // Free the variable space itself
    free(vs);
}

// Get the value of a variable. Returns false if it doesn't exist. 
// The base is added if the name starts with a period.
char get_var(const struct varspace *vs, const char *name, intptr_t *value) {
    struct variable *v = find_var(vs, name);
    if (v == NULL) {
        return FALSE;
    } else {
        *value = v->value;
        return TRUE;
    }
}
    

// Set the value of a variable. The base is added if the name starts with a period.
void set_var(struct varspace *vs, const char *name, intptr_t value) {
    struct variable *v = find_var(vs, name);
    if (v == NULL) {
        // it doesn't exist yet, make it
        v = alloc_var(vs, name);
        v->next = vs->variables;
        v->prev = NULL;
        if (vs->variables != NULL) {
            vs->variables->prev = v;
        }
        vs->variables = v;
    }
    v->value = value;
}
        

// Delete a variable. Returns false if it didn't exist.
char del_var(struct varspace *vs, const char *name) {
    struct variable *v = find_var(vs, name);
    if (v == NULL) return FALSE;
    del_var_ptr(v, vs);
    return TRUE;
}

// Delete a variable, given a pointer to it
void del_var_ptr(struct variable *v, struct varspace *vs) {

    if (v == vs->variables) vs->variables = v->next;
    if (v->prev != NULL) v->prev->next = v->next;
    if (v->next != NULL) v->next->prev = v->prev;
    
    free_var(v);
}

// Set the current base name
void set_base(struct varspace *vs, const char *base) {
    if (vs->cur_base) free(vs->cur_base);
    vs->cur_base = copy_string(base);
}

// Make a temporary copy with a different name
// Note: name is not copied!
struct varspace temp_rename(const struct varspace *vs, char *name) {
    struct varspace v;
    v.isref = TRUE;
    v.cur_base = name;
    if (vs != NULL) {
        v.variables = vs->variables;
    } else {
        v.variables = NULL;
    }
    return v;
}



