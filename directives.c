#include "directives.h"

// Set the given line to have no length and no further processing needed
// (Most assembler directives)
void no_asm_output(struct line *line) {
    line->n_bytes = 0;
    line->needs_process = FALSE;
}

// evaluate an expression, give error if it cannot be evaluated
int eval_on_line(struct asmstate *state, const struct parsed_expr *expr, intptr_t *result, const char *errmsg) {
    if (contains_undefined_names(expr, state->knowns)) {
        error_on_line(state->cur_line, "%s", errmsg);
        return FALSE;
    } else {
        *result = eval_expr(expr, state->knowns, &state->cur_line->info, state->cur_line->location);
        return TRUE;
    }
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
    if (cur_line->n_argmts != 1 || !parse_argmt(STRING, cur_line->argmts, &cur_line->info)) {
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
        
    char *copy_fname = copy_string(fname);
    pre_len = snprintf(pre_include, MAX_PATHLINE_SIZE, INCLUDE_PRE, dirname(copy_fname), fname);
    free(copy_fname);
    post_len = snprintf(post_include, MAX_PATHLINE_SIZE, INCLUDE_POST, fname);
    
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

// include a binary file
int dir_incbin(struct asmstate *state) {
    struct line *cur_line = state->cur_line;
    
    // See if we have the right arguments
    if (cur_line->n_argmts != 1 || !parse_argmt(STRING, cur_line->argmts, &cur_line->info)) {
        error_on_line(cur_line, "'incbin' needs one string argument.");
        return FALSE;
    }
    
    // Open the file
    char *fname = cur_line->argmts->data.string;
    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        error_on_line(cur_line, "cannot open file: %s", fname);
        return FALSE;
    }
    
    // Read the file
    unsigned char *mem = malloc(65536);
    if (mem == NULL) FATAL_ERROR("could not allocate memory for binary file");
    size_t size = fread(mem, 1, 65536, file);
    fclose(file);
    
    // Check if there is enough room (whole file has been read and fits below current line
    // in memory)
    if (cur_line->location + size >= 65536) {
        error_on_line(cur_line, "file is too big to include at %huX: %s",
            cur_line->location, fname);
        free(mem);
        return FALSE;
    }
    
    // Free extra memory and set up line
    mem = realloc(mem, size);
    if (mem == NULL) FATAL_ERROR("realloc() failed");
    cur_line->n_bytes = size;
    cur_line->bytes = mem;
    cur_line->needs_process = FALSE;
    
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
    intptr_t newloc = 0;
    //int newloc = eval_expr(expr, state->knowns, &cur_line->info, cur_line->location);
    
    
    if (!eval_on_line(state, expr, &newloc, 
            "org: all labels in expression must be fully defined previously")) {
        return FALSE;
    } else {
        // This is the new origin for this line
        cur_line->location = (int)newloc;
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
    intptr_t result = 0;
    if (!eval_on_line(state, arg->data.expr, &result,
            "ds: all labels in size expression need to be fully defined previously")) {
        return FALSE;
    }
    cur_line->n_bytes = (int)result;
        
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
    
    intptr_t cond = 0;
    if (eval_on_line(state, cur->argmts->data.expr, &cond, 
            "if: all labels in condition expression need to be fully defined previously"))
        return handle_if(state, cond);
    else
        return FALSE;
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

// align
int dir_align(struct asmstate *state) {
    struct line *cur = state->cur_line;
    intptr_t alignment = 0, fill = 0;
    unsigned char fill_u = 0;
    
    // we need 1 or 2 arguments
    if (cur->n_argmts != 1 && cur->n_argmts != 2) {
        error_on_line(cur, "align: invalid arguments; need alignment and optional fill byte");
        return FALSE;
    }
    
    // the first argument is the alignment
    if (!parse_argmt(EXPRESSION, cur->argmts, &cur->info)) return FALSE;
    if (!eval_on_line(state, cur->argmts->data.expr, &alignment,
            "align: alignment expression must be fully defined")) return FALSE;
    
    // if there is a second argument, it is the fill byte (default 0)
    if (cur->n_argmts == 2) {
        struct argmt *fill_arg = cur->argmts->next_argmt;
        if (!parse_argmt(EXPRESSION, fill_arg, &cur->info)) return FALSE;
        if (!eval_on_line(state, fill_arg->data.expr, &fill,
                "align: fill byte expression must be fully defined")) return FALSE;
        
        fill_u = (unsigned char) fill;
        
        // check if it is within byte value
        if (fill < -128 || fill > 255)
            error_on_line(cur, "align: warning: fill byte value (%X) truncated to 8 bits (%02X)",
                    (int)fill, (unsigned char)fill_u);
    }
    
    // If this line is already aligned, do nothing
    if (cur->location % alignment == 0) {
        no_asm_output(cur);
        return TRUE;
    } else {
        // Otherwise, pad it until it is aligned
        cur->n_bytes = alignment - (cur->location % alignment);
        cur->bytes = malloc(cur->n_bytes);
        if (cur->bytes == NULL) FATAL_ERROR("malloc() failed");
        memset(cur->bytes, fill_u, cur->n_bytes);
        cur->needs_process = FALSE;
        return TRUE;
    }
}
        
                
// assertion
int dir_assert(struct asmstate *state) {
    struct line *cur = state->cur_line;
    no_asm_output(cur); // assert doesn't output any bytes
    cur->needs_process = TRUE; // but it should be processed at the end
    
    // we need 1 or 2 arguments
    if (cur->n_argmts != 1 && cur->n_argmts != 2) {
        error_on_line(cur, "assert: invalid arguments; need expression and optional message");
        return FALSE;
    }
    
    // the first argument is the expression that needs to evaluate to nonzero
    if (!parse_argmt(EXPRESSION, cur->argmts, &cur->info)) return FALSE;
    // if there is a second argument, it must be a string that gives a message
    if (cur->n_argmts == 2) {
        if (!parse_argmt(STRING, cur->argmts->next_argmt, &cur->info)) return FALSE;
    }
    
    return TRUE;
}

// pushorg and poporg
int dir_pushorg(struct asmstate *state) {
    intptr_t neworg = 0;
    struct line *cur = state->cur_line;
    no_asm_output(cur);
    
    
    // we need 1 expression argument which must be fully defined
    if (cur->n_argmts != 1) {
        error_on_line(cur, "pushorg: no new origin given");
        return FALSE;
    }
    
    if (!parse_argmt(EXPRESSION, cur->argmts, &cur->info)) return FALSE;
    if (!eval_on_line(state, cur->argmts->data.expr, &neworg,
            "pushorg: origin expression must be fully defined")) return FALSE;
    
    if (neworg < 0 || neworg > 65535) {
        error_on_line(cur, "pushorg: invalid memory location: %X", (int) neworg);
        return FALSE;
    }
    
    push_org(state, cur, (int) neworg);
    return TRUE;
}

int dir_poporg(struct asmstate *state) {
    struct line *cur = state->cur_line;
    no_asm_output(cur);
    
    // we need no arguments
    if (cur->n_argmts != 0) {
        error_on_line(cur, "poporg: takes no arguments");
        return FALSE;
    }
    
    if (!pop_org(state, cur)) {
        error_on_line(cur, "poporg: no corresponding pushorg");
        return FALSE;
    } else {
        return TRUE;
    }
}


    
    
    