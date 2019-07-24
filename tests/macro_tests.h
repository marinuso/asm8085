/* asm8085 (C) 2019 Marinus Oosters */


#include "../macro.h"
#include "../parser.h"
#include "../util.h"

#define LINE(x) do { \
    error = FALSE; \
    l = parse_line(x, l, "test", &error); \
    if(error) { \
        free_line(l, FALSE); \
        FAIL("error parsing line: %s", x); \
    } \
} while(0)

#define M_TEST_LINE_TYPE(line,typ) do { \
    if (line->instr.type != typ) FAIL("%s: expected %s (%d), got %d", line->raw_text, #typ, typ, line->instr.type); \
} while(0)
    
#define M_TEST_LINE_OPCODE(line,val) do { \
    M_TEST_LINE_TYPE(line,OPCODE); \
    if (line->instr.instr != OP_##val) FAIL("%s: expected %s (%d), got %d", line->raw_text, #val, OP_##val, line->instr.instr); \
} while(0)

#define M_TEST_LINE_MACRO(line,val) do { \
    M_TEST_LINE_TYPE(line,MACRO); \
    if (line->instr.text == NULL) FAIL("%s: no text in line instruction", line->raw_text); \
    if (!strcmp(line->instr.text,val) FAIL("%s: expected %s, got %s", line->raw_text, val, line->instr.text); \
} while(0)

#define M_TEST_LINE_LABEL(line,lab) do { \
    {   \
        char *s = lab; \
        if (s == NULL) { \
            if (line->label != NULL) FAIL("%s: expected no label, but got %s", line->raw_text, line->label); \
        } else if (line->label == NULL) { \
            FAIL("%s: expected label %s, but got no label", line->raw_text,  s); \
        } else if (strcmp(s, line->label)) { \
            FAIL("%s: expected label %s, but got %s", line->raw_text, s, line->label); \
        } \
    } \
} while(0)

#define M_TEST_LINE(line, label, type, val, argsn) do { \
    if (label!=NULL) M_TEST_LINE_LABEL(line, label); \
    M_TEST_LINE_##type(line, val); \
    if (line->n_argmts != argsn) FAIL("%s: expected %d arguments, but got %d", line->raw_text, argsn, line->n_argmts); \
} while(0)

#define M_TEST_ARG(s, line, val) do { \
    { \
        char *trval; \
        char *trarg; \
        int cmp; \
        if (s || argmt == NULL) argmt = line->argmts; \
        if (argmt->raw_text == NULL) FAIL("%s: argument text is NULL", line->raw_text); \
        trval = trim_string(val); \
        trarg = trim_string(argmt->raw_text); \
        cmp = strcmp(trval, trarg); \
        free(trval); \
        free(trarg); \
        if(cmp) FAIL("%s: expected argument %s, but got %s", line->raw_text, val, argmt->raw_text); \
        argmt=argmt->next_argmt; \
    } \
} while(0)

TEST(macros, 
  /*startup*/
    struct line *mac_lines = NULL;
    struct line *mac_expand = NULL;
    struct maclist *maclist = NULL;
, /*shutdown*/
    if (mac_lines) free_line(mac_lines, TRUE);
    if (mac_expand) free_line(mac_expand, TRUE);
    if (maclist) free_maclist(maclist);
, /*test*/
{
    /* set up a macro */
    struct line *l = NULL;
    struct line *endm_def = NULL;
    const struct line *endm_out = NULL;
    struct line *mac_last = NULL;
    struct argmt *argmt = NULL;
    char *lc_label;
    
    char error = FALSE;
    
    LINE("testmacro macro one,two,three");
    mac_lines = l;
    LINE("!one !two !three");
    LINE("@label !two !three");
    LINE(" mov a,b");
    LINE(" jz @label");
    LINE(" endm");
    endm_def = l;
    LINE("not part of,macro");
    
    /* can we define the macro? */
    struct macro *m = define_macro(mac_lines, &endm_out);
    if (!m) FAIL("define_macro returned NULL");
    if (endm_out != endm_def) FAIL("endm line doesn't match, def(%p)=%s, out(%p)=%s", 
          endm_def, endm_def->raw_text, endm_out, endm_out->raw_text);
    
    maclist = add_macro(m, maclist);
    
    /* set up a macro expansion */
    l = NULL;
    LINE(" testmacro a,mov,(c,d)");
    
    /* invoke it */
    mac_expand = expand_macro(l, maclist, &mac_last);
    if (!mac_expand) FAIL("expand_macro returned NULL");

    /* test if the correct substitutions were made */
    l = mac_expand;
    M_TEST_LINE(l, "a", OPCODE, mov, 2);
    M_TEST_ARG(TRUE, l, "c");
    M_TEST_ARG(FALSE, l, "d");
    l=l->next_line;
    lc_label = l->label; /* get value for "@label" */
    M_TEST_LINE(l, NULL, OPCODE, mov, 2);
    M_TEST_ARG(TRUE, l, "c");
    M_TEST_ARG(FALSE, l, "d");
    l=l->next_line;
    M_TEST_LINE(l, NULL, OPCODE, mov, 2);
    M_TEST_ARG(TRUE, l, "a");
    M_TEST_ARG(FALSE, l, "b");
    l=l->next_line;
    M_TEST_LINE(l, NULL, OPCODE, jz, 1);
    M_TEST_ARG(TRUE, l, lc_label); /* "@label" must match earlier one */
    l=l->next_line;
    if (l != NULL) FAIL("spurious line: %s", l->raw_text);   
    
})