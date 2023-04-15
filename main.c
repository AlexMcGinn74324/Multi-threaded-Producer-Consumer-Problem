#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include "helpers.h"
//=============================Constants
#define MAX1 60
#define MAX2 75
//===============buffers
struct Queue* q1;
struct Queue* q2;
//==============Globals
pthread_cond_t filled1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t filled2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
struct locks lock1;
struct locks lock2;
struct consumerBundle* cb1;
struct consumerBundle* cb2;
//==============
void put(int value, int buffer[], int fill_ptr, int count, int max);
int get(int use_ptr, int buffer[], int count, int max);
void distributor(int* fd);

/*
 * Useful functions:
 * gettid() (always successful)
 * Condition Variables and Mutex locks(not semaphores)
 * OSTEP Chapter 30 (locks, cond vars, circular queue)
 * Chapter 26 (Threads)
 *
 * Left Off: Created both buffers and added all produced
 * of product1/2, retrieved from shared pipe. Queue has
 * ways to add and remove from queue (although to retrieve
 * values you will need to access each element to save and
 * then dequeue).
 * Should each queue have a shared consumption value as
 * well as the stored value for each node for writing to
 * file?
 *
 * Need to free nodes after dequeue (used to be done in dequeue function)
 */

int main(int argc, char* argv[]){
    int fd[2];  //single pipe
    q1 = createQueue(MAX1);   //creates queue, assigns front and rear to NULL
    q2 = createQueue(MAX2);
    pthread_t c1, c3;
    pthread_t c2, c4;
    //=====================lock structs set
    lock1.filled = &filled1;
    lock1.empty = &empty1;
    lock1.mutex = &mutex1;
    lock2.filled = &filled2;
    lock2.empty = &empty2;
    lock2.mutex = &mutex2;




//    puts("Begin Program:");
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
        producer(2, fd);    //PRODUCERS EXIT IN FUNCTION
    }

    cb1 = (struct consumerBundle*)malloc(sizeof(struct consumerBundle));
    cb1->q = q1;
    cb1->lock = &lock1;
    cb1->flag = 0;  //we haven't received kill signal
    cb1->cNum = 1;

    cb2 = (struct consumerBundle*)malloc(sizeof(struct consumerBundle));
    cb2->q = q2;
    cb2->lock = &lock2;
    cb2->flag = 0;  //we haven't received kill signal
    cb2->cNum = 1;

        //UNDO AFTER TEST***!!!!!!!!!!!!! TO OUTPUT TO FILE
//    int out = open("out.txt", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
//    printf("%d\n", STDOUT_FILENO);
//    int save = dup(STDOUT_FILENO);
//    if( (dup2(out, STDOUT_FILENO)) == -1){
//        perror("dup2 in main");
//        exit(1);
//    }

//    if((c1 == c3) && (c2 == c4));
    if(c3 == c4);
    //create consumer threads
    pthread_create(&c1, NULL, consumer, ((void*)cb1));
    pthread_create(&c2, NULL, consumer, ((void*)cb1));
    pthread_create(&c3, NULL, consumer, ((void*)cb2));
    pthread_create(&c4, NULL, consumer, ((void*)cb2));

    distributor(fd);
    puts("Distributor finished");
    printf("thread 1: %d\n", pthread_join(c1, NULL));
    printf("thread 2: %d\n", pthread_join(c2, NULL));
    printf("thread 3: %d\n", pthread_join(c3, NULL));
    printf("thread 4: %d\n", pthread_join(c4, NULL));


    //parent reaps child processes
    if( (waitpid(prod1, NULL, 0)) == -1){
        perror("waitpid prod1 in main");
        exit(1);
    }
    if( (waitpid(prod2, NULL, 0)) == -1){
        perror("waitpid prod2 in main");
        exit(1);
    }

//    dup2(save, STDOUT_FILENO);
//    printf("%d\n", save);
    return 0;
}
//==============================================================Distributor Thread
//thread functions should be of type void* and any arguments
// passed to it should be type void*
void distributor(int* fd){
    printf("Parent TID: %zu\n", pthread_self());
    int done = 0;
    data new = {0, 0, 0, 0};
    if( (close(fd[1])) == -1){  //close write end
        perror("Close in distributor");
        exit(1);
    }
    //continue to read until both sentinel values are sent
    while(done < 2){

        //read one record each time
        if( (read(fd[0],&new, 16)) == -1){
            perror("read in distributor");
            exit(1);
        }

        if(new.pCount == -1){   //if a thread is finished let the producer know
            done++;
        }
//        printf("New Count: %d, Type: %d\n", new.pCount, new.pType);
        if(new.pType == 1){
            enQueue(q1, new.pType, new.pCount, &lock1);
//            printf("Queue 1: %d %d\n", q1->size, cb1->q->size);
//            if(q1->rear != NULL && q2->rear != NULL){
//                printf("Distributor Queued:Type %d, Count: %d\n", q1->rear->pType, q1->rear->pCount);
//            }
        }else if(new.pType == 2){
            enQueue(q2, new.pType, new.pCount, &lock2);
//            printf("Queue 2 before: %d %d\n", q1->size, cb2->q->size);  //NOT LOCKED HERE
//            printf("Queue 2 after: %d %d\n", q1->size, cb2->q->size);
//            if(q1->rear != NULL && q2->rear != NULL){
//                printf("Distributor Queued:Type %d, Count: %d\n", q2->rear->pType, q2->rear->pCount);
//            }
        }
    }
    return;
}






//=======================================================================Old buffer implementation (Ignore)
//From OSTEP Chapter 30
//create global variables for buffer1/2, fillptr1/2, max 1/2 and use them to pass in values for each buffer
//void put(int value, int buffer[], int fill_ptr, int count, int max) {
//    buffer[fill_ptr] = value;
//    fill_ptr = (fill_ptr + 1) % max;
//    count++;
//}
//int get(int use_ptr, int buffer[], int count, int max) {
//    int tmp = buffer[use_ptr];
//    use_ptr = (use_ptr + 1) % max;
//    count--;
//    return tmp;
//}

//    printf("Nullifying Errors: %d %d %d %d %d %d %d %d %d %d\n", prod1, prod2, count1, count2, use_ptr1, use_ptr2,
//           fill_ptr1, fill_ptr2, buffer1[0], buffer2[0]); //nullify error