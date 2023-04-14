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
            cur.pCount = -1;
//            printf("Product Type: %d, Product Count: %d\n", cur.pType, cur.pCount);
//            write(fd[1], &cur, size); //unnecessary
//            close(fd[1]);
//            exit(1);
        }
        if( (write(fd[1], &cur, size)) == -1){
            perror("write in utility");
            exit(1);
        }
    }

    close(fd[1]);
    //send -1 count to pipe

    exit(1);
}
void* consumer(void* cbTemp){
    struct QNode *node = newNode(0, 0);
    struct consumerBundle *cb = (struct consumerBundle *) cbTemp;
//    unsigned int timer = 10000;//10,000 microseconds is .01s, the lower bound of our restriction. Sleeps for at least that

    while(cb->flag != 1) {
//    printf("%d\n", cb->flag);
//        if( (usleep(timer)) == -1) { //sleep
//            perror("usleep in producer function");
//            exit(1);
//        }
//        printf("Awoken: %zu\n", pthread_self());




    if(cb->q->size == 0){
        printf("Queue is empty\n");
    }
    printf("Size: %d\n", cb->q->size);




        //===============================================LOCKED
        if (pthread_mutex_lock((cb->lock->mutex)) != 0) {
            perror("consumer lock 1");
            exit(1);
        }

        if (cb->q->size <= 0  && cb->flag != 1) {
//            printf("Waiting in consumer: %p\n", cb->lock->empty);
//            printf("Waiting! cb->flag: %d, TID: %zu\n", cb->flag, pthread_self());
            if ((pthread_cond_wait((cb->lock->empty), (cb->lock->mutex))) != 0) {
                perror("Cond wait in consumer 1");
                exit(1);
            }
//            printf("Returning! cb->flag: %d, TID: %zu\n", cb->flag, pthread_self());
//            printf("Returning in consumer: lock %p, Flag %d\n", cb->lock->empty, cb->flag);
//            printf("Consumer size now > 0: %d\n", cb->q->size);
        }

//        puts("Consumer run 2");
//    if(cb != NULL){
//        printf("%d\n", cb->q->front->pType);
//    }
//        printf("Size before dequeue in Consumer: %d\n", cb->q->size);

        //queue size decremented in deQueue
        if(cb->flag != 1)
            node = deQueue(cb->q);

        if(node->pCount != -1){
            printf("%34s %3s %3s %16s\n", "Type", "Count", "Size", "Consumption Count");
            printf("Test Node Returned in Consumer: %1d %5d %3d %8d\n", node->pType, node->pCount, cb->q->size, cb->cNum);
        }else{  //we have received kill signal for this product
            cb->flag = 1;
//            printf("cb->flag: %d, TID: %zu\n", cb->flag, pthread_self());
        }

        //trying to signal until eventually blocked
//    int i = 0;
//    while(pthread_cond_signal((cb->lock->filled)) == 0 && i < 1000000){
//        printf("Consumer Signaling lock at &: %p, iteration: %d", &(cb->lock->filled), i);
//        i++;
//    }
        if(cb->flag != 1)
            cb->cNum++;




//    printf("Lock filled flag signaled: %p\n", &(cb->lock->filled));
        if ((pthread_cond_signal(cb->lock->filled)) != 0) {
            perror("cond signal in consumer");
            exit(1);
        }
        pthread_mutex_unlock((cb->lock->mutex));
        //=========================================UNLOCKED
    }
//    printf("%p\n", cb->lock->empty);
//    while(pthread_cond_signal(cb->lock->empty) != 0){
//        perror("cond signal in enqueue");
//        exit(1);
//    }
    printf("%zu returned successfully\n", pthread_self());
    pthread_cond_signal(cb->lock->empty);
//    printf("%d\n", node->pCount);
    return NULL;
}
