/* asm8085 (C) 2020 Marinus Oosters */

// This file contains tests for the assembly directives

#include "../assembler.h"

#define DIR_TEST(NAME,CODE) \
TEST(dir_##NAME \
,   /*startup*/ \
    struct asmstate *state = init_asmstate(); \
    struct line *lines; \
    intptr_t val; \
,   /*shutdown*/ \
    if(state) free_asmstate(state); \
    if(lines) free_line(lines,TRUE); \
,   /*test*/ \
    CODE \
)

#define CHECKVAR(NAME, VAL) do { \
    if (!get_var(state->knowns, #NAME, &val)) FAIL(#NAME " not defined"); \
    if (val != (VAL)) FAIL(#NAME " not " #VAL ", but %d", (int)val); \
} while(0)

DIR_TEST(equ, {

    // Load, parse, and process the file with the definitions in it
    lines = assemble(state, "test_inputs/equtest.asm");
    if (lines == NULL) FAIL("processing failed");
    
    // Check that 'qux' is 3.
    CHECKVAR(qux, 3);

    // Check that 'spam', 'ham', and 'eggs' exist but are unknown
    if (!get_var(state->unknowns, "spam", &val)) FAIL("spam not defined");
    if (!get_var(state->unknowns, "ham", &val)) FAIL("ham not defined");
    if (!get_var(state->unknowns, "eggs", &val)) FAIL("eggs not defined");
})

DIR_TEST(include, {
    
    lines = assemble(state, "test_inputs/includetest.asm");
    if (lines == NULL) FAIL("processing failed");
    
    // Check that 'inc3' is 30.
    CHECKVAR(inc3, 30);
})

DIR_TEST(org_db_dw_ds, {
    
    lines = assemble(state, "test_inputs/org_dbws_test.asm");
    if (lines == NULL) FAIL("processing failed");
    
    CHECKVAR(at100, 100);
    CHECKVAR(at104, 104);
    CHECKVAR(at108, 108);
    CHECKVAR(at120, 120);
})

DIR_TEST(if, {
    lines = assemble(state, "test_inputs/iftest.asm");
    if (lines == NULL) FAIL("processing failed");
    
    CHECKVAR(foo, 42);
    CHECKVAR(qux, 10);
    CHECKVAR(spam, 100);
    CHECKVAR(ham, 20);
    CHECKVAR(eggs, 30);
    CHECKVAR(bar, 47);
    
})


