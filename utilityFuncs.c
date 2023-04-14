#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "helpers.h"

/* This function will run the producer processes. They create 'products' in a loop over 150 iterations.
 * Each iteration they send 1 product into the pipe and briefly delay (.01-.2s). The data to be sent
 * over is held inside a 'data' struct, which holds the product type, consumption thread id, product
 * count, and consumption count. */
//input: Product type #
//output: n/a
//====================================================================================================
void producer(int pType, int fd[2]){
    int pCount = 0; //initialize this producer's count to 0
    unsigned int timer = 10000;//10,000 microseconds is .01s, the lower bound of our restriction. Sleeps for at least that
    size_t size = 16;
    close(fd[0]);   //close read end

//    printf("Fd1: %d, Fd2: %d\n", fd[0], fd[1]);
    //loop to create product
    for(int i = 0; i <= 150; i++){
        pCount++;
        data cur = {pType,pCount, 0, 0}; //initialize each element, consumer count/thread id to be updated later
        if( (usleep(timer)) == -1) { //sleep
            perror("usleep in producer function");
            exit(1);
        }

        if(i == 150){   //the 151st iteration
            cur.pType = -1;
            write(fd[1], &cur, size);
            close(fd[1]);
            exit(1);
//            printf("Product Type: %d, Product Count: %d\n", cur.pType, cur.pCount);
        }
        write(fd[1], &cur, size);
    }

    close(fd[1]);
    //send -1 count to pipe

    exit(1);
}
void* consumer(void* cbTemp){
    struct QNode* node = newNode(0,0);
    struct consumerBundle* cb = (struct consumerBundle*)cbTemp;     ///is this stupid?

    if(cb->q->size == 0){
        printf("Queue is empty\n");
    }
    printf("Max size: %d\n", cb->q->maxSize);
//    printf("Size: %d\n", cb->q->size);
    puts("INITIAL TEST IN CONSUMER");
    if(pthread_mutex_lock(&(cb->lock->mutex)) != 0){
        perror("consumer lock 1");
        exit(1);
    }

    puts("consumer run 1");

    if(cb->q->size <= 0){
        puts("Test Consumer");
        pthread_cond_wait(&(cb->lock->empty),&(cb->lock->mutex));   //wait if it's empty
    }
    puts("Consumer run 2");
    if(cb != NULL){
        printf("%d\n", cb->q->front->pType);
    }
    node = deQueue(cb->q,*(cb->lock));
    printf("Test Dequeue: %d\n",node->pType);
//    cb->q->size--;  //decrement the queue
    pthread_cond_signal(&(cb->lock->filled));
    pthread_mutex_unlock(&(cb->lock->mutex));

    return NULL;
}
