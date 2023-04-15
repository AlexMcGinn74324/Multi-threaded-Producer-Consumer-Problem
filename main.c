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

/* Before getting into main we've initialized all of our locks and condition variables
 * that will be used to provide mutual exclusion to our buffers (provided via a queue
 * that holds our product type, product count, size, max size, thread id, and consumption
 * count. We fork our producers that create and write these product type/count to a mutual
 * pipe that it shares with the distributor function. We then redirect output to a file,
 * create our threads for each consumption process, which asynchronously takes items from
 * the distributor and writes them to file. The main thread then runs the distributor
 * thread, which again takes items from the pipe and adds them to our buffer (asynchr
 * -onously), before returning to join/reap child threads/processes.  */
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
    if( (dup2(out, STDOUT_FILENO)) == -1){
        perror("dup2 in main");
        exit(1);
    }

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

    return 0;
}
//==============================================================Distributor Thread
//Input: fds for pipe shared with producer processes
//Output: Adds items to buffers (Queues)
/* The distributor is responsible for reading from the
 * pipe that it shares with the producer processes. It
 * then takes that data and adds it to the buffer that
 * the consumer threads then write to file.
 */
void distributor(int* fd){
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

        if(new.pType == 1){
            enQueue(q1, new.pType, new.pCount, &lock1);
        }else if(new.pType == 2){
            enQueue(q2, new.pType, new.pCount, &lock2);
        }
        if(done == 2){
            break;
        }
    }
    return;
}