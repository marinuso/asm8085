/* asm8085 (C) 2019 Marinus Oosters */

// This file contains tests for the functions in parser.c

#include "../parser.h"

// Macro: parse a line and do some sanity checks, then run code, and free line afterwards
#define TEST_LINE(str, code) do { \
    line = parse_line(str, NULL, "test", &error); \
    if (line == NULL) FAIL("parse_line gave NULL for %s", #str); \
    if (error) FAIL("parse_line set error for %s", #str); \
    if (strcmp(line->raw_text, str)) FAIL("raw_text does not match input string for %s", #str); \
    code; \
    if (line != NULL) { free_line(line, FALSE); line=NULL; } \
} while(0)

// Macro: test that the label is the given value
#define TEST_LABEL(val) do { \
    { \
        char *v = (val); /* to prevent the compiler from complaining about possibly passing NULL to strcmp */ \
        if( v == NULL ) { \
            if ( line->label != NULL) FAIL("expected no label, but found '%s'", line->label ); \
        } else { \
            if ( line->label == NULL) FAIL("expected label %s, but found NULL", #val); \
            if ( strcmp(line->label, v) ) FAIL("expected label %s, but found '%s'", #val, line->label); \
        } \
    } \
} while(0)        

// Macro: test for a given instruction
#define TEST_INSTR_NONE() 
#define TEST_INSTR_OP_OR_DIR(ood, val) do { \
    if (line->instr.instr != val) FAIL("expected %s %s (%d), but found %d", #ood, #val, val, line->instr.instr); \
} while(0)
#define TEST_INSTR_OPCODE(val) TEST_INSTR_OP_OR_DIR(opcode, val)
#define TEST_INSTR_DIRECTIVE(val) TEST_INSTR_OP_OR_DIR(directive, val)
#define TEST_INSTR_MACRO(val) do { \
    if (line->instr.text == NULL) FAIL("macro name is NULL, expected %s", #val); \
    if (strcmp(line->instr.text, val)) FAIL("macro name is \"%s\", expected %s", line->instr.text, #val); \
} while(0)
    
#define TEST_INSTR(typ, ...) do { \
    if (line->instr.type != typ) FAIL("expected instruction type %s (%d), but found %d", #typ, typ, line->instr.type); \
    TEST_INSTR_##typ (__VA_ARGS__); \
} while(0)

#define TEST_N_ARGMTS(n) do { if(line->n_argmts != (n)) FAIL("expected %d arguments, but found %d", n, line->n_argmts); } while(0)

// Macro: test all parts of a line
#define LINE_CONTENTS(lab, argn, typ, ...) do { TEST_LABEL(lab); TEST_INSTR(typ, ##__VA_ARGS__); TEST_N_ARGMTS(argn); } while(0)

// Macro: test an argument
#define TEST_ARGMT(text) do { \
    if (strcmp(s = trim_string(arg->raw_text), text)) \
        FAILC("expected \"%s\", but got \"%s\"", free(s), text, s); \
    else { \
        free(s); \
        arg=arg->next_argmt; \
    } \
} while(0)

// Parse various types of line
TEST(parse_line, struct line *line, if(line != NULL) free_line(line, FALSE), {
    char error = 0;

    
    /* an empty line should be empty */
    TEST_LINE("", LINE_CONTENTS(NULL, 0, NONE));

    /* line with only a label on it */
    TEST_LINE("label", LINE_CONTENTS("label", 0, NONE));
    
    /* line with only an instruction on it */
    TEST_LINE(" nop", LINE_CONTENTS(NULL, 0, OPCODE, OP_nop));
    
    /* line with label and instruction on it */
    TEST_LINE("label nop", LINE_CONTENTS("label", 0, OPCODE, OP_nop));
    
    /* line with arguments */
    TEST_LINE(" mov a,b", LINE_CONTENTS(NULL, 2, OPCODE, OP_mov));
    
    /* line with label, instruction, and arguments */
    TEST_LINE("label mov a,b", LINE_CONTENTS("label", 2, OPCODE, OP_mov));
    
    /* commenting anything out should make it ignore that bit */
    TEST_LINE("label mov a;,b", LINE_CONTENTS("label", 1, OPCODE, OP_mov));
    TEST_LINE("label mov ;a,b", LINE_CONTENTS("label", 0, OPCODE, OP_mov));
    TEST_LINE("label ;mov a,b", LINE_CONTENTS("label", 0, NONE));
    TEST_LINE("lab;el mov a,b", LINE_CONTENTS("lab", 0, NONE));
    TEST_LINE(";label mov a,b", LINE_CONTENTS(NULL, 0, NONE));
    
    /* line with directive on it */
    TEST_LINE(" db 1,2,3", LINE_CONTENTS(NULL, 3, DIRECTIVE, DIR_db));
    
    /* line with "macro invocation" on it */
    TEST_LINE(" macro_name_here arg,arg,arg", LINE_CONTENTS(NULL, 3, MACRO, "macro_name_here"));
    
    /* check argument parsing */
    TEST_LINE("label db 1, (2, 2), \"3, '3\", '4, \"4', 5, '(6', \"7)\", 'comment; char; in; string'; real comment", {
        struct argmt *arg;
        char *s;
        LINE_CONTENTS("label", 8, DIRECTIVE, DIR_db);
        arg = line->argmts;
        TEST_ARGMT("1");
        TEST_ARGMT("(2, 2)");
        TEST_ARGMT("\"3, '3\"");
        TEST_ARGMT("'4, \"4'");
        TEST_ARGMT("5");
        TEST_ARGMT("'(6'");
        TEST_ARGMT("\"7)\"");
        TEST_ARGMT("'comment; char; in; string'"); 
        // this should've been the last argument
        if (arg != NULL) FAIL("spurious extra argument: '%s'", arg->raw_text);
    });  
})


#define MKLINEINFO \
    struct lineinfo l; \
    l.lineno = 1; \
    l.filename = "test"; 

        
#define TEST_REG(name) do { \
    enum reg_e v; \
    if ((v=parse_reg(#name, &l, &error)) != R##name) FAIL("for register %s, parser returned %d", #name, v); \
} while(0)
    
#define TEST_RP(name) do { \
    enum reg_pair v; \
    if ((v=parse_reg_pair(#name, &l, &error)) != RP##name) FAIL("for register pair %s, parser returned %d", #name, v); \
} while(0)

// Parse register names
TEST(parse_register, MKLINEINFO, (void)0, {
    char error;
    TEST_REG(A);
    TEST_REG(B);
    TEST_REG(C);
    TEST_REG(D);
    TEST_REG(E);
    TEST_REG(H);
    TEST_REG(L);
    TEST_REG(M);
    TEST_RP(B);
    TEST_RP(D);
    TEST_RP(H);
    TEST_RP(SP);
    TEST_RP(PSW);
})

// Parse a string
#define TEST_STRING_2(test, rslt) do { \
    s = parse_str(test, &l, &error); \
    if (strcmp(s, rslt)) FAIL("given string %s, expected \"%s\", but got \"%s\"", test, #rslt, s); \
    free(s); s = NULL; \
} while(0)
#define TEST_STRING(test) TEST_STRING_2(#test, test)

TEST(parse_string, MKLINEINFO; char *s = NULL, if(s!=NULL) free(s), {
    char error;
    TEST_STRING("hello");
    TEST_STRING("");
    TEST_STRING(" '' ");
    TEST_STRING(" \"\" ");
    TEST_STRING("\a\b\e\f\n\r\t\v\\\'\"");
    TEST_STRING("f\1e\02d\003c\34b\148a");
    TEST_STRING("\xD\xE\xA\xD\xBE\xEF");
}) 