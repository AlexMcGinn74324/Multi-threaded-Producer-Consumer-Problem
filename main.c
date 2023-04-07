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
    int fd[2];  //single pipe
    data new = {0, 0, 0, 0};

    pipe(fd);   //pipe for communicating between producers/distributor
    int prod1 = 0, prod2 = 0;

    if( (prod1 = fork()) == -1){
        perror("fork #1 in main");
        exit(1);
    }else if(prod1 == 0){
        producer(1, fd);    //type 1
    }
    if( (prod1 > 0) && (prod2 = fork()) == -1){
        perror("fork #1 in main");
        exit(1);
    }else if(prod2 == 0){
        producer(2, fd);    //type 1
    }

    printf("pid: %d\n", getpid());



    //main will be the consumer process
    //we will fork and run the producer processes from here



    //distributor thread reads from pipe
    //dist thread then writes to buffer 1/2 depending on pType
    //buffers should be different sizes**



//    parent waits for child processes
        if( (waitpid(prod1, NULL, 0)) == -1){
            perror("waitpid prod1 in main");
            exit(1);
        }
        if( (waitpid(prod2, NULL, 0)) == -1){
            perror("waitpid prod2 in main");
            exit(1);
        }
    printf("Nullifying Errors: %d %d %d\n", new.pType, prod1, prod2); //nullify error

    return 0;
}
