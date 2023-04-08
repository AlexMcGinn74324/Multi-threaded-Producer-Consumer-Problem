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
//===============buffer vars
struct Queue* p1;
//==============
void put(int value, int buffer[], int fill_ptr, int count, int max);
int get(int use_ptr, int buffer[], int count, int max);
void* distributor(void* fd);
/*
 * Useful functions:
 * gettid() (always successful)
 * Condition Variables and Mutex locks(not semaphores)
 * OSTEP Chapter 30 (locks, cond vars, circular queue)
 * Chapter 26 (Threads)
 */

int main(int argc, char* argv[]){
    int fd[2];  //single pipe
    p1 = createQueue();   //creates queue, assigns front and rear to NULL

    pthread_t distId;

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
    if(prod1 > 0 && prod2 > 0){
        pthread_create(&distId, NULL, distributor, (void*)fd);
    }


    puts("Parent changes p1 after thread execution");
    enQueue(p1,2);
//    printf("Queue: %d\n", p1->front->key);

    puts("Test");


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


    return 0;
}
//thread functions should be of type void* and any arguments passed to it should be type void*
void* distributor(void* fd){
    int done = 0;
    data new = {0, 0, 0, 0};
//    printf("%p %p\n", fd, fd+4);
    if( (close(*(int*)(fd+4))) == -1){  //close write end
        perror("Close in distributor");
        exit(1);
    }
    //continue to read until both sentinel values are sent
    while(done < 2){
        read(*(int*)fd,&new, 16);   //read one record each time
        if(new.pType == -1){
            done++;
        }else if(new.pType == 1){
        }
//        printf("Type: %d, Count: %d, done: %d \n", new.pType, new.pCount, done);
    }
    printf("Child sees change Queue: %d\n", p1->front->key);
    return NULL;
}






//Old buffer implementation
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