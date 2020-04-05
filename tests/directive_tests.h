/* asm8085 (C) 2020 Marinus Oosters */

// This file contains tests for the assembly directives

#include "../assembler.h"

TEST(dir_equ
, /* startup */
    struct asmstate *state = init_asmstate();
    struct line *lines;
, /* shutdown */
    if(state) free_asmstate(state);
    if(lines) free_line(lines,TRUE);
, /* test */
{
    
    intptr_t val;
    
    // Load, parse, and process the file with the definitions in it
    lines = assemble(state, "equtest.asm");
    if (lines == NULL) FAIL("processing failed");
    
    // Check that 'qux' is 3.
    if (!get_var(state->knowns, "qux", &val)) FAIL("qux not defined");
    if (val != 3) FAIL("qux not 3, but %d", (int)val);
    
    // Check that 'spam', 'ham', and 'eggs' exist but are undefined
    if (!get_var(state->unknowns, "spam", &val)) FAIL("spam not defined");
    if (!get_var(state->unknowns, "ham", &val)) FAIL("ham not defined");
    if (!get_var(state->unknowns, "eggs", &val)) FAIL("eggs not defined");
})

