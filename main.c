#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include "helpers.h"
//=============================Constants
#define MAX1 60
#define MAX2 75
//===============buffers
struct Queue* p1;
struct Queue* p2;
//==============Globals
pthread_cond_t filled1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t filled2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
struct locks lock1;
struct locks lock2;
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
    p1 = createQueue(MAX1);   //creates queue, assigns front and rear to NULL
    p2 = createQueue(MAX2);
    struct QNode* node = newNode(0,0);

    lock1.filled = filled1;
    lock1.empty = empty1;
    lock1.mutex = mutex1;

    lock2.filled = filled2;
    lock2.empty = empty2;
    lock2.mutex = mutex2;

    puts("Begin Program:");
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

//    printf("pid: %d\n", getpid());
    //parent runs
    if(prod1 > 0 && prod2 > 0){
//        pthread_create(&distId, NULL, distributor, (void*)fd);
        distributor(fd);
    }


//    puts("Parent changes p1 after thread execution");
//    enQueue(p1,2);




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
//    deQueue(p1);
//    deQueue(p1);
//    deQueue(p1);

    //iterates through queues
    while( (node = deQueue(p1)) != NULL){
//        printf("Returned Node: pType: %d, pCount: %d\n", node->pType, node->pCount);
    }
    while( (node = deQueue(p2)) != NULL){
//        printf("Returned Node: pType: %d, pCount: %d\n", node->pType, node->pCount);
    }

    if(p1->front != NULL){
        printf("Queue 1 pType: %d, pCount: %d, size: %d\n", p1->front->pType, p1->front->pCount, p1->size);
    }
    if(p2->front != NULL){
        printf("Queue 2 pType: %d, pCount: %d, size: %d\n", p2->front->pType, p2->front->pCount, p2->size);
    }

    return 0;
}
//==============================================================Distributor Thread
//thread functions should be of type void* and any arguments
// passed to it should be type void*
void distributor(int* fd){
    int done = 0;
    data new = {0, 0, 0, 0};
//    printf("%p %p\n", fd, fd+4);
//    printf("%d", newFd[0]);
    if( (close(fd[1])) == -1){  //close write end
        perror("Close in distributor");
        exit(1);
    }
    //continue to read until both sentinel values are sent
    while(done < 2){
        read(fd[0],&new, 16);   //read one record each time
        if(new.pType == -1){
            done++;
        }else if(new.pType == 1){
            printf("Type: %d(1), Count: %d\n", new.pType, new.pCount);
            enQueue(p1, new.pType, new.pCount, lock1);
        }else if(new.pType == 2){
            printf("Type: %d(2), Count: %d\n", new.pType, new.pCount);
            enQueue(p2, new.pType, new.pCount, lock2);
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