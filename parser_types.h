/* asm8085 (C) 2019 Marinus Oosters */

#ifndef __PARSER_TYPES_H__
#define __PARSER_TYPES_H__

// Opcodes and assembler directives 
enum opcode {
    #define _OP(op) OP_##op,
    #include "instructions.h"
};

enum directive {
    #define _DIR(dir) DIR_##dir,
    #include "instructions.h"
};


/* Represents an instruction */
struct instr {
    // NONE means an empty line or a line with only a label on it
    enum instr_type { NONE, OPCODE, DIRECTIVE, MACRO } type;
    int instr; // enum operator or enum directive, or -1. 
    char *text;
};

/* Registers and register pairs */
enum reg_e    { R_INV = -1, RB, RC, RD, RE, RH, RL, RM, RA };
enum reg_pair { RP_INV = -1, RPB, RPD, RPH, RPSP, RPPSW=RPSP};


/* Argument type
 * These can be OR'ed and given to parse_arg if multiple types are allowed,
 * (but only (STRING | EXPRESSION) is allowed */
enum argmt_type { REGISTER = 1, REGPAIR = 2, STRING = 4, EXPRESSION = 8 };

/* Represents an argument */
struct argmt {
    struct argmt *next_argmt;
    
    char parsed;            /* True if the argument has already been parsed */
    char *raw_text; 

    enum argmt_type type;
    union {
        enum reg_e reg;
        enum reg_pair reg_pair;
        char *string;
        struct parsed_expr *expr; 
    } data; 
};
    
 
/* Info for error message and last label*/
struct lineinfo {
    char *filename;
    char *lastlabel; 
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
    
    /* Used during assembly */
    int visited;
    int n_bytes;
    char *bytes;
    int needs_process;
    int location;
    
};

// Deep copy of an argument (setting next to NULL)
struct argmt *copy_argmt(const struct argmt *argmt);

// Deep copy of an argument list
struct argmt *copy_argmt_list(const struct argmt *start);

// Deep copy of line (setting next to NULL)
struct line *copy_line(const struct line *line);


#endif