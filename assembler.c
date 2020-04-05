/* asm8085 (C) 2019-20 Marinus Oosters */

#include "assembler.h"

#define ERROR "%s: line %d: "

// Output on error: "<file>: line <line>: error\n"
static void error_on_line(const struct line *line, char *message, ...) {
    fprintf(stderr, ERROR, line->info.filename, line->info.lineno);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

// Output on error: "<file>: error\n"
static void error_in_file(const struct line *line, char *message, ...) {
    fprintf(stderr, "%s: ", line->info.filename);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}


// Sanity checks
char sanity_checks(const struct line *line) {
    char error = FALSE;
    
    int macdepth = 0;
    int i;
    char *l;
    
    for (; line != NULL; line=line->next_line) {
        if (line->instr.type == DIRECTIVE) {
            // Macro without name?
            i = line->instr.instr;
            if (i == DIR_macro) {
                if (line->label == NULL) { error_on_line(line, "macro without name"); error = TRUE; }
                macdepth++;
                if (macdepth<0) macdepth=1;
            }
            // equ without name
            else if (i == DIR_equ) {
                if (line->label == NULL) { error_on_line(line, "equ without name"); error=TRUE; }
            }
            // if/if(n)def/endm with name?
            else if (i == DIR_endm) {
                if (line->label != NULL) { error_on_line(line, "named endm"); error = TRUE; }
                macdepth--;
                if (macdepth < 0) { error_on_line(line, "endm without macro"); error = TRUE; }
                
            } else if (i == DIR_if || i==DIR_ifdef || i==DIR_endif || i==DIR_ifndef) {
                if (line->label != NULL) { error_on_line(line, "named if/endif"); error = TRUE; }
            }
        } else if (line->label != NULL) {
            // Label names OK?
            l = line->label;
            if (l[0] == '@') {
                if (macdepth <= 0) {
                    error_on_line(line, "@-label outside of macro");
                    error = TRUE;
                }
                l++;
            } else if (l[0] != '_' && !isalpha(l[0])) {
                error_on_line(line, "label name must start with letter or underscore: %s", line->label);
                error = TRUE;
            }
            for (l++; *l; l++) {
                if (*l != '_' && !isalnum(*l)) {
                    error_on_line(line, "invalid label name: %s", line->label);
                    error = TRUE;
                    break;
                }
            }
        }
    }
    
    return error;
}

// Create a new assembler state
struct asmstate *init_asmstate() {
    struct asmstate *state;
    if ((state = malloc(sizeof(struct asmstate))) == NULL) {
        FATAL_ERROR("could not allocate memory for assembly state");
    }
    
    state->macros = NULL;
    state->knowns = alloc_varspace();
    state->unknowns = alloc_varspace();
    state->prev_line = NULL;

    return state;
}

// Free assembler state
void free_asmstate(struct asmstate *state) {
    if (state != NULL) {   
        free_maclist(state->macros);
        free_varspace(state->knowns);
        free_varspace(state->unknowns);
        free(state);
    }
}


// Resolve all resolvable variables
void resolve_all(struct asmstate *state) {
    int resolved;
    struct line *resolve_line;
    struct variable *va, *dva;
    struct parsed_expr *expr;
    intptr_t answer;
    
    do {
        resolved = FALSE;
        va = state->unknowns->variables;
        while (va != NULL) {
            // Get unresolved equation
            dva = va;
            va = va->next;
            resolve_line = (struct line *) dva->value;
            
            
           
            // This line should be 'EQU' with one, already parsed, expression argument
            if (resolve_line->instr.type != DIRECTIVE
            ||  resolve_line->instr.instr != DIR_equ
            ||  resolve_line->n_argmts != 1
            ||  resolve_line->argmts->parsed != TRUE
            ||  resolve_line->argmts->type != EXPRESSION) {
                FATAL_ERROR("unknowns contains pointer to non-equ");
            }
            
            // Do we know all the variables to evaluate the expression?
            expr = resolve_line->argmts->data.expr;
            if (!contains_undefined_names(expr, state->knowns)) {
                // We can resolve it
                answer = eval_expr(expr, state->knowns, &resolve_line->info, resolve_line->location);
                // That means it's known now
                set_var(state->knowns, dva->name, answer);
                resolved = TRUE;
                del_var_ptr(dva, state->unknowns);
            }
        }
    } while(resolved);
}

// Set the given line to have no length and no further processing needed
// (Most assembler directives)
void no_asm_output(struct line *line) {
    line->n_bytes = 0;
    line->needs_process = FALSE;
}

// Handle an 'include' directive
int dir_include(struct asmstate *state) {
    struct line *cur_line = state->cur_line;
    
    no_asm_output(cur_line);
 
    // Store the next line (the file will be inserted in between)
    struct line *next_line = cur_line->next_line;
                    
    if (state->n_includes++ >= MAX_INCLUDES) {
        error_on_line(cur_line, "maximum amount of includes exceeded.");
        return FALSE;
    }
    
    // See if we have the right arguments
    if (cur_line->n_argmts != 1 || cur_line->argmts->type != STRING) {
        error_on_line(cur_line, "'include' needs one string argument.");
        return FALSE;
    }

    // Read the file
    char *fname = cur_line->argmts->data.string;
    struct line *lines = read_file(fname);
    struct line *flastline = NULL;
    
    if (lines == NULL || sanity_checks(lines)) {
        error_on_line(cur_line, "include: failed: %s", fname);
        return FALSE;
    }
    
    // find the last line of the file
    for (flastline = lines; flastline->next_line != NULL; flastline = flastline->next_line);
    
    // to make the includes within the file work correctly, 
    // insert a 'PUSHD <curdir>' before the file, and a 'POPD' after.
    
    char *pre_include, *post_include;
    if (!(pre_include = malloc(MAX_PATHLINE_SIZE * sizeof(char))))
        FATAL_ERROR("failed to allocate memory for pre_include");
    if (!(post_include = malloc(MAX_PATHLINE_SIZE * sizeof(char))))
        FATAL_ERROR("failed to allocate memory for post_include");
    
    int pre_len, post_len;
    
    pre_len = snprintf(INCLUDE_PRE, MAX_PATHLINE_SIZE, dirname(fname), fname);
    post_len = snprintf(INCLUDE_POST, MAX_PATHLINE_SIZE, fname);
    
    if (pre_len < 0 || post_len < 0) 
        FATAL_ERROR("failed to make pre-include or post-include line");
    if (pre_len >= MAX_PATHLINE_SIZE || post_len >= MAX_PATHLINE_SIZE)
        error_on_line(cur_line, "include: maximum path length exceeded");
    
    char error = 0;
    struct line *pre_line = parse_line(pre_include, cur_line, cur_line->info.filename, &error);
    if (error || pre_line == NULL) FATAL_ERROR("failed to construct pre_line");
    error = 0;
    struct line *post_line = parse_line(post_include, flastline, flastline->info.filename, &error);
    if (error || post_line == NULL) FATAL_ERROR("failed to construct post_line");
    
    // connect up the lines
    pre_line->next_line = lines;
    post_line->next_line = next_line;
    
    free(pre_include);
    free(post_include);
    
    return TRUE;
        
}

// Handle an 'org' directive
int dir_org(struct asmstate *state) {
    struct line *cur_line = state->cur_line;

    no_asm_output(cur_line);
    
    // There must be one argument
    if (cur_line->n_argmts != 1) {
        error_on_line(cur_line, "org: needs one expression argument");
        return FALSE;
    }
    
    // The argument must be a valid expression
    if (!parse_argmt(EXPRESSION, cur_line->argmts, &cur_line->info)) {
        return FALSE;
    }
    
    // The expression must be fully defined at this point
    struct parsed_expr *expr = cur_line->argmts->data.expr;
    int newloc = eval_expr(expr, state->knowns, &cur_line->info, cur_line->location);
    
    if (contains_undefined_names(expr, state->knowns)) {
        // Not all values were defined
        error_on_line(cur_line, "org: all labels in expression must be fully defined previously");
        return FALSE;
    } else {
        // This is the new origin for this line
        cur_line->location = newloc;
        // If the line has a label, that label must be set to the new location.
        if (cur_line->label != NULL) {
            set_var(state->knowns, cur_line->label, newloc);
        }
        // It has gone OK.
        return TRUE;
    }
}
        
    
// 'db' = Define Bytes 
int dir_db(struct asmstate *state) {
    struct line *cur_line = state->cur_line;
    cur_line->needs_process = TRUE; // This line will need processing afterwards, to evaluate any expressions
    
    if (cur_line->n_argmts < 1) {
        error_on_line(cur_line, "db: needs at least one argument");
        return FALSE;
    }
    
    // Parse and count up all the arguments 
    cur_line->n_bytes = 0;
    struct argmt *arg;
    for (arg = cur_line->argmts; arg != NULL; arg = arg->next_argmt) {
        // Arguments must be either expression or string
        if (!parse_argmt(STRING|EXPRESSION, arg, &cur_line->info)) {
            return FALSE;
        }
        
        if (arg->type == STRING) {
            // A string is replicated byte by byte
            cur_line->n_bytes += strlen(arg->data.string);
        } else if (arg->type == EXPRESSION) {
            // An expression will be evaluated to a byte
            cur_line->n_bytes += 1;
        } else {
            FATAL_ERROR("STRING|EXPRESSION was neither string nor expression");
        }
    }
    
    // Allocate that many bytes, to be filled in during the last pass
    cur_line->bytes = malloc(cur_line->n_bytes);
    if (cur_line->bytes == NULL) {
        FATAL_ERROR("failed to allocate memory for db");
    }
    
    return TRUE;
}
    
// 'dw' = Define Words    
int dir_dw(struct asmstate *state) {
    struct line *cur_line = state->cur_line;
    cur_line->needs_process = TRUE;
    if (cur_line->n_argmts < 1) {
        error_on_line(cur_line, "dw: needs at least one argument");
        return FALSE;
    }
    
    cur_line->n_bytes = 0;
    struct argmt *arg;
    for (arg = cur_line->argmts; arg != NULL; arg = arg->next_argmt) {
        // Arguments must be expressions
        if (!parse_argmt(EXPRESSION, arg, &cur_line->info)) {
            return FALSE;
        }
        
        // Each argument represents two bytes
        cur_line->n_bytes += 2;
    }
    
    // Allocate space for it
    cur_line->bytes = malloc(cur_line->n_bytes);
    if (cur_line->bytes == NULL) {
        FATAL_ERROR("failed to allocate memory for dw");
    }
    
    return TRUE;
}

// 'ds' = Define Space 
int dir_ds(struct asmstate *state) {
    struct line *cur_line = state->cur_line;
    cur_line->needs_process = FALSE;
    
    if (cur_line->n_argmts != 1) {
        error_on_line(cur_line, "ds: no length given");
        return FALSE;
    }
    
    // The argument needs to be an expression
    struct argmt *arg = cur_line->argmts;
    if (!parse_argmt(EXPRESSION, arg, &cur_line->info)) {
        return FALSE;
    }
    
    // This expression needs to be fully specified
    struct parsed_expr *expr = arg->data.expr;
    cur_line->n_bytes = eval_expr(expr, state->knowns, &cur_line->info, cur_line->location);
    if (contains_undefined_names(expr, state->knowns)) {
        error_on_line(cur_line, "ds: all labels in size expression need to be fully defined previously");
        return FALSE;
    }
    
    // Allocate space and zero-fill it
    cur_line->bytes = calloc(cur_line->n_bytes, sizeof(char));
    if (cur_line->bytes == NULL) {
        FATAL_ERROR("failed to allocate memory for ds");
    }
    
    return TRUE;
}


// 'equ' = define label as expression
int dir_equ(struct asmstate *state) {
    struct line *cur_line = state->cur_line;
    no_asm_output(cur_line);
    
    if (cur_line->n_argmts != 1) {
        error_on_line(cur_line, "equ: one expression is needed");
        return FALSE;
    }
    
    if (!parse_argmt(EXPRESSION, cur_line->argmts, &cur_line->info)) {
        return FALSE;
    }
    
    if (cur_line->label == NULL) {
        FATAL_ERROR("equ without label - should have already been prevented");
    }
    
    struct parsed_expr *expr = cur_line->argmts->data.expr;
    if (contains_undefined_names(expr, state->knowns)) {
        // The expression cannot yet be fully evaluated, but we know it exists.
        // Store it as an 'unknown', to be resolved when all names are defined.
        set_var(state->unknowns, cur_line->label, (intptr_t) cur_line);
    } else {
        // We have all names necessary to evaluate the expression, so do so and
        // store the value.
        set_var(state->knowns, cur_line->label, 
            eval_expr(expr, state->knowns, &cur_line->info, cur_line->location));
    }
    
    return TRUE;
}
    
    
// Macro definition 
int dir_macro(struct asmstate *state) {
    
    // Find the macro lines 
    struct line *macro_start = state->cur_line;
    const struct line *endm;
    struct macro *macro;
    macro = define_macro(macro_start, &endm);
    if (macro == NULL) return FALSE;
    
    // Cut the macro definition out of the line list
    state->cur_line = state->prev_line;
    state->cur_line->next_line = endm->next_line;
    
    // Add the macro to the macro list
    state->macros = add_macro(macro, state->macros);
    
    return TRUE;
}

int dir_endm(__attribute__((unused)) struct asmstate *state) {
    // This should never happen as sanity_checks() has already checked the nesting
    FATAL_ERROR("endm reached during assembly");
}


// Given a line with an 'if((n)def)', find the line before the corresponding endif
struct line *find_endif(struct line *start) {
    struct line *p, *s = start;
    int depth = 1;
   
    while (depth > 0) {
        p = s;
        s = s->next_line;
        if (s == NULL) {
            error_on_line(start, "if without endif");
            return NULL;
        } else if (s->instr.type == DIRECTIVE) {
            if (s->instr.instr == DIR_if
            ||  s->instr.instr == DIR_ifdef
            ||  s->instr.instr == DIR_ifndef) {
                depth++;
            } else if (s->instr.instr == DIR_endif) {
                depth--;
            }
        }
    }
    
    return p;
}   

// Given a line with an 'if((n)def)' on it, either remove it and its corresponding
// endif (to accept), or remove the entire block (to reject)
int handle_if(struct asmstate *state, int accept) {
    // Find the corresponding endif
    struct line *next_endif = find_endif(state->cur_line);
    if (next_endif == NULL) return FALSE;
    
    if (accept) {
        // The statement is true, so assembly should proceed as if the 'if' and
        // 'endif' just weren't there
        
        // Remove the 'if'
        state->prev_line->next_line = state->cur_line->next_line;
        state->cur_line = state->prev_line;
        
        // Remove the 'endif'
        next_endif->next_line = next_endif->next_line->next_line;
        
        return TRUE;
    } else {
        // The statement is false, so assembly should proceed as if the 'if' and
        // 'endif', and the whole block of code in between, wasn't there
        
        state->prev_line->next_line = next_endif->next_line->next_line;
        state->cur_line = state->prev_line;
        return TRUE;
    }
}    

int dir_if(struct asmstate *state) {
    struct line *cur = state->cur_line;
    no_asm_output(cur);

    // The current line should have an expression on it
    if (cur->n_argmts != 1) {
        error_on_line(cur, "if: needs one condition argument");
        return FALSE;
    }
    
    // The expression should be valid and resolvable
    if (!parse_argmt(EXPRESSION, cur->argmts, &cur->info)) {
        return FALSE;
    }
    
    int cond = eval_expr(cur->argmts->data.expr, state->knowns, &cur->info, cur->location);
    if (contains_undefined_names(cur->argmts->data.expr, state->knowns)) {
        error_on_line(cur, "if: all labels in condition expression need to be fully defined previously");
        return FALSE;
    } else {
        return handle_if(state, cond);
    }
}
    
int dir_ifdef(struct asmstate *state) {
    struct line *cur = state->cur_line;
    no_asm_output(cur);

    // The current line should have an expression on it
    if (cur->n_argmts != 1) {
        error_on_line(cur, "ifdef: needs one condition argument");
        return FALSE;
    }
    
    // The expression should be valid
    if (!parse_argmt(EXPRESSION, cur->argmts, &cur->info)) {
        return FALSE;
    }
    
    // The condition is true if all names are known
    return handle_if(state, !contains_undefined_names(cur->argmts->data.expr, state->knowns));
}    
    
int dir_ifndef(struct asmstate *state) {
    struct line *cur = state->cur_line;
    no_asm_output(cur);

    // The current line should have an expression on it
    if (cur->n_argmts != 1) {
        error_on_line(cur, "ifndef: needs one condition argument");
        return FALSE;
    }
    
    // The expression should be valid
    if (!parse_argmt(EXPRESSION, cur->argmts, &cur->info)) {
        return FALSE;
    }
    
    // The condition is true if not all names are known
    return handle_if(state, contains_undefined_names(cur->argmts->data.expr, state->knowns));
}    

int dir_endif(struct asmstate *state) {
    // If an endif is encountered, that means it is not balanced
    error_on_line(state->cur_line, "endif without if");
    return FALSE; 
}

// pushd and popd
int dir_pushd(struct asmstate *state) {    
    struct line *cur = state->cur_line;
    no_asm_output(cur);
    
    // need one string argument
    if (cur->n_argmts != 1) {
        error_on_line(cur, "pushd: need directory argument");
        return FALSE;
    }
    
    if (!parse_argmt(STRING, cur->argmts, &cur->info)) {
        return FALSE;
    }
    
    if (pushd(cur->argmts->data.string)) {
        error_on_line(cur, "pushd: cannot cd to `%s'", cur->argmts->data.string);
        return FALSE;
    }
    
    return TRUE;
}

int dir_popd(struct asmstate *state) {
    struct line *cur = state->cur_line;
    no_asm_output(cur);
    
    // need no arguments
    if (cur->n_argmts != 0) {
        error_on_line(cur, "popd: invalid arguments");
        return FALSE;
    }
    
    if (popd()) {
        error_on_line(cur, "popd: failed");
        return FALSE;
    }
    
    return TRUE;
}


// Assemble a file
struct line *assemble(struct asmstate *state, const char *filename) {
    intptr_t foo;
    
    /* Read and parse the file */
    struct line *lines = read_file(filename);
    state->cur_line = lines;
    if (lines == NULL) return NULL; // error
    
    /* Do sanity checks */
    if (sanity_checks(lines)) {
        error_in_file(lines, "assembly aborted.");
        return NULL;
    }
    
    while (state->cur_line != NULL) {
        
        if (state->prev_line == NULL) {
            // initialize values
            state->cur_line->location = 0;
        } else {
            // use values from previous line
            state->cur_line->location = state->prev_line->location + state->prev_line->n_bytes;
        }

        set_base(state->knowns, state->cur_line->info.lastlabel);
        set_base(state->unknowns, state->cur_line->info.lastlabel);
        
        // If the current line has a label, then it is defined as the current location
        if (state->cur_line->label != NULL && !(
            state->cur_line->instr.type == MACRO   // Macros are named by their labels and don't count
            || (state->cur_line->instr.type == DIRECTIVE 
                && state->cur_line->instr.instr == DIR_equ)  // Definitions don't count either
            )) {
            // See if it is already known. This would be an error.
            if (get_var(state->knowns, state->cur_line->label, &foo)
            ||  get_var(state->unknowns, state->cur_line->label, &foo)) {
                error_on_line(state->cur_line, "label is already defined elsewhere: %s", state->cur_line->label);
                return NULL;
            }
            
            // If not, we know what the value is.
            set_var(state->knowns, state->cur_line->label, state->cur_line->location);
            
            
        }
        
        // Handle the current line 
        switch(state->cur_line->instr.type) {
            case NONE: // No instruction on the line.
                no_asm_output(state->cur_line);
                break;
                
            case DIRECTIVE: // Assembler directive
                resolve_all(state); // make sure all variables are defined as far as possible
                switch(state->cur_line->instr.instr) {

                    #define _DIR(dir)\
                    case DIR_##dir:\
                        if (!dir_##dir(state)) return NULL;\
                        break;
                        
                    #include "instructions.h"
                    
                    default:
                        FATAL_ERROR("invalid assembler directive");
                }
                break;
            
            // TODO: Implement the rest
            
            default:
                FATAL_ERROR("invalid instruction type");
        }
        
        // Next line
        state->prev_line = state->cur_line;
        state->cur_line = state->prev_line->next_line;
        
    }
    
    return lines;
}