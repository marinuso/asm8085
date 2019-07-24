/* asm8085 (C) 2019 Marinus Oosters */

#include "util.h"

#define RPL_BLK_SZ 1024

// Replace substrings in string, except within "..." or '...'
char *string_replace(const char *str, const struct replacement *rpls, int n_replacements) {
    char *out = malloc(sizeof(char) * RPL_BLK_SZ);
    int *olens = malloc(sizeof(int) * n_replacements);
    int *nlens = malloc(sizeof(int) * n_replacements);
    int i, sidx, size;
    char replaced = FALSE;
    char strdelim = '\0';
    char escaped = FALSE;
    
    if (out == NULL || olens == NULL || nlens == NULL) {
        if(out) free(out);
        if(olens) free(olens);
        if(nlens) free(nlens);
        FATAL_ERROR("failed to allocate memory during string replacement");
    }
    
    // Precalculate lengths of search and replacement strings
    for (i=0; i<n_replacements; i++) {
        olens[i] = strlen(rpls[i].old);
        nlens[i] = strlen(rpls[i].new);
    }
    
    sidx = 0;
    size = RPL_BLK_SZ;
    while (*str) {
        replaced = FALSE;
        
        // Do not apply replacements within string literals
        if (escaped) {
            // Next character is not escaped, but this one is copied verbatim
            escaped = FALSE;
        } else if (!strdelim && (*str=='"' || *str=='\'')) {
            // Start of string
            strdelim = *str;
        } else if (strdelim && *str=='\\') {
            // Next character is escaped
            escaped = TRUE;
        } else if (strdelim && *str==strdelim) {
            // End of string
            strdelim = '\0';
        } else if (!strdelim) {
            // Not in a string, so attempt to apply replacement
            
            for (i=0; i<n_replacements; i++) {
                // String starts with replacement?
                if (! strncmp(rpls[i].old, str, olens[i])) {
                    // Allocate more memory if necessary
                    if (sidx + nlens[i] >= size) {
                        size += RPL_BLK_SZ;
                        out = realloc(out, sizeof(char) * size);
                        if (out == NULL) FATAL_ERROR("failed to allocate memory during string replacement");
                    }
                    // Write replacement
                    memcpy(out+sidx, rpls[i].new, nlens[i]);
                    sidx += nlens[i];
                    str += olens[i];
                    replaced = TRUE;
                    break;
                }
            }
        }
        
        if (!replaced) {
            // Write current character unchanged
            if (sidx >= size) {
                // Allocate more memory if necessary
                size += RPL_BLK_SZ;
                out = realloc(out, sizeof(char) * size);
            }
            out[sidx++] = *str;
            str++;
        }
    }
    
    // zero-terminate the string
    out[sidx++] = '\0';
    
    // free any unused memory 
    out = realloc(out, sizeof(char) * sidx);
    if (out == NULL) FATAL_ERROR("failed to allocate memory during string replacement");
    
    free(olens);
    free(nlens);
    return out; 
}


// Join two strings
char *join_strings(const char *foo, const char *bar) {
    size_t foosize = strlen(foo);
    size_t barsize = strlen(bar);
    size_t bufsize = foosize + barsize + 1;
    
    char *str = malloc(sizeof(char)*bufsize);
    if (str == NULL) {
        FATAL_ERROR("failed to allocate memory to join two strings");
    }
    
    memcpy(str, foo, foosize);
    memcpy(str + foosize, bar, barsize);
    str[bufsize-1] = '\0';
    return str;
}

// Strip a string of leading and trailing whitespace
char *trim_string(const char *string) {
    const char *p1 = string;
    const char *p2 = p1 + strlen(string) - 1;
    while (*p1 && isspace(*p1)) p1++;
    while (p2>p1 && isspace(*p2)) p2--;
    return copy_string_part(p1, p2+1);
}

// Make a copy of a string in memory.
char *copy_string(const char *string) {
    size_t bufsize = strlen(string) + 1;
    char *new_string = malloc(sizeof(char) * bufsize);
    
    // If this ever fails, we're done for anyway.
    if (! new_string) {
        FATAL_ERROR("memory allocation failure");
    }
    
    strncpy(new_string, string, bufsize);
    return new_string;
}

// Make a copy of part of a string in memory (and zero-terminate it)
char *copy_string_part(const char *begin, const char *end) {
    size_t length = end - begin;
    char *new_string = malloc(sizeof(char) * (length + 1));
    if (! new_string) {
        FATAL_ERROR("memory allocation failed");
    }
    memcpy(new_string, begin, length);
    new_string[length] = '\0';
    return new_string;
}

// Scan ahead in string until predicate has boolean value
const char *scan_ahead(const char *string, int (*predicate)(int), int value) {
    while (*string && !predicate(*string) != !value) string++;
    return string;
}

// Make a copy of part of a string in memory until predicate has value or end is reached (and zero-terminate it)

char *copy_string_pred(const char *str, int (*predicate)(int), int value) {
    return copy_string_part(str, scan_ahead(str, predicate, value));
}

// See if a string has a prefix
char has_case_insensitive_prefix(const char *string, const char *prefix) {
    return !strncasecmp(string, prefix, strlen(prefix));
}


