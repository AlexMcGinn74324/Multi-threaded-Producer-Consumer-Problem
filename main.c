#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "helpers.h"

/*
 * Useful functions:
 * gettid() (always successful)
 */

int main(int argc, char* argv[]){
    data new = {0, 0, 0, 0};
    new.pType = 3;

    printf("%d\n", new.pType);

    return 0;
}
