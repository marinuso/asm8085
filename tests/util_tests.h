/* asm8085 (C) 2019 Marinus Oosters */

// This file contains tests for the functions in util.c

#include "../util.h"

#define UTIL_INIT char *s=NULL;
#define UTIL_FREE if (s!=NULL) free(s);
#define UTIL_TEST(name, code) TEST(util_##name, UTIL_INIT, UTIL_FREE, code)


UTIL_TEST(copy_string, {
    char *test = "foobar";
    s = copy_string(test);
    if (strcmp(s, test)) FAIL("copied string is not the same");
    SUCCEED;
})

UTIL_TEST(prefix, {
    char *ta = "foobar";
    char *tb = "FOO";
    char *tc = "bar";
    
    if (! has_case_insensitive_prefix(ta, tb)) FAIL("prefix not found, but it's there");
    if (has_case_insensitive_prefix(ta, tc)) FAIL("prefix found, but it's not there");
    SUCCEED;
})

UTIL_TEST(copy_string_part, {
    char *test = "hello world";
    s = copy_string_part(test, test+5);
    if (strcmp(s, "hello")) FAIL("copied string is not the same");
    SUCCEED;
})

UTIL_TEST(scan_ahead, {
    const char *test = "      hello";
    const char *test2 = scan_ahead(test, isspace, FALSE);
    if (strcmp(test2, "hello")) FAIL("failed to skip whitespace");
    const char *test3 = "hello     ";
    const char *test4 = scan_ahead(test3, isspace, FALSE);
    if (strcmp(test3, test4)) FAIL("skipped non-whitespace (%p -> %p)", test3, test4);
    SUCCEED;
})

UTIL_TEST(copy_string_pred, {
    char *test = "hello     world";
    s = copy_string_pred(test, isspace, TRUE);
    if (strcmp(s, "hello")) FAIL("copy_string_pred result was: %s", s);
    SUCCEED;
})

UTIL_TEST(trim_string, {
    const char *test = "   hello world    ";
    char *s = trim_string(test);
    if (strcmp(s, "hello world")) FAIL("trim_string returned \"%s\"", s);
    SUCCEED;
})


