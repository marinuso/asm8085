/* asm8085 (C) 2019 Marinus Oosters */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

// Opcodes and assembler directives 

enum opcode {
    #define _OP(op) OP_##op,
    #include "instructions.h"
};

enum directive {
    #define _DIR(dir) DIR_##dir,
    #include "instructions.h"
};

/* Get the opcode number for s. Returns -1 if not a valid operator. */
enum opcode op_from_str(const char *s);

/* Get the directive number for s. Returns -1 if not a valid operator. */
enum directive dir_from_str(const char *s);

/* Represents an instruction */
struct instr {
    // NONE means an empty line or a line with only a label on it
    enum { NONE, OPCODE, DIRECTIVE, MACRO } type;
    int instr; // enum operator or enum directive, or -1. 
    char *text;
};

/* Represents an argument */
struct argmt {
    struct argmt *next_argmt;
    
    char parsed;            /* True if the argument has already been parsed */
    char *raw_text; 

    enum { REGISTER, REGPAIR, EXPRESSION } type;
};
    
/* Info for error message */
struct lineinfo {
    char *filename;
    int lineno;
};
    
/* Represents one line */
struct line {
    
    char *raw_text;         /* The raw text of the line */
    struct lineinfo info;   /* To generate error messages */
    
    struct line *next_line;
       
    /* A line can have a label, an instruction, and arguments */
    char *label;            /* Label, or NULL */
    struct instr instr;
    struct argmt *argmts; 
    int n_argmts;
};


/* Read a file, parsing the lines as it goes. 
 *
 * Returns NULL if the file cannot be opened. 
 */
struct line *read_file(const char *filename, char *error);

/* Free a line, recursively if needed (i.e. free all the following lines too). 
 * When freeing only one line, the next line is returned; otherwise NULL is returned.
 */
struct line *free_line(struct line *line, char recursive);

/* Parse a line. */
struct line *parse_line(const char *text, struct line *prev, const char *filename, char *error);


/* Print an error message given line info */
void error_on_line(FILE *file, const struct lineinfo *info, const char *message);


#endif