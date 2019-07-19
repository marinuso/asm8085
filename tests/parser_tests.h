/* asm8085 (C) 2019 Marinus Oosters */

// This file contains tests for the functions in parser.c

#include "../parser.h"
#include "../expression.h"

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
    if ((v=parse_reg(#name)) != R##name) FAIL("for register %s, parser returned %d", #name, v); \
} while(0)
    
#define TEST_RP(name) do { \
    enum reg_pair v; \
    if ((v=parse_reg_pair(#name)) != RP##name) FAIL("for register pair %s, parser returned %d", #name, v); \
} while(0)

// Parse register names
TEST(parse_register, (void)0, (void)0, {
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
    s = parse_str(test); \
    if (strcmp(s, rslt)) FAIL("given string %s, expected \"%s\", but got \"%s\"", test, #rslt, s); \
    free(s); s = NULL; \
} while(0)
#define TEST_STRING(test) TEST_STRING_2(#test, test)

TEST(parse_string, char *s = NULL, if(s!=NULL) free(s), {
    TEST_STRING("hello");
    TEST_STRING("");
    TEST_STRING(" '' ");
    TEST_STRING(" \"\" ");
    TEST_STRING("\a\b\e\f\n\r\t\v\\\'\"");
    TEST_STRING("f\1e\02d\003c\34b\148a");
    TEST_STRING("\xD\xE\xA\xD\xBE\xEF");
}) 

// See if we can read a file
#define TEST_F_LINE(code) do { \
    if (line == NULL) FAIL("line was NULL"); \
    code; \
    line = line->next_line; \
} while(0)

TEST(parse_file, 
    /* startup */
    struct line *start = NULL;
    struct line *line = NULL;
    char tempfile[] = "/tmp/test_asm8085_XXXXXX";
    /* shutdown */
,   if (start != NULL) free_line(start, TRUE);
    unlink(tempfile); 
    /* test */
, {
    char error = 0;
    int fd = mkstemp(tempfile);
    if (fd == -1) FAIL("could not create temporary file");
    FILE *f = fdopen(fd, "w");
    fputs("                        \n",f);
    fputs("       mov    a,b       \n",f);
    fputs("       db     1,2,3,4   \n",f);
    fputs("label                   \n",f);
    fputs("label2 mov    c,d       \n",f);
    fclose(f);
    
    start = read_file(tempfile);
    if (error) FAIL("error reading file");
    line = start;
    
    TEST_F_LINE(LINE_CONTENTS(NULL, 0, NONE));
    TEST_F_LINE(LINE_CONTENTS(NULL, 2, OPCODE, OP_mov));
    TEST_F_LINE(LINE_CONTENTS(NULL, 4, DIRECTIVE, DIR_db));
    TEST_F_LINE(LINE_CONTENTS("label", 0, NONE));
    TEST_F_LINE(LINE_CONTENTS("label2", 2, OPCODE, OP_mov));
    
    if (line != NULL) FAIL("spurious extra line: '%s'", line->raw_text);
})

// Test argument parser
#define TEST_PARSE_ARGMT_CHOICE(typein, typeout) do { \
    if (! parse_argmt(typein, argmt, &l)) { \
        FAIL("cannot parse argument '%s' as type %s.", argmt->raw_text, #typein); \
    } else if (argmt->type != typeout) { \
        FAIL("expected type %s (%d), but got %d.", #typeout, typeout, argmt->type); \
    } \
    argmt = argmt->next_argmt; \
} while(0)
    
#define TEST_PARSE_ARGMT(type) TEST_PARSE_ARGMT_CHOICE(type, type)
    
TEST(parse_argmt, 
    /* startup */
    struct line *line = NULL;
,   /* shutdown */
    if(line != NULL) free_line(line, FALSE);
,   /* test */ 
{
    struct lineinfo l;
    l.filename = "test";
    l.lineno = 1;
    
    char error = FALSE;
    line = parse_line("  test  a,b,'string','another',5 + 6 * 7,8 * 9 + 10,psw", NULL, "test", &error);
    if (error) FAIL("parsing line gave error");
    if (line->n_argmts != 7) FAIL("not all arguments were parsed");
    
    struct argmt *argmt = line->argmts;
    TEST_PARSE_ARGMT(REGISTER);
    TEST_PARSE_ARGMT(REGPAIR);
    TEST_PARSE_ARGMT(STRING);
    TEST_PARSE_ARGMT_CHOICE(STRING|EXPRESSION, STRING);
    TEST_PARSE_ARGMT(EXPRESSION);
    TEST_PARSE_ARGMT_CHOICE(STRING|EXPRESSION, EXPRESSION);
    TEST_PARSE_ARGMT(REGPAIR);   
})


// Test if we can copy a line
TEST(copy_line, 
    /* startup */
    struct line *line = NULL;
    struct line *copy;
,   /* shutdown */
    if (line != NULL) free_line(line, FALSE);
,   /* test */
{
    struct lineinfo l;
    l.filename = "test";
    l.lineno = 1;
    
    char error = FALSE;
    line = parse_line("label test a,psw,5+6", NULL, "test", &error);
    if (error) FAIL("parsing line gave error");
    
    // Check that everything is there
    LINE_CONTENTS("label", 3, MACRO, "test");
    
    
    // Parse and test the arguments
    struct argmt *argmt = line->argmts;
    TEST_PARSE_ARGMT(REGISTER);
    TEST_PARSE_ARGMT(REGPAIR);
    TEST_PARSE_ARGMT(EXPRESSION);
    
    argmt = line->argmts;
    if (argmt->data.reg != RA) FAIL("arg 1 isn't register A"); argmt=argmt->next_argmt;
    if (argmt->data.reg_pair != RPPSW) FAIL("arg 2 isn't register pair PSW"); argmt=argmt->next_argmt;
    if (eval_expr(argmt->data.expr, NULL, &l, 0) != 5+6) FAIL("arg 3 doesn't evaluate to 5+6");
    
    // make a copy of the line and free the old one
    copy = copy_line(line);
    free_line(line, FALSE);
    line = copy;
    
    // see if everything is still there
    LINE_CONTENTS("label", 3, MACRO, "test");
    
    // Test the arguments
    argmt = line->argmts;
    if (argmt->data.reg != RA) FAIL("arg 1 isn't register A"); argmt=argmt->next_argmt;
    if (argmt->data.reg_pair != RPPSW) FAIL("arg 2 isn't register pair PSW"); argmt=argmt->next_argmt;
    if (eval_expr(argmt->data.expr, NULL, &l, 0) != 5+6) FAIL("arg 3 doesn't evaluate to 5+6");
    
})