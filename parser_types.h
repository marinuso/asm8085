/* asm8085 (C) 2019 Marinus Oosters */

#ifndef __PARSER_TYPES_H__
#define __PARSER_TYPES_H__

/* Represents an instruction */
struct instr {
    // NONE means an empty line or a line with only a label on it
    enum { NONE, OPCODE, DIRECTIVE, MACRO } type;
    int instr; // enum operator or enum directive, or -1. 
    char *text;
};

/* Registers and register pairs */
enum reg_e    { R_INV = -1, RB, RC, RD, RE, RH, RL, RM, RA };
enum reg_pair { RP_INV = -1, RPB, RPD, RPH, RPSP, RPPSW=RPSP};

/* Represents an argument */
struct argmt {
    struct argmt *next_argmt;
    
    char parsed;            /* True if the argument has already been parsed */
    char *raw_text; 

    enum { REGISTER, REGPAIR, STRING, EXPRESSION } type;
    union {
        enum reg_e reg;
        enum reg_pair reg_pair;
        char *string;
        struct token_stack_node *expr; 
    } data; 
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

#endif