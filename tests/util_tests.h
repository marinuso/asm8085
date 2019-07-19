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

#define TEST_JOIN_STRINGS(a,b) do { \
    s = join_strings(a, b); \
    if (strcmp(s, a b)) FAIL("join_strings returned \"%s\" instead of \"%s\"", s, a b); \
    free(s); s = NULL; \
} while(0)

UTIL_TEST(join_strings, {
    TEST_JOIN_STRINGS("foo", "bar");
    TEST_JOIN_STRINGS("", "bar");
    TEST_JOIN_STRINGS("foo", "");
})

#define TEST_RPLC(str,test) do { \
    s = string_replace(str, rplc, 5); \
    if (strcmp(s, test)) FAIL("string_replace returned \"%s\" instead of \"%s\"", s, test); \
    free(s); s = NULL; \
} while(0)
    
UTIL_TEST(string_replace, {
    struct replacement rplc[5];
    rplc[0].old = "foobar"; rplc[0].new = "bar";
    rplc[1].old = "barbaz"; rplc[1].new = "baz";
    rplc[2].old = "aaaaaa"; rplc[2].new = "AAA";
    rplc[3].old = "dog";    rplc[3].new = "hippopotamus";
    rplc[4].old = "meow";   rplc[4].new = "woof";
    
    // Test: string not containing any of them should be returned without change
    TEST_RPLC("this is a test", "this is a test");
    
    // Test: empty string gives empty string
    TEST_RPLC("", "");
    
    // Test: string is replaced
    TEST_RPLC("foobar","bar");
    TEST_RPLC("barbaz","baz");
    TEST_RPLC("aaaaaa","AAA");
    TEST_RPLC("dog","hippopotamus");
    TEST_RPLC("meow","woof");
    
    // Test: strings are replaced
    TEST_RPLC("foobarbarbaz", "barbaz");
    
    // Test: string constants are left alone
    TEST_RPLC("meow 'meow' meow \"meow\"", "woof 'meow' woof \"meow\"");
    
    
    
})