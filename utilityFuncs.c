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
    size_t size = 16;
    close(fd[0]);   //close read end

    //loop to create product
    for(int i = 0; i <= 150; i++){
        pCount++;
        data cur = {pType,pCount, 0, 0}; //initialize each element, consumer count/thread id to be updated later

        unsigned int timer = 10000;//10,000 microseconds is .01s, the lower bound of our restriction. Sleeps for at least that
        if( (usleep(timer)) == -1) { //sleep
            perror("usleep in producer function");
            exit(1);
        }

        if(i == 150){   //the 151st iteration
            cur.pCount = -1;
//            printf("Type: %d, pCount: %d\n", pType, cur.pCount);
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

//==========================================================CONSUMER===================================================
void* consumer(void* cbTemp){
    struct consumerBundle *cb = (struct consumerBundle *) cbTemp;   //transferring address isn't a race condition
    struct QNode *node = newNode(0, 0);


    while(cb->flag != 1) {  //===================Loop until flag is thrown

        if (pthread_mutex_lock((cb->lock->mutex)) != 0) {   //===============LOCKED
            perror("consumer lock 1");
            exit(1);
        }
        if(cb->flag == 1){
            printf("Flag received 1 by %zu\n", pthread_self());
            return NULL;
        }
//        printf("Thread %zu locked:\n", pthread_self());

//        unsigned int timer = 10000;       //Test code
//        if( (usleep(timer)) == -1) {
//            perror("usleep in producer function");
//            exit(1);
//        }

        while(cb->q->size == 0 && cb->flag != 1) {
//            printf("%zu waiting\n", pthread_self());
            if ((pthread_cond_wait((cb->lock->empty), (cb->lock->mutex))) != 0) {
                perror("Cond wait in consumer 1");
                exit(1);
            }
            if(cb->flag == 1){
//                printf("Flag received 2 by %zu\n", pthread_self());
                return NULL;
            }

//            printf("Thread: %zu, size: %d\n", pthread_self(), cb->q->size);
        }



        //queue size decremented in deQueue
//        printf("Thread %zu entering dequeue:\n", pthread_self());
        if(cb->flag != 1)
            node = deQueue(cb->q);
//        printf("Thread %zu awoken: pCount %d\n", pthread_self(), node->pCount);

        if(node->pCount != -1){
                pthread_mutex_lock(cb->fMutex);
                printf("%34s %3s %3s %16s %13s\n", "Type", "P.Count", "Size", "Consumption Count", "C. Thread ID");
                printf("Test Node Returned in Consumer: %1d %6d %3d %14d %25zu\n", node->pType, node->pCount, cb->q->size, cb->cNum,pthread_self());
                pthread_mutex_unlock(cb->fMutex);
        }else{  //we have received kill signal for this product
            cb->flag = 1;
//            printf("cb->flag: %d, TID: %zu\n", cb->flag, pthread_self());
        }

        if(cb->flag != 1)
            cb->cNum++;



//        printf("Thread %zu finished dequeue, unlocking:\n", pthread_self());
//    printf("Lock filled flag signaled: %p\n", &(cb->lock->filled));
        if ((pthread_cond_signal(cb->lock->filled)) != 0) {
            perror("cond signal in consumer");
            exit(1);
        }
        pthread_mutex_unlock((cb->lock->mutex));
        //=========================================UNLOCKED, loop again
    }
    //if the first thread for this buffer has terminated successfully, we signal the other one to stop waiting
//    printf("%zu returned successfully with flag %d\n", pthread_self(), cb->flag);
    if( (pthread_cond_signal(cb->lock->empty)) != 0){
        perror("Final signal in consumer");
        exit(1);
    }
//    printf("%d\n", node->pCount);
    return NULL;
}
