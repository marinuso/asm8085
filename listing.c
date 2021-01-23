#include "listing.h"

// Write up to four bytes
void write_bytes(FILE *f, const struct line *l, int offset, int first) {
    const char *byte_out[] = { 
        "           ", // no bytes
        "%02X         ", // one byte
        "%02X %02X      ", // two bytes
        "%02X %02X %02X   ", // three bytes 
        "%02X %02X %02X %02X" // four bytes 
    }; 
    
    int n = l->n_bytes - offset;
    
    
    if (!first) fprintf(f,"            ");
    if (n<0) FATAL_ERROR("negative byte amount");
    
    const unsigned char *b = l->bytes + offset; 
    
    switch(n) {
        case 0: // No bytes
            fprintf(f, byte_out[0]);
            break;
        case 1:
            fprintf(f, byte_out[1], b[0]);
            break;
        case 2:
            fprintf(f, byte_out[2], b[0], b[1]);
            break;
        case 3:
            fprintf(f, byte_out[3], b[0], b[1], b[2]);
            break;
        default:    // 4 or up
            fprintf(f, byte_out[4], b[0], b[1], b[2], b[3]);
    }
    
    if (!first) fprintf(f, "\n");
}

void write_listing(FILE *f, const struct asmstate *state, const struct line *lines) {
    const struct line *line;
    int offset;
    intptr_t value = 0;
    
    // Handle all the lines
    for (line=lines; line!=NULL; line=line->next_line) {
        // Print line number, if it isn't auto-generated
        if (line->info.lineno == 0) fprintf(f, "      ");
        else fprintf(f, "%5d ", line->info.lineno);
        
        // If the line defines bytes, print the location 
        if (line->n_bytes > 0) fprintf(f, "%04X: ", line->location);
        // If it is an 'equ', print its value
        else if (line->instr.type == DIRECTIVE 
              && line->instr.instr == DIR_equ) {
         
            set_base(state->knowns, line->info.lastlabel);
            if (!get_var(state->knowns, line->label, &value)) {
                fprintf(f, "???? =");
            } else {
                fprintf(f, "%04X =", (unsigned short) value);
            }                
        } else fprintf(f, "      ");
        
        // For a binary include, don't print all the bytes
        if (line->instr.type == DIRECTIVE
         && line->instr.instr == DIR_incbin) {
             fprintf(f, "[.........] %s\n", line->raw_text);
        } else {
            // Print bytes, if there are any
            offset = 0;
            write_bytes(f, line, offset, TRUE);
        
            // Print rest of line
            fprintf(f, " %s\n", line->raw_text);
        
            // If there were more than 4 bytes, print the rest of the bytes on separate lines
            for (offset = 4; offset < line->n_bytes; offset += 4)
                write_bytes(f, line, offset, FALSE);
        }
    }
    
    // If there are no symbols defined, skip the symbol table
    if (state->knowns->variables == NULL) return; 
    
    // Then write the symbol table 
    fprintf(f, "\n\n");
    fprintf(f, "************************************************************\n");
    fprintf(f, "                        Symbol table                        \n");
    fprintf(f, "************************************************************\n");
    fprintf(f, "\n\n");
    
    fprintf(f, "Name                    = Value\n");
    fprintf(f, "-----------------------   ----------------------------------\n");
    
    struct variable *v;
    // They are in reverse order of occurrence, so find the first one
    for (v = state->knowns->variables; v != NULL && v->next != NULL; v = v->next);
    
    // And then print them in reverse order
    for (; v != NULL; v = v->prev) {
        fprintf(f, "%-23s = %04Xh\n", v->name, (unsigned short) v->value);
    }
}


        
        
        