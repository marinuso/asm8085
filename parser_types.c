/* asm8085 (C) 2019 Marinus Oosters 
 *
 * Functions to make copies of data structures in parser_types.h 
 */

#include "expression.h"
#include "util.h"
#include "parser_types.h"


// Deep copy of an argument (setting next to NULL)
struct argmt *copy_argmt(const struct argmt *argmt) {
    struct argmt *copy = calloc(1, sizeof(struct argmt));
    if (copy == NULL) FATAL_ERROR("failed to allocate memory for copy of argmt");
    
    copy->next_argmt = NULL;
    copy->raw_text = copy_string(argmt->raw_text);
    copy->parsed = argmt->parsed;
    
    // If the argument has already been parsed, copy over the parsed data
    if (copy->parsed) {
        copy->type = argmt->type;
        
        switch (copy->type) {
            case EXPRESSION:
                // Make a deep copy of the parsed expression
                copy->data.expr = copy_parsed_expr(argmt->data.expr);
                break;
            case STRING:
                // Make a copy of the string
                copy->data.string = copy_string(argmt->data.string);
                break;
            case REGISTER:
                copy->data.reg = argmt->data.reg;
                break;
            case REGPAIR:
                copy->data.reg_pair = argmt->data.reg_pair;
                break;
            default:
                FATAL_ERROR("invalid argument type while copying argument: %d", copy->type);
        }
    }
    
    return copy;
}

// Deep copy of an argument list
struct argmt *copy_argmt_list(const struct argmt *argmts) {
    struct argmt *copy_start = NULL, *copy_ptr = NULL, *copy;
    for (; argmts != NULL; argmts=argmts->next_argmt) {
        copy = copy_argmt(argmts);
        if (copy_start == NULL) {
            copy_start = copy;
        } else {
            copy_ptr->next_argmt = copy;
        }
        copy_ptr = copy;
    }
    return copy_start;
}

// Deep copy of line (setting next to NULL)
struct line *copy_line(const struct line *line) {
    struct line *copy = malloc(sizeof(struct line));
    if (copy == NULL) FATAL_ERROR("failed to allocate memory for copy of line");
    
    copy->raw_text = copy_string(line->raw_text);
    copy->info.lineno = line->info.lineno;
    copy->info.filename = copy_string(line->info.filename);
    copy->next_line = NULL;
    
    copy->label = line->label == NULL ? NULL : copy_string(line->label);
    copy->instr.type = line->instr.type;
    copy->instr.instr = line->instr.instr;
    if (line->instr.instr != NONE && line->instr.text != NULL) {
        copy->instr.text = copy_string(line->instr.text);
    } else {
        copy->instr.text = NULL;
    }
    
    copy->n_argmts = line->n_argmts;
    copy->argmts = copy_argmt_list(line->argmts);
    
    return copy;
}
    
