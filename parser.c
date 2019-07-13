/* asm8085 (C) 2019 Marinus Oosters */

#include "parser.h"

#define LINE_BUF_SIZE 512      // 512 characters ought to be enough for everybody

#define PARSE_ERROR "%s: line %d: parse error: "

/* Free all the memory associated with an argument */
void free_argmt(struct argmt *argmt) {
    struct argmt *next;
    while(argmt != NULL) {
        next = argmt->next_argmt;
        free(argmt->raw_text);
        free(argmt);
        argmt = next;
    }
}


/* Free all the memory associated with one line. */
void free_line_mem(struct line *line) {
    if(line->raw_text) free(line->raw_text);
    if(line->label) free(line->label);
    if(line->info.filename) free(line->info.filename);
    if(line->instr.text) free(line->instr.text);
    if(line->argmts) free_argmt(line->argmts);
}

/* Free a line, recursively if needed (i.e. free all the following lines too). 
 * When freeing only one line, the next line is returned; otherwise NULL is returned.
 */
struct line *free_line(struct line *line, char recursive) {
    struct line *next;
    do {
        next = line->next_line;
        free_line_mem(line);
    } while (next != NULL && recursive);
    
    return next;
}

/* Read a file, parsing the lines as it goes. 
 */
struct line *read_file(const char *filename, char *error) {
    
    struct line *begin = NULL, *prev = NULL, *cur = NULL;
    int idx;
    char line_buf[LINE_BUF_SIZE], ch;
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        fprintf(stderr, "%s: cannot open file.\n", filename);
        *error = TRUE;
        return NULL;
    }
    
    
    while (!feof(file)) {
        // Read a line
        idx = 0;
        do {
            line_buf[idx++] = ch = fgetc(file);
        } while (!feof(file) && ch != '\n' && idx < LINE_BUF_SIZE);
        line_buf[idx] = '\0';
        
        // Parse the line
        prev = cur; 
        cur = parse_line(line_buf, prev, filename, error);
        if (cur == NULL) {
            FATAL_ERROR("failed to allocate memory for line");
        } else if (prev == NULL) {
            begin = cur; 
        }
    }
    
    if (*error) {
        fprintf(stderr, "%s: there were errors parsing the file.\n", filename);
    }
    
    return begin;

}
    
/* Set the label, if there is one. Returns pointer to start of instruction or to end of string. */
char *parse_label(struct line *l) {
    char *label, *ptr = l->raw_text;
    int length = 0;
    if (*ptr == '\0') {
        // Empty line, no label 
        l->label = NULL;
    } else if (isspace(*ptr)) {
        // This line starts with whitespace and has no label.
        l->label = NULL;
    } else {
        // There is a label
        label = copy_string_pred(ptr, isspace, TRUE);
        length = strlen(label);
        ptr += length;
        
        // If the label ends with ':', that's not part of the label.
        if (label[length-1] == ':') {
            label[length-1] = '\0';
        }
        
        l->label = label;
    }
    
    // Skip ahead to next nonwhitespace character or end of line
    while (*ptr && isspace(*ptr)) ptr++;
    return ptr;
}
        
/* Set the instruction */
const char *parse_instruction(struct line *l, const char *ptr) {
    if (!*ptr) {
        // There is no instruction on this line.
        l->instr.type = NONE;
        l->instr.instr = -1;
        l->instr.text = NULL;
    } else {
        // Get the instruction
        l->instr.text = copy_string_pred(ptr, isspace, TRUE);
        ptr += strlen(l->instr.text);
        
        // See if it is an opcode or directive
        if ((l->instr.instr = op_from_str(l->instr.text)) != -1) {
            l->instr.type = OPCODE;
        } else if ((l->instr.instr = dir_from_str(l->instr.text)) != -1) {
            l->instr.type = DIRECTIVE;
        } else if (!strcmp(l->instr.text, "=")) {
            // Special case: = is an alias for 'equ'.
            l->instr.type = DIRECTIVE;
            l->instr.instr = DIR_equ;
        } else {
            // It's not a valid opcode or directive, so assume for now it's a macro. 
            l->instr.type = MACRO;
        }
       
        // Scan ahead to next nonwhitespace character or end of line
        while (*ptr && isspace(*ptr)) ptr++;
    }
    
    return ptr;
}
   
/* Parse the arguments (split on commas that aren't in brackets or strings) */
void parse_arguments(struct line *l, const char *ptr, char *error) {
    char parse_buf[LINE_BUF_SIZE], strdelim=0;
    int bracket_depth = 0, length = 0;
    struct argmt *prev = NULL, *cur = NULL;
  
    l->n_argmts = 0;
    l->argmts = NULL;
        
    if (!*ptr) {
        // There are no arguments.
        return;
    }
    
    while (*ptr) {
        // Scan until nonwhitespace is reached.
        while (*ptr && isspace(*ptr)) ptr++;
        if (!*ptr) break;
        
        // Copy until ',' or end-of-string is reached, or the buffer is full.
        length = 0;
        bracket_depth = 0;
        strdelim = '\0';
        
        while (*ptr && !(bracket_depth == 0 && strdelim == 0 && *ptr == ',') && length<LINE_BUF_SIZE) {
            parse_buf[length++] = *ptr;
            
            // Check for string begin and end
            if (!strdelim && (*ptr == '"' || *ptr == '\'')) {
                strdelim = *ptr;
            } else if (strdelim && *ptr == strdelim) {
                strdelim = 0;
            } else if (strdelim && *ptr == '\\' && *(ptr+1) == strdelim) {
                // Handle escaped string delimiter
                parse_buf[length++] = *ptr++;
            }
            
            // Handle brackets     
            if (!strdelim) {
                if (*ptr == '(') bracket_depth++;
                else if (*ptr == ')') bracket_depth--;
            }
            
            ptr++;
        }
        
        // Check for errors
        if (bracket_depth != 0) {
            error_on_line(stderr, &l->info, "mismatched brackets.");
            *error = 1;
        } else if(strdelim != 0) {
            error_on_line(stderr, &l->info, "non-terminated string.");
            *error = 1;
        } else {
            parse_buf[length] = '\0';
                        
            // It's OK, so store the current argument.
            prev = cur;
            
            // Allocate memory
            cur = calloc(1, sizeof(struct argmt));
            if (!cur) {
                FATAL_ERROR("failed to allocate memory for argument");
            }
           
            // If this is the first argument, store it in the line. Otherwise, chain it to the previous one.
            if (l->argmts == NULL) {
                l->argmts = cur;
            } else {
                prev->next_argmt = cur;
            }
           
            // There is one more argument than before.
            l->n_argmts++;

            cur->raw_text = copy_string(parse_buf);
            cur->parsed = FALSE;
            cur->next_argmt = NULL; 
        }

        // Skip over the ',' we might be on.
        if (*ptr == ',') ptr++;
        
    } 
}

/* Zero out the first comment character (;) that isn't in a string. Returns location if one was found. */
char *comment_stop(char *ptr) {
    char strdelim;
    for (strdelim='\0'; *ptr; ptr++) {  
        if (!strdelim) {
            // Not in string. 
            
            // Comment? Then stop.
            if (*ptr == ';') { *ptr = '\0'; return ptr; }
            // String start?
            else if (*ptr == '"' || *ptr == '\'') { strdelim = *ptr; }
        } else {
            
            // Escaped character?
            if (*ptr == '\\' && *(ptr+1)) { ptr++; }
            // String end?
            else if (*ptr == strdelim) { strdelim = '\0'; }
        }
    }
    
    return NULL;
}
                
/* Parse a line. */
struct line *parse_line(const char *text, struct line *prev, const char *filename, char *error) {
    char *comment;
    const char *parse_ptr;
    struct line *l = calloc(1, sizeof(struct line));
   
    if (!l) FATAL_ERROR("failed to allocate space for line");
    
    // Link this line to the previous line if there is one
    if (prev) {
        prev->next_line = l;
        l->info.lineno = prev->info.lineno + 1;
    } else {
        l->info.lineno = 1;
    }
    
    // Copy the text and filename across
    l->raw_text = copy_string(text);
    comment = comment_stop(l->raw_text);
    l->info.filename = copy_string(filename);
    
    // Parse the three parts of the line
    parse_ptr = parse_label(l);
    parse_ptr = parse_instruction(l, parse_ptr);
    parse_arguments(l, parse_ptr, error);
    
    // If there was a comment, restore it.
    if (comment) *comment = ';';
    
    return l;
}


// Get opcode number, -1 if invalid.
enum opcode op_from_str(const char *s) {
    int n = 0;
    #define _OP(op) if (strcasecmp(s, #op)) n++; else return n;
    #include "instructions.h"
    return -1;
}

// Get directive number, -1 if invalid.
enum directive dir_from_str(const char *s) {
    int n = 0;
    #define _DIR(dir) if (strcasecmp(s, #dir)) n++; else return n;
    #include "instructions.h"
    return -1;
}


// Print an error message given line info
void error_on_line(FILE *file, const struct lineinfo *info, const char *message) {
    fprintf(stderr, PARSE_ERROR "%s\n", info->filename, info->lineno, message);
}

