#include "opcodes.h"

#define ERROR "%s: line %d: "


// Output on error: "<file>: line <line>: error\n"
static void error_on_line(const struct line *line, char *message, ...) {
    fprintf(stderr, ERROR, line->info.filename, line->info.lineno);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

// Declare N bytes for memory
void alloc_line_bytes(struct line *line, size_t amt) {
    line->n_bytes = amt;
    line->bytes = malloc(amt);
    if (line->bytes == NULL) FATAL_ERROR("could not allocate memory for line");
}

// Opcode, no arguments 
#define ARG_BYTE(val) { \
    /* Instruction consists of just one byte */ \
    alloc_line_bytes(line, 1); \
    line->bytes[0] = (val); \
    /* No futher processing needed */ \
    line->needs_process = FALSE; \
}

#define ARG_NONE(val) { \
    if (line->n_argmts != 0) { \
        error_on_line(line, "%s: takes no arguments", opcode); \
        return FALSE; \
    } \
    ARG_BYTE(val); \
}

// Opcode, constant 3-bit argument (for RST) 
#define ARG_3CONST(val) { \
    /* Retrieve value from line */ \
    if (line->n_argmts != 1) { \
        error_on_line(line, "%s: requires one constant argument in the range [0..7]", opcode); \
        return FALSE; \
    } \
    if (!parse_argmt(EXPRESSION, line->argmts, &line->info)) return FALSE; \
    /* Value must be already available */ \
    int v = eval_expr(line->argmts->data.expr, state->knowns, &line->info, line->location); \
    if (contains_undefined_names(line->argmts->data.expr, state->knowns) || v<0 || v>7) { \
        error_on_line(line, "%s: argument must be a constant in the range [0..7]", opcode); \
        return FALSE; \
    } \
    ARG_BYTE(val); \
}

// Opcode, register argument
#define ARG_R(val) { \
    /* Retrieve register from line */ \
    if (line->n_argmts != 1) { \
        error_on_line(line, "%s: requires one register argument", opcode); \
        return FALSE; \
    } \
    if (!parse_argmt(REGISTER, line->argmts, &line->info)) return FALSE; \
    enum reg_e r = line->argmts->data.reg; \
    ARG_BYTE(val); \
} 

// Opcode, register pair argument
#define ARG_RP(val) { \
    /* Retrieve register from line */ \
    if (line->n_argmts != 1) { \
        error_on_line(line, "%s: requires one register pair argument", opcode); \
        return FALSE; \
    } \
    if (!parse_argmt(REGPAIR, line->argmts, &line->info)) return FALSE; \
    enum reg_pair rp = line->argmts->data.reg_pair; \
    ARG_BYTE(val); \
} 

// Opcode, N-byte immediate argument
#define ARG_IMM(len, val) { \
    /* Retrieve expression from line */ \
    if (line->n_argmts != 1) { \
        error_on_line(line, "%s: requires one expression argument", opcode); \
        return FALSE; \
    } \
    if (!parse_argmt(EXPRESSION, line->argmts, &line->info)) return FALSE; \
    /* Instruction consists of opcode + (byte) argument */ \
    alloc_line_bytes(line, 1 + (len)); \
    line->bytes[0] = (val); \
    /* The expression needs to be evaluated at the end */ \
    line->needs_process = TRUE; \
}

// Opcode, register + immediate 8-bit argument
#define ARG_R8(val) { \
    /* Retrieve arguments */ \
    if (line->n_argmts != 2) { \
        error_on_line(line, "%s: requires a register and an expression argument", opcode); \
        return FALSE; \
    } \
    struct argmt *reg_a = line->argmts; \
    struct argmt *exp_a = reg_a->next_argmt; \
    if (!parse_argmt(REGISTER, reg_a, &line->info)) return FALSE; \
    if (!parse_argmt(EXPRESSION, exp_a, &line->info)) return FALSE; \
    enum reg_e r = reg_a->data.reg; \
    /* Instruction consists of opcode + 1-byte immediate argument */ \
    alloc_line_bytes(line, 2); \
    line->bytes[0] = (val); \
    /* The expression needs to be evaluated at the end */ \
    line->needs_process = TRUE; \
}

// Opcode, register pair + immediate 16-bit argument
#define ARG_RP16(val) { \
    /* Retrieve arguments */ \
    if (line->n_argmts != 2) { \
        error_on_line(line, "%s: requires a register pair and an expression argument", opcode); \
        return FALSE; \
    } \
    struct argmt *reg_a = line->argmts; \
    struct argmt *exp_a = reg_a->next_argmt; \
    if (!parse_argmt(REGPAIR, reg_a, &line->info)) return FALSE; \
    if (!parse_argmt(EXPRESSION, exp_a, &line->info)) return FALSE; \
    enum reg_pair rp = reg_a->data.reg_pair; \
    /* Instruction consists of opcode + 2-byte immediate argument */ \
    alloc_line_bytes(line, 3); \
    line->bytes[0] = (val); \
    /* The expression needs to be evaluated at the end */ \
    line->needs_process = TRUE; \
}

// Two registers (destination and source, for MOV) 
#define ARG_DS(val) { \
    /* Retrieve arguments */ \
    if (line->n_argmts != 2) { \
        error_on_line(line, "%s: requires two registers", opcode); \
        return FALSE; \
    } \
    struct argmt *dst_a = line->argmts; \
    struct argmt *src_a = dst_a->next_argmt; \
    if (!parse_argmt(REGISTER, dst_a, &line->info)) return FALSE; \
    if (!parse_argmt(REGISTER, src_a, &line->info)) return FALSE; \
    enum reg_e d = dst_a->data.reg; \
    enum reg_e s = src_a->data.reg; \
    /* Instruction consists of opcode */ \
    ARG_BYTE(val); \
}

// Define opcode generating functions
#define _OP(name, instr) \
int op_##name(struct asmstate *state) { \
    char opcode[] = #name; \
    struct line *line = state->cur_line; \
    instr; \
    return TRUE; \
} 

#include "instructions.h"


