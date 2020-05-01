/* asm8085 (C) 2019-20 Marinus Oosters */

#include "assembler.h"

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
    state->orgstack = NULL;
    
    state->n_includes = 0;
    state->n_macro_exp = 0;

    return state;
}

// Free orgstack
void free_orgstack(struct orgstack_item *top) {
    struct orgstack_item *s;
    while (top != NULL) {
        s = top;
        top = top->prev;
        free(s);
    }
}

// Free assembler state
void free_asmstate(struct asmstate *state) {
    if (state != NULL) {   
        free_maclist(state->macros);
        free_varspace(state->knowns);
        free_varspace(state->unknowns);
        free_orgstack(state->orgstack);
        free(state);
    }
}

// Push to org stack
void push_org(struct asmstate *state, struct line *start, int newloc) {
    struct orgstack_item *s;
    if ((s = malloc(sizeof(struct orgstack_item))) == NULL)
        FATAL_ERROR("failed to allocate memory for orgstack");
    
    s->loc = start->location;
    s->start = start;
    s->start->location = newloc;
    s->prev = state->orgstack;
    state->orgstack = s;
}
    
// Pop from org stack 
int pop_org(struct asmstate *state, struct line *end) {
    if (state->orgstack == NULL) return FALSE;  // empty org stack
    
    // Pop it
    struct orgstack_item *s;
    s = state->orgstack;
    state->orgstack = s->prev;
    
    // Restore original position plus the size of the relocated lines
    int n_bytes = end->location - s->start->location;
    end->location = s->loc + n_bytes;
    
    return TRUE;
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



// Assemble lines
struct line *asm_lines(struct asmstate *state, struct line *lines) {
    intptr_t foo;
    struct line *macro, *macro_end;
    
    state->cur_line = lines;
    if (lines == NULL) goto error;
    
    /* Do sanity checks */
    if (sanity_checks(lines)) {
        error_in_file(lines, "assembly aborted.");
        goto error;
    }
    
    while (state->cur_line != NULL) {
        
        if (state->prev_line == NULL) {
            // initialize values
            state->cur_line->location = 0;
        } else {
            // use values from previous line
            state->cur_line->location = state->prev_line->location + state->prev_line->n_bytes;
            
            if (state->cur_line->location > 0xFFFF) {
                error_on_line(state->cur_line, "line would be assembled beyond memory (location = %d)",
                        state->cur_line->location);
                return NULL;
            }
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
                goto error;
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
                        if (!dir_##dir(state)) goto error;\
                        break;
                        
                    #include "instructions.h"
                    
                    default:
                        FATAL_ERROR("invalid assembler directive");
                }
                break;
                
            case MACRO: // Macro expansion
                if (state->n_macro_exp++ > MAX_MACRO_EXP) {
                    // put a limit on the maximum amount of macro expansions
                    // this way, the user is given an error, rather than a segfault
                    // if a macro expansion is infinite
                    error_on_line(state->cur_line, "too many macro expansions"); 
                    goto error;
                }
                
                macro = expand_macro(state->cur_line, state->macros, &macro_end);
                if (macro == NULL) goto error;
                
                macro_end->next_line = state->cur_line->next_line;
                state->prev_line->next_line = macro;
                state->cur_line = state->prev_line;
                break;
            
            case OPCODE: // Opcode
                switch(state->cur_line->instr.instr) {
                    
                    #define _OP(op, _)\
                    case OP_##op:\
                        if (!op_##op(state)) goto error;\
                        break;
                    
                    #include "instructions.h"
                    
                    default:
                        FATAL_ERROR("invalid opcode");
                }
                break; 
                
            default:
                FATAL_ERROR("invalid instruction type: %d\ninstr: %s\nline: %s", 
                    state->cur_line->instr.type, 
                    state->cur_line->instr.text,
                    state->cur_line->raw_text);
        }
        
        // Next line
        state->prev_line = state->cur_line;
        state->cur_line = state->prev_line->next_line;
        
    }    
    
    return lines;
error:
    return NULL;
    
}

// Assemble a file
struct line *assemble(struct asmstate *state, const char *filename) {
    
    /* Switch the working directory for includes */
    char *fcopy;
    fcopy = copy_string(filename);
    if (pushd(dirname(fcopy)) == -1) {
        fprintf(stderr, "%s: cannot open directory: %s\n", filename, fcopy); 
        free(fcopy);
        goto error;
    }
    free(fcopy);
    
    /* Read and parse the file */
    fcopy = copy_string(filename);
    struct line *lines = read_file(basename(fcopy));
    if (lines == NULL) fprintf(stderr, "%s: failed to read file\n", filename);
    
    free(fcopy);
    
    lines = asm_lines(state, lines); 
    if (lines == NULL) goto error; 
    
    popd();
    resolve_all(state);
    return lines;
    
error:
    popd();
    return NULL;
}

// Evaluate an expression given a state. Return the result, or give errors.
int eval_state(struct argmt *argmt, struct asmstate *state, const struct line *line, intptr_t *result) {
    intptr_t temp = 0;
    *result = eval_expr(argmt->data.expr, state->knowns, &line->info, line->location);
    if (!contains_undefined_names(argmt->data.expr, state->knowns)) {
        return TRUE;
    }
    
    /* Figure out which labels are not defined, and output them.*/
    struct token_stack_node *ts = argmt->data.expr->start;
    struct varspace vs = temp_rename(state->knowns, argmt->data.expr->basename);
    for(; ts != NULL; ts=ts->next) {
        if (ts->token->type != NAME) continue;
        if (!get_var(&vs, ts->token->text, &temp)) {
            // This label is not defined. 
            //error_on_line(line, "undefined label: %s", ts->token->text);
            
            // Is it an unknown label (i.e. `equ' with underspecified expression?)
            struct varspace uvs = temp_rename(state->unknowns, argmt->data.expr->basename);
            if (get_var(&uvs, ts->token->text, &temp)) {
                // It is.
                struct line *unk = (struct line *)temp;
                if (unk->instr.type != DIRECTIVE || unk->instr.instr != DIR_equ) {
                    FATAL_ERROR("unknown value pointer not pointing to equ");
                }
                
                // Tell the user what the problem is
                error_on_line(unk, "underspecified expression: %s", unk->argmts->raw_text);
            }
        }
    }
    
    return FALSE;
}

// Evaluate all remaining expressions, and fill in the results
int complete(struct asmstate *state, struct line *lines) {
    struct line *line;
    struct argmt *argmt;
    unsigned char *pos;
    int assertok = TRUE; // set to FALSE if assertion fails, this way all assertions are tried
    intptr_t result = 0;
    size_t size;

    resolve_all(state);

    // process each line in turn 
    for (line = lines; line != NULL; line=line->next_line) {
        // Skip lines that don't need processing
        if (! line->needs_process) continue;
        line->needs_process = FALSE;
        
        switch(line->instr.type) {
            case NONE:
            case MACRO:
                FATAL_ERROR("needs_process is set on empty or macro line");
                break;
            
            case DIRECTIVE:
                argmt = line->argmts;
                pos = line->bytes;
                switch(line->instr.instr) {
                    case DIR_db:
                        // Define bytes
                        for (; argmt != NULL; argmt = argmt->next_argmt) {
                            switch(argmt->type) {
                                case EXPRESSION:
                                    // Evaluate the expressoin
                                    if (!eval_state(argmt, state, line, &result)) {
                                        return FALSE; // could not evaluate
                                    }
                                    // Give a warning (but don't fail) if the argument doesn't fit
                                    if (result < -128 || result > 255) {
                                        error_on_line(line, "warning: result does not fit in byte, will be truncated");
                                        error_on_line(line, "  %s == %02x", argmt->raw_text, result);
                                    }
                                    // Truncate to byte and store
                                    *pos++ = (unsigned char) result;
                                    break;
                                    
                                case STRING:
                                    // A string is just copied.
                                    size = strlen(argmt->data.string);
                                    memcpy(pos, argmt->data.string, size);
                                    pos += size;
                                    break;
                                    
                                default:
                                    FATAL_ERROR("db: wrong argument type %d (%s)",argmt->type,argmt->raw_text);
                            }
                        }
                        break;
                   case DIR_dw:
                        // Define words
                        for (; argmt != NULL; argmt = argmt->next_argmt) {
                            if (argmt->type != EXPRESSION) {
                                FATAL_ERROR("dw: wrong argument type %d",argmt->type);
                            }
                            
                            // Evaluate the argument
                            if (!eval_state(argmt, state, line, &result)) {
                                return FALSE;
                            }
                            
                            // Warn if value doesn't fit in 2 bytes
                            if (result < -32768 || result > 65535) {
                                error_on_line(line, "warning: result does not fit in word, will be truncated");
                                error_on_line(line, "  %s == %04x", argmt->raw_text, result);
                            }
                            
                            *pos++ = (unsigned char) (result & 0xFF); // Low byte first
                            *pos++ = (unsigned char) (result >> 8); // High byte second 
                        }
                        break;
                    
                    case DIR_assert:
                        // Assertion
                        if (!eval_state(argmt, state, line, &result)) return FALSE;
                        
                        if (!result) {
                            // Assertion failed
                            char *msg;
                            // Is there a message?
                            if (argmt->next_argmt) msg = argmt->next_argmt->data.string; // yes
                            else msg = argmt->raw_text; // no, use the expression
                            error_on_line(line, "assertion failed: %s", msg);
                            assertok = FALSE;
                        }
                        break;
                    
                    default:
                        FATAL_ERROR("invalid directive %d", line->instr.instr);
                }
                break;
            
            case OPCODE:
                // Make sure this opcode actually does take an operand
                if (line->n_bytes != 2 && line->n_bytes != 3) {
                    FATAL_ERROR("n_bytes == %d", line->n_bytes);
                }
                
                // An immediate value is always the last argument
                argmt = line->argmts;
                while (argmt->next_argmt != NULL) argmt=argmt->next_argmt;
                
                // The argument must be an expression
                if (argmt->type != EXPRESSION) FATAL_ERROR("invalid argument type");
                
                // Evaluate the expression 
                if (!eval_state(argmt, state, line, &result)) return FALSE;
                
                pos = line->bytes + 1; // First byte is opcode
                if (line->n_bytes == 2) {
                    // Immediate value is one byte long
                    if (result < -128 || result > 255) {
                        error_on_line(line, "warning: result does not fit in byte, will be truncated");
                        error_on_line(line, "  %s == %02x", argmt->raw_text, result);
                    }
                    *pos = (unsigned char) result;
                } else {
                    // Immediate value is two bytes long
                    if (result < -32768 || result > 65535) {
                        error_on_line(line, "warning: result does not fit in byte, will be truncated");
                        error_on_line(line, "  %s == %04x", argmt->raw_text, result);
                    }
                    *pos++ = (unsigned char) (result & 0xFF);
                    *pos = (unsigned char) (result >> 8);
                }
                
                break;
                
            default:
                FATAL_ERROR("invalid instruction type %d", line->instr.type);
        }
    }
    
    return assertok;
}
