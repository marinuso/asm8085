/* asm8085 (C) 2019-20 Marinus Oosters */

#include "parser.h"

#define LINE_BUF_SIZE 512 

#define PARSE_ERROR "%s: line %d: parse error: "

// Print an error message given line info
static void parse_error_line(FILE *file, const struct lineinfo *info, const char *message, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, 1024, message, args);
    fprintf(file, PARSE_ERROR "%s\n", info->filename, info->lineno, buffer);
    va_end(args);
}

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
    if(line->info.lastlabel) free(line->info.lastlabel);
    if(line->instr.text) free(line->instr.text);
    if(line->argmts) free_argmt(line->argmts);
    if(line->bytes) free(line->bytes);
}

/* Free a line, recursively if needed (i.e. free all the following lines too). 
 * When freeing only one line, the next line is returned; otherwise NULL is returned.
 */
struct line *free_line(struct line *line, char recursive) {
    struct line *next;
    do { 
        next = line->next_line;
        free_line_mem(line);
        line = next; 
    } while (next != NULL && recursive);
    
    return next;
}

/* Read a file, parsing the lines as it goes. 
 */
struct line *read_file(const char *filename) {
    
    char error = FALSE;
    
    struct line *begin = NULL, *prev = NULL, *cur = NULL;
    int idx;
    char line_buf[LINE_BUF_SIZE];
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        fprintf(stderr, "%s: cannot open file: %s\n", filename, strerror(errno));
        return NULL;
    }
    
    
    while (!feof(file)) {
        // Read a line      
        if (fgets(line_buf, LINE_BUF_SIZE, file) == NULL) break; // no more characters left
        idx = strlen(line_buf);
        
        // remove '(\r)\n' from end
        if (line_buf[idx-1] == '\n') line_buf[idx-1] = '\0';
        if (line_buf[idx-2] == '\r') line_buf[idx-2] = '\0';
        
        // Parse the line
        prev = cur; 
        cur = parse_line(line_buf, prev, filename, &error);
        if (cur == NULL) {
            FATAL_ERROR("failed to allocate memory for line");
        } else if (prev == NULL) {
            begin = cur; 
        }
    }
    
    if (error) {
        fprintf(stderr, "%s: there were errors parsing the file.\n", filename);
        free_line(begin, TRUE);
        return NULL;
    }
    
    return begin;

}

/* label ends with space or ':' */
int isLabelEnd(int ch) {
    return ch==':' || isspace(ch);
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
        label = copy_string_pred(ptr, isLabelEnd, TRUE);
        length = strlen(label);
        ptr += length;
        
        l->label = label;
        // If label starts with '.', it's not a top-level label
        if (label[0] != '.') l->info.lastlabel = copy_string(label);
    }
    
    // Skip ahead to next non-label character or end of line
    while (*ptr && isLabelEnd(*ptr)) ptr++;
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
    char *parse_buf = NULL, strdelim=0;
    int bracket_depth = 0, length = 0;
    struct argmt *prev = NULL, *cur = NULL;
    int parse_buf_size = LINE_BUF_SIZE;
  
    l->n_argmts = 0;
    l->argmts = NULL;
        
    if (!*ptr) {
        // There are no arguments.
        return;
    } else {
        parse_buf = malloc(parse_buf_size);
        if (parse_buf == NULL) FATAL_ERROR("malloc() failed");
    }
    
    while (*ptr) {
        // Scan until nonwhitespace is reached.
        while (*ptr && isspace(*ptr)) ptr++;
        if (!*ptr) break;
        
        // Copy until ',' or end-of-string is reached, or the buffer is full.
        length = 0;
        bracket_depth = 0;
        strdelim = '\0';
        
        while (*ptr && !(bracket_depth == 0 && strdelim == 0 && *ptr == ',')) {
            // add more memory if necessary
            if (length >= parse_buf_size-1) {
                parse_buf_size *= 2;
                parse_buf = realloc(parse_buf, parse_buf_size);
                if (parse_buf == NULL) FATAL_ERROR("realloc() failed");
            }
            
            parse_buf[length++] = *ptr;
            
            // Check for string begin and end
            if (!strdelim && (*ptr == '"' || *ptr == '\'' || *ptr == '`')) {
                strdelim = *ptr;
            } else if (strdelim && *ptr == strdelim) {
                strdelim = 0;
            } else if (*ptr == '\\') {
                // Handle escaped chracter string delimiter
                parse_buf[length++] = *++ptr;
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
            parse_error_line(stderr, &l->info, "mismatched brackets.");
            *error = 1;
        } else if(strdelim != 0) {
            parse_error_line(stderr, &l->info, "non-terminated string.");
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

    free(parse_buf);
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
        l->info.lastlabel = copy_string(prev->info.lastlabel);
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
    
    //fprintf(stderr, "[%s],[%d],[%d],[%d]<-%s\n", l->label, l->instr.type, l->instr.instr, l->n_argmts, l->raw_text);
    
    // If there was a comment, restore it.
    if (comment) *comment = ';';
    
    l->next_line = NULL; 
    
    return l;
}


// Get opcode number, -1 if invalid.
enum opcode op_from_str(const char *s) {
    int n = 0;
    #define _OP(op, _) if (strcasecmp(s, #op)) n++; else return n;
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


/* Parse a register */
enum reg_e parse_reg(const char *t) {
    if (!strcasecmp("a",t)) return RA; 
    if (!strcasecmp("b",t)) return RB;
    if (!strcasecmp("c",t)) return RC;
    if (!strcasecmp("d",t)) return RD;
    if (!strcasecmp("e",t)) return RE;
    if (!strcasecmp("h",t)) return RH;
    if (!strcasecmp("l",t)) return RL;
    if (!strcasecmp("m",t)) return RM;
   
    return R_INV;
}

/* Parse a register pair */
enum reg_pair parse_reg_pair(const char *t) {
    if (!strcasecmp("b",t))   return RPB;
    if (!strcasecmp("d",t))   return RPD;  
    if (!strcasecmp("h",t))   return RPH;  
    if (!strcasecmp("sp",t))  return RPSP; 
    if (!strcasecmp("psw",t)) return RPSP; // SP and PSW use the same encoding
    
    return RP_INV;
}


char parse_octal(const char *t, const char **out) {
    char o = 0;
    for (; *t >= '0' && *t <= '7'; t++) {
        o *= 8;
        o += *t - '0';
    }
    *out = t;
    return o;
}

int hex(char t) {
    if (t >= '0' && t <= '9') return t-'0';
    if (t >= 'A' && t <= 'F') return t-'A'+10;
    if (t >= 'a' && t <= 'f') return t-'a'+10;
    return -1;
}

char parse_hex(const char *t, const char **out) {
    char h = 0;
    for (; hex(*t)>=0; t++) {
        h *= 16;
        h += hex(*t);
    }
    *out = t;
    return h;
}

    
/* Parse a string */
char *parse_str(const char *t) {
    char *s = calloc(strlen(t), sizeof(char));
    char *p;
    const char *o;
    char delim = '\0';
    
    // Check delimiter
    delim = *t;
    if (delim != '"' && delim != '\'') {
        goto error;
    }
    t++;
    
    for (p=s; *t && *t != delim; t++) {
        if (*t == '\\') {
            // escape
            t++;
            switch(*t) {
                case 'a': *p++='\a'; break;
                case 'b': *p++='\b'; break;
                case 'e': *p++='\e'; break;
                case 'f': *p++='\f'; break;
                case 'n': *p++='\n'; break;
                case 'r': *p++='\r'; break;
                case 't': *p++='\t'; break;
                case 'v': *p++='\v'; break;
                case '\\': *p++='\\'; break;
                case '\'': *p++='\''; break;
                case '"': *p++='"'; break;
                case 'x': *p++=parse_hex(t+1, &o); t=o-1; break;
                default:
                    // Octal number?
                    if (*t >= '0' && *t <= '7') {
                        *p++=parse_octal(t, &o);
                        t=o-1;
                    } else {
                        goto error;
                    }
            }
        } else {
            // normal character
            *p++ = *t;
        }
    }
    
    // Check that there is nothing but whitespace after the delimiter
    if (*t == delim) t++;
    for (; *t; t++) {
        if (!isspace(*t)) goto error;
    }
    
    return s; 
    
error:
    free(s);
    return NULL;
}

/* Parse an argument */
char parse_argmt(enum argmt_type types, struct argmt *argmt, const struct lineinfo *info) {
    char *s = trim_string(argmt->raw_text);
    char success = TRUE;
    
    switch ((int) types) {
        case REGISTER:
            argmt->data.reg = parse_reg(s);
            if (argmt->data.reg == R_INV) {
                parse_error_line(stderr, info, "invalid register: %s; expected a, b, c, d, e, f, h, l, or m.", s); 
                success = FALSE; 
            } else {
                argmt->type = REGISTER;
            }
            break;
            
        case REGPAIR:
            argmt->data.reg_pair = parse_reg_pair(s);
            if (argmt->data.reg_pair == RP_INV) {
                parse_error_line(stderr, info, "invalid register pair: %s; expected b, d, h, sp or psw.", s);
                success = FALSE;
            } else {
                argmt->type = REGPAIR;
            }
            break;
        
        case STRING:
        case (STRING | EXPRESSION):
            argmt->data.string = parse_str(s);
            if (argmt->data.string != NULL) {
                argmt->type = STRING;
                break;
            } else {
                if (!(types & EXPRESSION)) {
                    /* It's not allowed to also be an expression, so this is definitely an error */
                    parse_error_line(stderr, info, "invalid string: %s", s);
                    success = FALSE;
                    break;
                }
                /* Otherwise, fall through into EXPRESSION, as that's also a possibility */
            }  
            // below extra comment is needed verbatim to shut up new warning
            // fall through
        case EXPRESSION:
            argmt->data.expr = parse_expr(s, info); // This prints its own error messages if needed
            if (argmt->data.expr != NULL) {
                argmt->type = EXPRESSION;
            } else {
                success = FALSE;
            }
            break;
            
        default:
            FATAL_ERROR("invalid argument type code: %d (this is a bug)", types);
    }
    
    free(s);
    argmt->parsed = success;
    return success;
}

