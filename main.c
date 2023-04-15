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
pthread_mutex_t fMutex = PTHREAD_MUTEX_INITIALIZER;
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
    cb1->fMutex = &fMutex;  //lock for file access

    cb2 = (struct consumerBundle*)malloc(sizeof(struct consumerBundle));
    cb2->q = q2;
    cb2->lock = &lock2;
    cb2->flag = 0;  //we haven't received kill signal
    cb2->cNum = 1;
    cb2->fMutex = &fMutex;

    int out = open("out.txt", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
//    int save = dup(STDOUT_FILENO);
    if( (dup2(out, STDOUT_FILENO)) == -1){
        perror("dup2 in main");
        exit(1);
    }

//    if((c1 == c3) && (c2 == c4));
//    if(c3 == c4);
    //create consumer threads
    pthread_create(&c1, NULL, consumer, ((void*)cb1));
    pthread_create(&c2, NULL, consumer, ((void*)cb1));
    pthread_create(&c3, NULL, consumer, ((void*)cb2));
    pthread_create(&c4, NULL, consumer, ((void*)cb2));

    distributor(fd);
    puts("Distributor finished");
    if( (pthread_join(c1, NULL) == 0)){
        printf("Thread 1 join successful\n");
    }
    if( (pthread_join(c2, NULL) == 0)){
        printf("Thread 2 join successful\n");
    }
    if( (pthread_join(c3, NULL) == 0)){
        printf("Thread 3 join successful\n");
    }
    if( (pthread_join(c4, NULL) == 0)){
        printf("Thread 4 join successful\n");
    }



    //parent reaps child processes
    if( (waitpid(prod1, NULL, 0)) == -1){
        perror("waitpid prod1 in main");
        exit(1);
    }
    if( (waitpid(prod2, NULL, 0)) == -1){
        perror("waitpid prod2 in main");
        exit(1);
    }

//    printf("%zu\n", pthread_self());

//    dup2(save, STDOUT_FILENO);
//    printf("%d\n", save);
    return 0;
}
//==============================================================Distributor Thread
//thread functions should be of type void* and any arguments
// passed to it should be type void*
void distributor(int* fd){
//    printf("Parent TID: %zu\n", pthread_self());
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
//            printf("Type: %d, Count: %d\n", new.pType, new.pCount);
            enQueue(q1, new.pType, new.pCount, &lock1);

//            if(q1->rear != NULL && q2->rear != NULL){
//                printf("Distributor Queued:Type %d, Count: %d\n", q1->rear->pType, q1->rear->pCount);
//            }
        }else if(new.pType == 2){
//            printf("Type: %d, Count: %d\n", new.pType, new.pCount);
            enQueue(q2, new.pType, new.pCount, &lock2);
//            if(q1->rear != NULL && q2->rear != NULL){
//                printf("Distributor Queued:Type %d, Count: %d\n", q2->rear->pType, q2->rear->pCount);
//            }
        }
        if(done == 2){
            break;
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