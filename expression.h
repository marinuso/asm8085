/* asm8085 (C) 2019 Marinus Oosters */

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "util.h"
#include "parser_types.h"
#include "expr_fns.h"
#include "varspace.h"
#include "assembler.h"

#define MAX_NUM_LEN 10  // maximum length of number token
#define EVAL_STACK_SIZE 1024

// operators 
enum operator {
    #define _OPR(op, name, prcd, argsn) OPR_##name, 
    #include "operators.h"
};

// keywords 
enum keyword {
    #define _KWD(kwd) KWD_##kwd,
    #include "operators.h" 
};

// token stack

struct token_stack_node {
    const struct token *token;
    struct token_stack_node *prev, *next;
}; 

// token 
struct token {
    struct token *next_token;
    char *text;
    enum { NUMBER, NAME, KEYWORD, LBRACE, RBRACE, OPERATOR } type;
    int value; // set if NUMBER, OPERATOR or KEYWORD.
};

// parsed expression 
struct parsed_expr {
    struct token_stack_node *start;
    struct token *token_list; // so we can free the tokens afterwards (not guaranteed to be there)
    char *basename; // basename to use for dot names in expression 
};

// deep copy a token. next_token is set to NULL.
struct token *copy_token(const struct token *);

// deep copy a parsed expression
struct parsed_expr *copy_parsed_expr(const struct parsed_expr *);

// free parsed expression
void free_parsed_expr(struct parsed_expr *);

// parse an expression
struct parsed_expr *parse_expr(const char *text, const struct lineinfo *info); 

// evaluate parsed expression
int eval_expr(const struct parsed_expr *expr, const struct varspace *vs, const struct lineinfo *info, int location);

// free tokens
void free_tokens(struct token *);

// tokenize a string (filename and lineno are to generate an error)
struct token *tokenize(const char *text, const struct lineinfo *info, char *error);

// get the next valid token in the string, NULL if none, error message generated if not empty.
// out_ptr points at location after token
struct token *get_token(const char *ptr, const char **out_ptr, const struct lineinfo *info, char accept_token, char *error);

// evaluate a string
int evaluate(const char *text, const struct varspace *vs, const struct lineinfo *info, int location);

// See if a parsed expression contains names not defined in vs.
char contains_undefined_names(const struct parsed_expr *, const struct varspace *) ;


#endif
