/* asm8085 (C) 2019-20 Marinus Oosters */

// This file contains tests for the functions in expression.c

#define EX_INIT \
    struct varspace *vs = alloc_varspace(); \
    struct lineinfo info; \
    info.filename = "test"; \
    info.lastlabel = NULL; \
    info.lineno = 1; 
    
#define EX_FREE \
    free_varspace(vs);

#define EX_TEST(name, code) TEST(ex_##name, EX_INIT, EX_FREE, code)

/** Test the tokenizer **/

// Macro: try to get a token from something, run some code if it didn't error out, 
// FAIL if that code doesn't set ok to true. 

#define TOKC(what, try_op, code, msg, ...) do { \
    t = get_token(what, &out_ptr, &info, try_op, &error); \
    if (t==NULL) FAIL("get_token returned NULL on %s", #what); \
    if (error) FAILC("get_token returned error on %s", free_tokens(t), #what); \
    { \
        char ok = 0; \
        code; \
        if (!ok) FAILC(msg, free_tokens(t), ##__VA_ARGS__); \
        free_tokens(t); \
    } \
} while(0);

// Macro: see if token matches value
#define TOKTEST(tok, typ, val) ((tok)->type == (typ) && (tok)->value == (val)) 


// Macro: try to tokenize something, make sure that type and value match
#define TOKM(what, try_op, ttype, tvalue) \
    TOKC(what, try_op, ok = TOKTEST(t, ttype, tvalue), \
            "given %s, tokenizer failed to produce %s (%d) of %s (%d); result was: %d / %d.", \
            #what, #ttype, ttype, #tvalue, tvalue, t->type, t->value)


// Macro: test an operator
#define TOKOP(op,name) TOKM(#op, 1, OPERATOR, OPR_##name) 
// Macro: test a number
#define TOKNUM(str,val) TOKM(str, 0, NUMBER, val)
// Macro: test a name
#define TOKNAME(str) \
    TOKC(str, NAME, ok = !strcmp(t->text, str) && t->type == NAME, \
       "tokenizer failed to parse name %s; result was %d, \"%s\"", \
       #str, t->type, t->text)

EX_TEST(single_tokens, {
    struct token *t;
    const char *out_ptr;
    char error = 0;
    
    // Test the braces
    TOKM("(", 0, LBRACE, '(');
    TOKM(")", 0, RBRACE, ')');
    
    // Test the keywords 
    TOKM("high", 0, KEYWORD, KWD_high);
    TOKM("low", 0, KEYWORD, KWD_low);
    // Test the operators
    TOKOP(!=, NE);
    TOKOP(!, BOOL_NOT);
    TOKOP(~, BIT_NOT);
    TOKOP(*, MUL);
    TOKOP(/, DIV);
    TOKOP(%, MOD);
    TOKOP(+, ADD);
    TOKOP(-, SUB);
    TOKOP(<<, SHL);
    TOKOP(>>, SHR);
    TOKOP(<=, LE);
    TOKOP(>=, GE);
    TOKOP(<, LT);
    TOKOP(==, EQ);
    TOKOP(&&, BOOL_AND);
    TOKOP(||, BOOL_OR);
    TOKOP(&, AND);
    TOKOP(^, XOR);
    TOKOP(|, OR);
    // Test numbers
    TOKNUM("42",         42);
    TOKNUM("-42",       -42);
    TOKNUM("$42",      0x42);
    TOKNUM("-$42",    -0x42);
    TOKNUM("42h",      0x42);
    TOKNUM("42H",      0x42);
    TOKNUM("0x42",     0x42);
    TOKNUM("-0x42",   -0x42);
    TOKNUM("0X42",     0x42);
    TOKNUM("42o",       042);
    TOKNUM("0o42",      042);
    TOKNUM("0O42",      042);
    TOKNUM("042",       042);
    TOKNUM("'*'",        42);
//    TOKNUM("0b101010",   42);
    TOKNUM("101010b",    42);
//    TOKNUM("-0b101010", -42);
    TOKNUM("-101010b",  -42);
    TOKNUM("0aah",     0xAA);
    TOKNUM("0bbh",     0xBB);
    // Test backtick 
    TOKNUM("`nop`",       0);
    TOKNUM("`mov a,b`", 0x78);
    TOKNUM("`mvi a,_`", 0x3E);
    // Test names
    TOKNAME("hello");
    TOKNAME("a.bc.def");
    TOKNAME(".boo");
    TOKNAME("$");  
    

    SUCCEED;
})


// Macro: see if token matches value and fail otherwise, freeing 't'; on success advance token pointer
#define TOKCHK(tok, typ, val) do { \
    if ((tok) == NULL) FAILC("got null token instead of %s of %s", free_tokens(t), #typ, #val); \
    if (! TOKTEST(tok,typ,val)) { \
        FAILC("expected %s (%d) of %s, but got %d / %d '%s' ", free_tokens(t), #typ, typ, #val, tok->type, tok->value, tok->text); \
    } \
    tok = tok->next_token; \
} while(0);

// Macro: see if token is name and matches string, and fail otherwise, freeing 't'
#define TOKCHK_NAME(tok, nam) do { \
    if (tok == NULL) FAILC("got null token instead of name of " #nam, free_tokens(t)); \
    if (tok->type != NAME) FAILC("token type is %d, not NAME (%d) ", free_tokens(t), tok->type, NAME); \
    if (strcmp(tok->text, nam)) FAILC("expected name %s, but got \"%s\"", free_tokens(t), #nam, tok->text); \
    tok = tok->next_token; \
} while(0);
 
 
EX_TEST(token_string, {
    struct token *t;
    struct token *r;
    char error = 0;
    
    // Test if a chain of tokens parses correctly.
    t = tokenize("5-6 + -7*high hello >> ($<.world)", &info, &error);
    if (t == NULL) FAIL("tokenize returned NULL");
    if (error) FAIL("tokenize set error");
    r = t; 
      
    // See if each token we expect to be there is actually there
    TOKCHK(r, NUMBER, 5);
    TOKCHK(r, OPERATOR, OPR_SUB);
    TOKCHK(r, NUMBER, 6);
    TOKCHK(r, OPERATOR, OPR_ADD);
    TOKCHK(r, NUMBER, -7);
    TOKCHK(r, OPERATOR, OPR_MUL);
    TOKCHK(r, KEYWORD, KWD_high);
    TOKCHK_NAME(r, "hello"); 
    TOKCHK(r, OPERATOR, OPR_SHR);
    TOKCHK(r, LBRACE, '(');
    TOKCHK_NAME(r, "$"); 
    TOKCHK(r, OPERATOR, OPR_LT);
    TOKCHK_NAME(r, ".world");
    TOKCHK(r, RBRACE, ')');
    
    // Since we're past the last token, r should now be NULL
    if (r != NULL) FAILC("spurious extra token, type is: %d, value is: %d", free_tokens(t), r->type, r->value);
    
    // Free the tokens
    free_tokens(t);
    SUCCEED;
})


/** Test the evaluator **/

// Macro: the location used for $
#define LOC 1234

// Macro: see if a string evaluates to a value
#define EVAL(str, val) { \
    int ret = evaluate(str, vs, &info, LOC); \
    if ((val) != ret) { \
        FAIL("%s should evaluate to %s (%d), but evaluates to %d", #str, #val, val, ret); \
    } \
}

EX_TEST(eval_single, {
    // Number?
    EVAL("42", 42);
    // Variable?
    set_var(vs, "foo", 42);
    EVAL("foo", 42);
    // Location?
    EVAL("$", LOC);
    SUCCEED;
})

// Macro: see if a C expression evaluates to the same thing in the evaluator
#define EVAL_C(expr) EVAL (#expr, expr)
EX_TEST(eval_simple, {
    // Test the C-style operators
    EVAL_C(5 != 6);
    EVAL_C(!0); EVAL_C(!1);
    EVAL_C(~42);
    EVAL_C(5*6);
    EVAL_C(6/5);
    EVAL_C(6%5);
    EVAL_C(6+5);
    EVAL_C(6-5);
    EVAL_C(128>>2);
    EVAL_C(128<<2);
    EVAL_C(5<=6);
    EVAL_C(5>=6);
    EVAL_C(5<6);
    EVAL_C(5>6);
    EVAL_C(5==6);
    EVAL_C(5&&6);
    EVAL_C(5||6);
    EVAL_C(5&6);
    EVAL_C(5^6);
    EVAL_C(5|6);
    
    // Test the high and low keywords
    EVAL("high $FACE", 0xFA);
    EVAL("low $FACE", 0xCE);
    SUCCEED;
})

// Test backtick quoted instructions
EX_TEST(backticks, {
   
    // nop=00h, lxi b=01h, stax b=02h, inx b=03h, inr b=04h, dcr b=05h
    // they should evaluate to normal numbers that can be added up together
    EVAL("`nop` + `lxi b,_` + `stax b` + `inx b` + `inr b` + `dcr b`", 15);
    
})

// Test the evaluation order 
EX_TEST(evaluation_order, {
    EVAL("$AA + high $FACE", 0xFA+0xAA);
    EVAL("high $FACE + $AA", 0xFA+0xAA);
    EVAL("high ($FACE + $AA)", 0xFB);
    
    EVAL("high $ == $ >> 8 && low $ == $ % 256 && low $ == ($&$FF)", 1);
    
    EVAL_C( 1 + 10 / 5 * 2 - 6 );
    EVAL_C( (1 + 10) / 5 * (2 - 6) );
    EVAL_C( ~( 10 + 5 ) + 6 );
    EVAL_C( 6 + ~( 10 + 5 ) );
    
    
    // see if we can distinguish the - operator from negative numbers 
    EVAL("-5--6-7--8", 2);
    
    SUCCEED;
})

// Test making a deep copy of a parsed expression
#define TEST_COPY_EX 1 + (2 * 3) + 4 - 5 * 6 + ~7 * (8 + (9 - 10))
#define VAL(x) (x)
#define STR2(x) #x
#define STR(x) (STR2(x))

EX_TEST(copy_parsed_expr, {
    int rslt;
    
    // Parse the expression
    struct parsed_expr *orig = parse_expr(STR(TEST_COPY_EX), &info);
    if (!orig) FAIL("parse_expr returned NULL");

    rslt = eval_expr(orig, vs, &info, LOC);
    if (rslt != VAL(TEST_COPY_EX)) {
        FAILC("evaluating original returned %d instead of %d",
                free_parsed_expr(orig), rslt, VAL(TEST_COPY_EX));
    }

    // Make a deep copy of the parsed expression
    struct parsed_expr *copy = copy_parsed_expr(orig);
    if (!copy) FAIL("copy_parsed_expr returned NULL");
    
    // Free the original (this should leave the copy intact)
    free_parsed_expr(orig);
    
    // Test that the copy still returns the right value
    rslt = eval_expr(copy, vs, &info, LOC);
    if (rslt != VAL(TEST_COPY_EX)) {
        FAILC("evaluating copy returned %d instead of %d",
              free_parsed_expr(copy), rslt, VAL(TEST_COPY_EX));
    }              
 
    free_parsed_expr(copy); 
})

// Test that escaped characters resolve to the right values
EX_TEST(escaped_chars, {
    EVAL("'\\a'", '\a');
    EVAL("'\\b'", '\b');
    EVAL("'\\e'", '\e');
    EVAL("'\\f'", '\f');
    EVAL("'\\n'", '\n');
    EVAL("'\\r'", '\r');
    EVAL("'\\t'", '\t');
    EVAL("'\\v'", '\v');
})

