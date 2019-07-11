/* asm8085 (C) 2019 Marinus Oosters */
// Test harness

#include <stdio.h>
#include <stdlib.h>

// File containing all the tests 
#define TESTFILE "tests_list.h"

struct test_result {
    char result;
    char *fail_message;
};

/* FAIL and SUCCEED */
#define SUCCEED do { \
    _test_result.result = 1; \
    _test_result.fail_message = NULL; \
    goto _test_final; \
} while(0)

#define FAIL(msg) do { \
    _test_result.result = 0; \
    _test_result.fail_message = msg; \
    goto _test_final; \
} while(0)

/* Construct functions for each test */
#define TEST(name, startup, shutdown, test_code) \
struct test_result test_##name() { \
    struct test_result _test_result; \
    startup; \
    test_code; \
_test_final: \
    shutdown; \
    return _test_result; \
}
#include TESTFILE
#undef TEST

/* See how many tests there are */
const int tests_max = 0
#define TEST(a, b, c, d) + 1
#include TESTFILE
#undef TEST
;

/* Construct a list of all tests */
struct test {
    const char *name;
    struct test_result (*testcode)();
} tests[] = {
#define TEST(name, b, c, d) { #name, test_##name } ,
#include TESTFILE
#undef TEST
};

/* Run a certain test */
char run_test(struct test t) {
    fprintf(stderr, "%-20s: ", t.name);
    struct test_result r = t.testcode();
    if (r.result) {
        fprintf(stderr, "pass.\n");
    } else {
        fprintf(stderr, "fail: %s\n", r.fail_message);
    }
    return r.result;
}

/* Run a test with a certain name */
char run_test_by_name(const char *name) {
    int i;
    
    for (i=0; i<tests_max; i++) {
        if (!strcmp(tests[i].name, name)) {
            return run_test(tests[i]);
        }
    }
    
    fprintf(stderr,"No such test: %s\n", name);
    return 0;
}

/* Run all tests */
char run_tests() {
    int test_num, n_failures = 0;
    fprintf(stderr,"Running all tests...\n");
    for (test_num=0; test_num<tests_max; test_num++) {
        fprintf(stderr, "%3d/%3d: ", test_num+1, tests_max);
        n_failures += !run_test(tests[test_num]);
    }
    
    fprintf(stderr,"---------\nSummary: %d tests passed, %d failed.\n", tests_max-n_failures, n_failures);
    return n_failures == 0;
}


/* If arguments are given, they are test names. If no arguments are given, run all tests. 
 * Return 0 if all tests succeeded, 1 otherwise. */
int main(int argc, char **argv) {
    if (argc == 1) {
        return !run_tests();
    }
    
    int argn;
    char ok = 1;
    for (argn = 1; argn < argc; argn++) {
        fprintf(stderr, "%3d/%3d: ", argn, argc-1);
        ok = ok && run_test_by_name(argv[argn]);
    }
    
    return !ok;
    
}

