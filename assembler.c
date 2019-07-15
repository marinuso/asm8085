/* asm8085 (C) 2019 Marinus Oosters */

#include "assembler.h"

#define ERROR "%s: line %d: "

// Output on error: "<file>: line <line>: error\n"
static void error_on_line(struct line *line, char *message, ...) {
    fprintf(stderr, ERROR, line->info.filename, line->info.lineno);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}


