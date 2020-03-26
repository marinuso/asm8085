/* asm8085 (C) 2019-20 Marinus Oosters */

#ifndef __DIRSTACK_H__
#define __DIRSTACK_H__

#include "util.h"
#include <unistd.h>

// Push current directory and change working directory to given directory
int pushd(const char *);

// Pop directory and change to it
int popd();

#endif