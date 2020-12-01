/* asm8085 (C) 2019 Marinus Oosters */

#include "macro.h"

#define ERROR "%s: line %d: "

// Free data structures
void free_macro(struct macro *macro) {
    if (macro == NULL) return;
    free_line(macro->header, FALSE);
    free_line(macro->body, TRUE);
    free(macro->name);
    free(macro);
}

void free_maclist(struct maclist *maclist) {
    if (maclist == NULL) return;
    struct maclist *item, *next = maclist;
    while (next != NULL) {
        item = next;
        next = item->next;
        free_macro(item->macro);
        free(item);
    }
}

// Trim a string and strip the first outer pair of brackets from it if there are any
char *trim_strip_brackets(const char *s) {
    char *trim = trim_string(s);
    size_t length = strlen(trim);
    if (trim[0] == '(' && trim[length-1] == ')') {
        // return string w/o brackets
        char *out = malloc(sizeof(char) * (length-1));
        if (out == NULL) FATAL_ERROR("failed to allocate space for string while removing brackets");
        memcpy(out, trim+1, length-2);
        out[length-2] = '\0';
        free(trim);
        return out;
    } else {
        // return string as is
        return trim;
    }
}
        
// Argument sorting compare function
int replacement_compare(const void *a, const void *b) {
    struct replacement *ra = (struct replacement *)a;
    struct replacement *rb = (struct replacement *)b;
    
    // Sort by length of argument to replace, descending
    size_t ralen = strlen(ra->old);
    size_t rblen = strlen(rb->old);
    
    return (ralen<rblen) - (ralen>rblen);
}


// Expand a macro, given the invocation on the given line
struct line *expand_macro(struct line *invocation, struct maclist *macros, struct line **last) {
    struct replacement replacements[MACRO_ARG_MAX + 1];
    char expansion_id[EXPANSION_ID_MAX_LEN] = {'\0'};
    char *tmp;
    char *new_line; 
    char error = FALSE, cur_error = FALSE;
    char errstr[128] = {'\0'};
 
    struct line *start_line = NULL, *line_prev = NULL, *line_cur = invocation, *line_mac; 
    
    // Sanity check
    if (invocation->instr.type != MACRO) {
        FATAL_ERROR("internal error: expand_macro called with non-macro line.");
    }
    
    // Find the macro
    struct macro *macro = find_macro(invocation->instr.text, macros);
    if (macro == NULL) {
        error_on_line(invocation, "macro not found: %s", invocation->instr.text);
        return NULL;
    }
    
    int i, n_argmts = invocation->n_argmts;
    const struct argmt *mac_argptr = macro->header->argmts;
    const struct argmt *inv_argptr = invocation->argmts;
    
    // Check arguments 
    if (macro->header->n_argmts != n_argmts) {
        error_on_line(invocation, "macro %s requires %d arguments, but got %d", n_argmts, macro->header->n_argmts);
        return NULL;
    }
    
    // Set up expansion ID
    replacements[0].old = "@";
    replacements[0].new = expansion_id;
    if (snprintf(expansion_id, EXPANSION_ID_MAX_LEN, EXPANSION_TEMPLATE, macro->name, ++macro->expansions) 
            >= EXPANSION_ID_MAX_LEN) {
        FATAL_ERROR("Maximum length exceeded for unique macro expansion identifier.\n"
                    EXPANSION_TEMPLATE, macro->name, macro->expansions);
    }
    
    // Make error string
    strncpy(errstr, invocation->info.filename, 127);
    char *file_ends = strchr(errstr, ':');
    if (file_ends == NULL) file_ends = errstr + strlen(errstr);
    snprintf(file_ends, 128 - (file_ends - errstr), ": [%s]", invocation->instr.text);
        
    // Set up arguments
    for (i=1; i<=n_argmts; i++) {
        
        // sanity check
        if (mac_argptr == NULL) FATAL_ERROR("internal error: mac_argptr == NULL");
        if (inv_argptr == NULL) FATAL_ERROR("internal error: inv_argptr == NULL");
        
        // "old" = "!" + trim(arg)
        tmp = trim_string(mac_argptr->raw_text);
        replacements[i].old = join_strings("!", tmp);
        free(tmp);
        
        // "new" = input string, trimmed and with braces removed if there are any
        replacements[i].new = trim_strip_brackets(inv_argptr->raw_text);
        
        // advance argument pointers
        inv_argptr = inv_argptr->next_argmt;
        mac_argptr = mac_argptr->next_argmt;
    }
       
    // Sort the arguments by length descending (so that e.g. "!foo" isn't replaced before "!foobar"),
    // but leave '#' in the first position alone.
    qsort(&replacements[1], n_argmts, sizeof(struct replacement), replacement_compare);
    
    // Process each line in turn
    line_prev = NULL;
    start_line = NULL;
    for (line_mac=macro->body; line_mac != NULL; line_mac=line_mac->next_line) {
        // Macro argument substitution
        new_line = string_replace(line_mac->raw_text, replacements, n_argmts+1);
        cur_error = FALSE;
        line_cur = parse_line(new_line, line_prev, errstr, &cur_error);
        if (line_prev == NULL) line_cur->info.lineno = line_mac->info.lineno;
        
        if (cur_error) {
            fprintf(stderr, "expanded line: %s\n", new_line);
            error = TRUE;
        }
        free(new_line);
        if (start_line == NULL) {
            start_line = line_cur;
        }
        line_prev = line_cur;
    }
      
    if (error) {
        // Free the created lines and return NULL
        free_line(start_line, TRUE);
        *last = NULL;
        return NULL;
    } else {
        *last = line_cur;
        if (start_line == NULL) {
            // Empty macro definition: simply continue on the next line
            return line_cur->next_line;
        } else {
            return start_line;
        }
    }
}


// Read a macro definition
struct macro *define_macro(const struct line *definition, const struct line **endm) {
    char error = FALSE;
    
    // Sanity check
    if (definition->instr.type != DIRECTIVE && definition->instr.instr != DIR_macro) {
        FATAL_ERROR("define_macro was called with non-macro line");
    }
    
    // Macro must have label
    if (definition->label == NULL) {
        error_on_line(definition, "macro without label");
        return NULL;
    }
    
    // Macro label must not be nested
    if (definition->label[0] == '.') {
        error_on_line(definition, "macro label cannot be nested: %s", definition->label);
        return NULL;
    }
    
    // See if the maximum amount of arguments isn't exceeded
    if (definition->n_argmts > MACRO_ARG_MAX) {
        error_on_line(definition, "macro %s: too many arguments (maximum %d)",
                    definition->label, MACRO_ARG_MAX);
        return NULL;
    }
            
    
    // Find the end
    *endm = find_endm(definition, &error);
    if (error) {
        return NULL; // find_endm prints its own relevant error messages.
    }
        
    struct macro *macro = calloc(1, sizeof(struct macro));
    if (macro == NULL) FATAL_ERROR("failed to allocate memory for macro");
    
    macro->name = copy_string(definition->label);
    macro->header = copy_line(definition);
    macro->body = NULL;
    macro->expansions = 0;
    
    /* copy all the relevant lines into the macro definition */
    struct line *copy, *prev_copy = NULL;
    const struct line *line = definition->next_line;
    for (; line != *endm; line=line->next_line) {
        copy = copy_line(line);
        if (macro->body == NULL) {
            macro->body = copy;
        } else {
            prev_copy->next_line = copy;
        }
        prev_copy = copy;
    }
        
    return macro;
}
  
// Find a macro defintion
struct macro *find_macro(const char *name, const struct maclist *macros) {
    while (macros != NULL) {
        if (!strcmp(macros->macro->name, name)) return macros->macro;
        macros = macros->next;
    }
    return NULL;
}

// Add macro definition to list
struct maclist *add_macro(struct macro *macro, struct maclist *macros) {
    struct maclist *n = malloc(sizeof(struct maclist));
    if (n == NULL) FATAL_ERROR("failed to allocate memory for macro list entry");
    n->macro = macro;
    n->next = macros;
    return n;
}

// Find the location of the endm, given macro starting line
const struct line *find_endm(const struct line *start, char *error) {
    *error = FALSE;
    const struct line *line = start->next_line; 
    int endms = 1;
    while (line != NULL && endms>0) {
        // Check for nested macros (not allowed)
        if (line->instr.type == DIRECTIVE && line->instr.instr == DIR_macro) {
            error_on_line(line, "in macro %s: nested macros are not allowed", start->label);
            *error = TRUE;
            // but do take it into account when searching for the endm (to give better error messages)
            endms++;
        } else if (line->instr.type == DIRECTIVE && line->instr.instr == DIR_endm) {
            if (--endms) {
                error_on_line(line, "note: ENDM for invalid nested macro is here");
            }
            if (endms==0) break;
        }
        line=line->next_line;
    }
    if (line == NULL) {
        // we reached the end without finding the endm line
        error_on_line(start, "macro without endm: %s", start->label);
        *error = TRUE;
    }
    return line;
}

