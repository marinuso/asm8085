/* asm8085 (C) 2019 Marinus Oosters */

#include "assembler.h"

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




// Sanity checks
char sanity_checks(const struct line *line) {
    char error = FALSE;
    
    int macdepth = 0;
    int i;
    char *l;
    
    for (; line != NULL; line=line->next_line) {
        if (line->instr.type == DIRECTIVE) {
            // Macro without name?
            i = line->instr.instr;
            if (i == DIR_macro) {
                if (line->label == NULL) { error_on_line(line, "macro without name"); error = TRUE; }
                macdepth++;
                if (macdepth<0) macdepth=1;
            }
            // equ without name
            else if (i == DIR_equ) {
                if (line->label == NULL) { error_on_line(line, "equ without name"); error=TRUE; }
            }
            // if/if(n)def/endm with name?
            else if (i == DIR_endm) {
                if (line->label != NULL) { error_on_line(line, "named endm"); error = TRUE; }
                macdepth--;
                if (macdepth < 0) { error_on_line(line, "endm without macro"); error = TRUE; }
                
            } else if (i == DIR_if || i==DIR_ifdef || i==DIR_endif || i==DIR_ifndef) {
                if (line->label != NULL) { error_on_line(line, "named if/endif"); error = TRUE; }
            }
        } else if (line->label != NULL) {
            // Label names OK?
            l = line->label;
            if (l[0] == '@') {
                if (macdepth <= 0) {
                    error_on_line(line, "@-label outside of macro");
                    error = TRUE;
                }
                l++;
            }
            if (l[0] != '_' && !isalpha(l[0])) {
                error_on_line(line, "label name must start with letter or underscore: %s", line->label);
                error = TRUE;
            }
            for (l++; *l; l++) {
                if (*l != '_' && !isalnum(*l)) {
                    error_on_line(line, "invalid label name: %s", line->label);
                    error = TRUE;
                    break;
                }
            }
        }
    }
    
    return error;
}