/* asm8085 (C) 2019 Marinus Oosters */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

#define FALSE 0
#define TRUE (!FALSE)

#define FATAL_ERROR(msg, ...) \
    do { \
        fprintf(stderr, "%s (%d): %s: fatal error: " msg "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        exit(255); \
    } while(0)


// For string replacement
struct replacement {
    char *old;
    char *new;
};

// Replace substrings in string, except within "..." or '...'
char *string_replace(const char *str, const struct replacement *rpls, int n_replacements);

// Join two strings
char *join_strings(const char *, const char *);

// Strip a string of leading and trailing whitespace
char *trim_string(const char *);

// Make a copy of a string (allocate new memory for it).
char *copy_string(const char *);

// Scan ahead in string until predicate has given boolean value or end of string is reached
// (the predicate type is to match that of isspace and the like)
const char *scan_ahead(const char *, int (*)(int), int);

// See if a string has a prefix
char has_case_insensitive_prefix(const char *string, const char *prefix);

// Make a copy of part of a string in memory (and zero-terminate it)
char *copy_string_part(const char *begin, const char *end);

// Make a copy of part of a string in memory until predicate has given boolean value or end is reached (and zero-terminate it)
char *copy_string_pred(const char *str, int (*)(int), int);

// Rotate left
unsigned char rol(unsigned char byte, char n);

#endif