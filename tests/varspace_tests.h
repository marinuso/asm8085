/* asm8085 (C) 2019 Marinus Oosters */

// This file contains tests for the functions in varspace.c

#define VS_INIT struct varspace *vs = alloc_varspace();
#define VS_FREE free_varspace(vs);
#define VS_TEST(name, code) TEST(vs_##name, VS_INIT, VS_FREE, code)

// Test if a variable can be set and then retrieved
#define TEST_VAR(VAR,VAL) \
    if (! get_var(vs, #VAR, &val)) FAIL("Variable '" #VAR "' not found."); \
    if (val != VAL) FAIL("Variable '" #VAR "' does not have the right value.");    
VS_TEST(set_get_var, {
    // store a couple of variables
    set_var(vs, "foo", 42);
    set_var(vs, "bar", 43);
    set_var(vs, "baz", 44);
    
    // see if we can retrieve them
    intptr_t val;

    TEST_VAR(foo, 42);
    TEST_VAR(bar, 43);
    TEST_VAR(baz, 44);

    SUCCEED;
})
#undef TEST_VAR
    

// Test if a variable's value can be changed
#define TEST_SET(VAL) \
    set_var(vs, "foo", VAL); \
    if (!get_var(vs, "foo", &val) || val!=VAL) FAIL("Variable not set to " #VAL);
VS_TEST(redefine_var, {
    intptr_t val;
    TEST_SET(42);
    TEST_SET(43);
    TEST_SET(44);
    SUCCEED;
})
#undef TEST_SET

// Test if a variable can be removed
VS_TEST(del_var, {
    intptr_t val;
    set_var(vs, "foo", 42);
    if (!get_var(vs, "foo", &val) || val!=42) FAIL("Variable not set.");
    if (!del_var(vs, "foo")) FAIL("del_var couldn't find the variable.");
    if (get_var(vs, "foo", &val)) FAIL("Variable not actually deleted.");
    SUCCEED;
})

// Test if basenames work
#define TEST_GET(VAR,VAL,MSG) if (!get_var(vs, VAR, &val) || val!=VAL) FAIL(MSG);
#define TEST_SET(VAR,VAL,MSG) set_var(vs, VAR, VAL); TEST_GET(VAR,VAL,MSG)
VS_TEST(basenames, {
    intptr_t val;
    
    // Set the base
    set_base(vs, "foo");
    
    // set a variable without a dot, this should mean it's still top-level
    TEST_SET("foo", 42, "Cannot set toplevel variable.");
    
    // set a variable with a dot, we should be able to retrieve it
    TEST_SET(".foo", 43, "Cannot set .foo from within foo.");
    
    // test that it doesn't overwrite the toplevel var with the same name
    TEST_GET("foo", 42, "Toplevel 'foo' is no longer there");
    
    // test that we can set a variable with another basename
    TEST_SET("bar.bar", 44, "Cannot set bar.bar from within foo.");
     
    // Switch the base
    set_base(vs, "bar");
     
    // test that we can still get the toplevel variable
    TEST_GET("foo", 42, "Toplevel variable no longer there after switching base");
    
    // test that we can now get the "bar.bar" we just set
    TEST_GET(".bar", 44, "Cannot get .bar from within bar");
    
    // test that we can't see foo.foo 
    if (get_var(vs, ".foo", &val)) FAIL("bar.foo exists");
    
    // make bar.foo
    TEST_SET(".foo", 45, "Cannot set bar.foo");
    
    // test that we can get foo.foo if fully specified and that it hasn't changed
    TEST_GET("foo.foo", 43, "foo.foo is no longer there");
    
    SUCCEED;    
})
#undef TEST_GET
#undef TEST_SET
