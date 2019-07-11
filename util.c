/* asm8085 (C) 2019 Marinus Oosters */

#include "util.h"


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
char *scan_ahead(const char *string, int (*predicate)(int), int value) {
    while (*string && !predicate(*string) != !value) *string++;
    return string;
}

// Make a copy of part of a string in memory until predicate is false or end is reached (and zero-terminate it)

char *copy_string_pred(const char *str, int (*predicate)(int), int value) {
    return copy_string_part(str, scan_ahead(str, predicate, value));
}

// See if a string has a prefix
char has_case_insensitive_prefix(const char *string, const char *prefix) {
    return !strncasecmp(string, prefix, strlen(prefix));
}


