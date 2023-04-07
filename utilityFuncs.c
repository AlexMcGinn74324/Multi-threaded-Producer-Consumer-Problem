#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "helpers.h"

/* This function will run the producer threads. They create 'products' in a loop over 150 iterations.
 * Each iteration they send 1 product into the pipe and briefly delay (.01-.2s). The data to be sent
 * over is held inside a 'data' struct, which holds the product type, consumption thread id, product
 * count, and consumption count.
 */

void producer(){

}


void consumer(){
//threads arguments must  be cast to void* and back after
}