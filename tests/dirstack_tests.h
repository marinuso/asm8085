/* asm8085 (C) 2019-20 Marinus Oosters */

// This file contains tests for the functions in dirstack.c

#define MAXDIRLEN 4096

TEST(pushd_popd
,   /*startup*/
    char *dnames[11] = {NULL};
    char curdir[MAXDIRLEN];
    int i;
,   /*shutdown*/
    for (i=0; i<11; i++) if(dnames[i]) free(dnames[i]);
,   /*test*/   
{
    // First directory is current directory
    dnames[0] = malloc(MAXDIRLEN * sizeof(char));
    if (getcwd(dnames[0], MAXDIRLEN) == NULL) FAIL("cannot get current directory: %s", strerror(errno));
    
    // Make 10 random directories
    for (i=1; i<11; i++) {
        dnames[i] = malloc(MAXDIRLEN * sizeof(char));
        strcpy(dnames[i], "/tmp/asm8085_test_XXXXXX");
        char *s = mkdtemp(dnames[i]);
        if (s == NULL) {
            FAIL("cannot make directory: %s", strerror(errno));
        }
    }
    
    // Change to each directory in turn
    for (i=1; i<11; i++) {
 
        if (pushd(dnames[i]) == -1) FAIL("cannot change to directory: %s", strerror(errno));
        
        // Did we get there?
        if (getcwd(curdir, MAXDIRLEN) == NULL) FAIL("cannot get directory: %s", strerror(errno));
        if (strcmp(curdir, dnames[i])) FAIL("while pushing %d, directory does not match: '%s' instead of '%s'", i, curdir, dnames[i]);
    }
    
    // See if we can pop them back
    for (i=9; i>=0; i--) {
        if (popd() == -1) FAIL("cannot pop directory: %s", strerror(errno));
        // Did we get there?
        if (getcwd(curdir, MAXDIRLEN) == NULL) FAIL("cannot get directory: %s", strerror(errno));
        if (strcmp(curdir, dnames[i])) FAIL("while popping %d, directory does not match: '%s' instead of '%s'", i, curdir, dnames[i]);
    }
   
})

