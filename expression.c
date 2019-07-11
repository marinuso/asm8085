/* asm8085 (C) 2019 Marinus Oosters */

#include "expression.h"

#define STRING_DEFAULT_SIZE 512

static const char *operators[] = {
    #define _OPR(opr, name, pred, val) #opr ,
    #include "operators.h"
    NULL
};

struct operator_info_s = {
    char *opr,
    int precedence,
    int valence
};

static const struct operator_info_s operator_info [] = {
    #define _OPR(opr, name, pred, val) { opr, pred, val },
    #include "operators.h"
    { NULL, 0, 0 }
};

static const char *keywords[] = {
    #define _KWD(kwd) #kwd ,
    #include "operators.h"
    NULL
};

struct base_fx_s {
    char fx[2];
    int base;
};

static const struct base_fx_s base_pfx[] = {
    {"0b", 2},
    {"0o", 8},
    {"0x", 16},
    {"$", 16},
    {"", 0}
};

static const struct base_fx_s base_sfx[] = {
    {"b", 2},
    {"o", 8},
    {"h", 16},
    {"", 0}
};

// free tokens
void free_tokens(struct token *tok) {
    struct token *next = tok;
    while (next != NULL) {
        tok = next;
        next = tok->next_token;
        free(tok->text);
        free(tok);
    }
}

// allocate space for token and fail if NULL
struct token *alloc_token() {
    struct token *t = malloc(sizeof(struct token));
    if (t == NULL) FATAL_ERROR("failed to allocate space for token");
    return t;
}



int is_name_character (int c) {
    return c=='_' || c=='.' || isalnum(c);
}

int is_op_character(int c) {
    return c!='_' && c!=' ' && c!='.' && ispunct(c);
}


// get the next "word" (string of alphanumeric and/or symbolic characters)
char *get_word(char *c) {
    int (*pred)(int) = is_name_character(*c) ? is_name_character : is_op_character;
    return copy_string_pred(c, pred, TRUE);
}

// if the token at *ptr is a literal from the given list, return a token, otherwise NULL
struct token *try_from_list(const char *const *list, const char *begin, char **out_ptr, char allow_partial_match) {
    int i;
    
    // check for empty
    if (!*ptr) return NULL;
    
    // get next word
    char *word = get_word(begin);
    
    for (i=0; list[i]!=NULL; i++) {
        char found = allow_partial_match ? has_case_insensitive_prefix(word, list[i]) : !strncasecmp(list[i], word);
        if (found) {
            // gotcha
            
            if (allow_partial_match) {
                // trim the word so that it is just the matched prefix
                int new_length = strlen(list[i])+1;
                word[new_length-1] = '\0';
                word = realloc(word, sizeof(char) * new_length);
                if (word == NULL) FATAL_ERROR("realloc failed");
            }
            
            struct token *t = alloc_token();
            t->text = word;
            // the value is set to the list index
            t->value = i;
            *out_ptr = ptr + strlen(word);
            return t;
        }
    }
    
    // we failed, free the word copy we made.
    free(word);
    return NULL;
}
        

// try to parse an operator
struct token *try_operator(const char *begin, char **out_ptr) {
    struct token *t = try_from_list(operators, begin, out_ptr, TRUE);
    if (t != NULL) t->type = OPERATOR;
    return t;
}

// try to parse a keyword
struct token *try_keyword(const char *begin, char **out_ptr) {
    struct token *t = try_from_list(keywords, begin, out_ptr, FALSE);
    if (t != NULL) t->type = KEYWORD;
    return t;
}

// try to parse a name
struct token *try_name(const char *begin, char **out_ptr) {
    // empty line
    if (!*ptr) return NULL;
    struct token *t;
    char *name;
       
    // a name must start with an alphabetic character, '.' or '_'.
    if (!is_name_character(*ptr) || isdigit(*ptr)) {
        // special case: '$' by itself is a metavariable referring to the current location
        if (ptr[0] == '$' && !is_name_character(ptr[1])) {
            name = copy_string("$");
            *out_ptr = begin + 1;
        } else {
            // it's not valid after all
            return NULL;
        }
    } else {
        // it is a normal name 
        name = get_word(*begin);
        *out_ptr = begin + strlen(name);
    }
    
    struct token *t = alloc_token();
    if (t == NULL) FATAL_ERROR("failed to allocate space for token");
    t->text = name;
    t->type = NAME;
    return t;
}

// bracket
struct token *try_bracket(const char *begin, char **out_ptr) {
    struct token *t = NULL;
    if (*ptr == '(' || *ptr == ')') {
        char *s = calloc(1, sizeof(char));
        if (s == NULL) FATAL_ERROR("failed to allocate space for token text");
        t = alloc_token();
        if (t == NULL) FATAL_ERROR("failed to allocate space for token");
        *s = *begin;
        t->text = s;
        t->type = (*s == '(') ? LBRACE : RBRACE;
        *out_ptr = begin+1;
    }
    return t;
}
    

// if the token at *ptr is a valid number, return a token, otherwise NULL
struct token *try_number(const char *begin, char **out_ptr) {
    int sign = 1, base = 10, i, out_num;
    char base_given = FALSE;
    char digits[] = "0123456789ABCDEF";
    char number_text[MAX_NUM_LEN+1] = {'\0'};
    char *ptr = begin;
    struct base_fx_s *b;
    
    // check for empty
    if (!*ptr) return NULL;
    
    // check for negative number
    if (*ptr == '-') { sign = -1; ptr++; }
    
    // check if a base is given (start with 0x, 0o, 0b or $)
    b = base_pfx;
    while (b->base != 0) {
        size_t pfxlen = strlen(b->fx);
        if (! strncasecmp(b->fx, ptr, pfxlen) {
            base = b->base;
            ptr += pfxlen;
            base_given = TRUE;
            break;
        } else {
            b++;
        }
    }
    
    if (base_given) {
        // Truncate the list of valid digits if a base was given
        digits[base] = '\0';
    } else {
        // If there was no prefix, the number itself must start with a (base-10) digit
        // to prevent clashes with names (e.g. 'ABCDH' could be the label 'abcdh' or the number '0xABCD' otherwise)
        if (! (*ptr >= '0' && *ptr <= '9')) return NULL;
    }
    
    // Collect all the necessary digits
    for (i=0; i<MAX_NUM_LEN && ptr[i] && strchr(digits, ptr[i]); i++) {
        number_text[i] = ptr[i];
    }
    
    // If the length was exceeded, this was not a valid token.
    if (i == MAX_NUM_LEN) return NULL;
    
    ptr += i;
    
    // If we didn't have a base yet, check for a base suffix
    if (! base_given) {
        b = base_sfx;
        while (b->base != 0) {
            if (toupper(*b->fx) == toupper(*ptr)) {
                base = b->base;
                ptr ++;
                break;
            }
        }
        
        // Since the base may now have changed, check if the digits are all still valid
        digits[base] = '\0';
        for (i=0; number_text[i]; i++) {
            if (! strchr(digits, number_text[i])) return NULL;
        }
    }
    
    // If we are here, we now have a valid number in base "base"
    out_num = 0;
    for (i=0; number_text[i]; i++) {
        out_num *= base;
        out_num += strchr(digits, number_text[i]) - digits;
    }
    
    // Allocate space for the token
    struct token *token = alloc_token();
    
    // Set token text, type and number value
    token->text = copy_string_part(begin, ptr);
    token->value = out_num * sign;
    token->type = NUMBER;
    
    *out_ptr = ptr;
    return token;
    
}
    
            

// get the next valid token in the string, NULL if none, error message generated if not empty.
// out_ptr points at location after token
struct token *get_token(const char *ptr, char **out_ptr, const struct lineinfo *info, char accept_operator, char *error) {
    // Scan ahead to next nonwhitespace character
    ptr = scan_ahead(ptr, isspace, TRUE);
    
    if (!*ptr) {
        // Line is empty.
        *out_ptr = ptr;
        return NULL;
    }
    
    // Test each type in order. Operator is tried first, if operators are allowed, this is to distinguish
    // negative numbers from minus followed by a positive number.
    
    struct token *t = NULL;
    
    if (accept_operator) { t = try_operator(ptr, out_ptr); if (t != NULL) return t; }
    t = try_number(ptr, out_ptr); if (t != NULL) return t; 
    t = try_keyword(ptr, out_ptr); if (t != NULL) return t; 
    t = try_name(ptr, out_ptr); if (t != NULL) return t; 
    t = try_bracket(ptr, out_ptr); if (t != NULL) return t;
    
    
    // Invalid token
    char *inv = get_word(ptr);
    fprintf(stderr, "%s: line %d: invalid token: %s\n", info->filename ,info->lineno, inv);
    free(inv);
    *error = TRUE;
    return NULL;
}

// Tokenize a string
struct token *tokenize(const char *text, const struct lineinfo *info, char *error) {
    
    struct token *t = NULL, *start = NULL, *prev = NULL;
    char accept_operator = FALSE, *out_ptr;
    
    while (*text) {
        text = scan_ahead(text, isspace, TRUE);
        if (!*text) break; // end of line
        
        t = get_token(text, &out_ptr, info, accept_operator, error);
        if (t == NULL) goto err_cleanup; // error
        
        // t now holds a token
        if (start == NULL) start = t; // first token
        else prev->next_token = t; // chain to previous token
        prev = t;
        
        // an operator may follow a value (name, number or closing brace), nothing else
        accept_operator = t->type == NAME || t->type == NUMBER || t->type == RBRACE;
    
        // continue tokenizing after the previous token
        text = out_ptr;
    }
    
    return t;
err_cleanup:
    free_tokens(start);
    return NULL;
}
            
           
// Push/pop
struct token_stack_node *pop(struct token_stack_node *n) {
    struct token_stack_node *r = n->prev;
    if (r != NULL) r->next = NULL;
    free(n);
    return r;
}

struct token_stack_node *push(struct token_stack_node *p, const struct token *tok) {
    struct token_stack_node *n = malloc(sizeof(struct token_stack_node));
    if (n == NULL) FATAL_ERROR("failed to allocate space for stack");
    
    n->prev = p;
    if (p!=NULL) p->next = n;
    n->token = tok;
    return n;
}

void free_stack(struct token_stack_node *entry) {
    struct token_stack_node *next = entry;
    while (next != NULL) {
        entry = next;
        next = entry->prev;
        free(entry);
    }
}

// find RPN execution order for tokens (using shunting yard algorithm)
struct token_stack_node *execution_order(const struct token *tok, const struct lineinfo *info) {
    struct token_stack_node *out_stack = NULL, *op_stack = NULL;
    
    // Process all the tokens
    for (; tok != NULL; tok = tok->next_token) {
        switch(tok->type) {
            /* values (names, numbers) go onto the output stack immediately */
            case NAME:
            case NUMBER:
                out_stack = push(out_stack, tok);
                break;
            
            /* keywords and left braces go onto the operator stack */
            case LBRACE:
            case KEYWORD:
                op_stack = push(op_stack, tok);
                break;
                
            /* operators */
            case OPERATOR:
                if (operator_info[tok->value].valence == 1) {
                    /* unary operators bind as strongly as keywords do */
                    op_stack = push(op_stack, tok);
                } else {
                    while ( op_stack != NULL &&           
                            op_stack->token->type != LBRACE &&                                
                            ( op_stack->token->type == KEYWORD ||
                              operator_info[op_stack->token->value].precedence >= operator_info[tok->value].precedence
                            )) {
                        out_stack = push(out_stack, op_stack->token);
                        op_stack = pop(op_stack);
                    }
                }
                op_stack = push(op_stack, tok);
                break;
                
           /* right parenthesis: pop all operators until left parenthesis */
           case RBRACE:
                while (op_stack != NULL && op_stack->token->type != LBRACE) {
                    out_stack = push(out_stack, op_stack->token);
                    op_stack = pop(op_stack);
                }
                
                // check for missing brace
                if (op_stack == NULL) {
                    fprintf(stderr, "%s: line %d: missing '('.\n", info->filename, info->lineno);
                    goto err_cleanup;
                }
                
                // remove the brace that's still on the stack
                op_stack = pop(op_stack);
                break;
                
           default:
                // you never know
                FATAL_ERROR("unrecognized token type %d", tok->type);
        }
    }
    
    // Find the beginning of the operator stack
    struct token_stack_node *ptr;
    for (ptr = op_stack; ptr != NULL && ptr->prev != NULL; ptr = ptr->prev) {
        // Check for stray left braces while we're at it
        if (ptr->token->type == LBRACE) {
            fprintf(stderr, "%s: line %d: missing ')'.\n", info->filename, info->lineno);
            goto err_cleanup;
        }
    }
    
    if (ptr != NULL && out_stack != NULL) {
        // push the remainder of the operator stack onto the output stack (i.e. just join them together)
        // ptr is now the first item of the operator stack and out_stack is the last item in the output stack
        out_stack->next = ptr;
        ptr->prev = out_stack;
    }
    
    // Find the beginning of the output stack
    for (ptr = out_stack; ptr != NULL && ptr->prev != NULL; ptr = ptr->prev);
    
    // If ptr is now 0, that means there was no expression at all.
    if (ptr == NULL) {
        fprintf(stderr, "%s: line %d: no expression found where one was expected.\n", info->filename, info->lineno);
    }
    
    return ptr;
    
err_cleanup:
    // free both stacks before returning 0.
    free_stack(out_stack);
    free_stack(op_stack);
    return NULL;
}
        
       



int eval_rpn_queue(const struct token_stack_node *node, const struct varspace *vs, const struct lineinfo *info, int location) {
    int stack[EVAL_STACK_SIZE], stackptr=0, val;
    
    for (; node != NULL; node = node->next) {
        struct token *t = node->token;
        switch (t->type) {
            case NUMBER:
                stack[stackptr++] = t->value;
                break;
                
            case NAME:
                if (!strcmp(t->text, "$")) {
                    // $ = current location
                    val = location; 
                } else if (!get_var(vs, t->text, &val)) {
                    fprintf(stderr, "%s: line %d: undefined name: %s\n", info->filename, info->lineno, t->text);
                    return 0;
                }
                stack[stackptr++] = val;
                break;
                
            case KEYWORD:
                if (stackptr < 1) {
                    fprintf(stderr, "%s: line %d: missing argument for: %s\n", info->filename, info->lineno, t->text);
                    return 0;
                }
                stack[stackptr-1] = eval_keyword(t->value, stack[stackptr-1]);
                break;
            
            case OPERATOR:
                val = operator_info[t->value].valence;
                if (stackptr < val) {
                    fprintf(stderr, "%s: line %d: missing argument for: %s\n", info->filename, info->lineno, t->text);
                    return 0;
                }
                stackptr -= val - 1;
                stack[stackptr-1] = eval_operator(t->value, &stack[stackptr-1]);
                break;
        
            default:
                fprintf(stderr, "%s: line %d: internal error: invalid token type %d (%s). (this is a bug)\n",
                                    info->filename, info->lineno, t->type, t->text);
                return 0;
        }
        
        if (stackptr >= EVAL_STACK_SIZE) {
            fprintf(stderr, "%s: line %d: evaluation stack size exceeded.\n", info->filename, info->lineno);
            return 0;
        }
        
    }
    
    // there should be exactly one value left on the stack, which is the result of the evaluation
    // if there's more than one, there are unused values
    if (stackptr > 1) {
        fprintf(stderr, "%s: line %d: invalid expression\n",  info->filename, info->lineno);
        return 0;
    } else {
        return stack[0];
    }
}


int evaluate(const char *text, const struct varspace *vs, const struct lineinfo *info, int location) {
    char error = 0;
    struct token *t = tokenize(text, info, &error);
    if (t==NULL || *error) return 0; // an error message will have already been printed
    struct token_stack_node *n = execution_order(t, info);
    if (n==NULL) return 0;
    return eval_rpn_queue(n, vs, info, location);
}

