#include "dirstack.h"

struct dirstack_item {
    char *name;
    struct dirstack_item *prev;
};

// Keep top of stack
static struct dirstack_item *stack_top = NULL;

void freeitem(struct dirstack_item *itm) {
    free(itm->name);
    free(itm);
}

int popd() {
    // stack is empty?
    if (stack_top == NULL) return -1;
  
    // try to change directory
    int rv = chdir(stack_top->name);
    if (rv == 0) {
        // Success: pop item off stack
        struct dirstack_item *itm = stack_top;
        stack_top = itm->prev;
        freeitem(itm);
    }
    
    return rv;
}


int pushd(const char *dir) {
    struct dirstack_item *itm = malloc(sizeof(struct dirstack_item));
    if (itm == NULL) {
        FATAL_ERROR("failed to allocate memory for directory stack");
    }
    
    // Get current directory name
    itm->name = malloc(PATH_MAX);
    if (itm->name == NULL) {
        FATAL_ERROR("failed to allocate memory for path name");
    }
    
    if (getcwd(itm->name, PATH_MAX) == NULL) {
        free(itm->name);
        free(itm);
        return -1;
    }
    
    
    // Try to change to given directory
    int rv = chdir(dir);
    
    if (rv == 0) {
        // Success: push previous directory onto stack
        itm->prev = stack_top;
        stack_top = itm;
    } else {
        // Failure: free item
        freeitem(itm);
    }
    
    return rv;
}

        
    
    