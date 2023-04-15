#ifndef MAKEFILE_HELPERS_H
#define MAKEFILE_HELPERS_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct{
    int pType;
    int pCount;
    int tId;
    int cCount;
}data;

struct locks{
    pthread_cond_t* filled;
    pthread_cond_t* empty;
    pthread_mutex_t* mutex;
};

struct consumerBundle{
    struct locks *lock;
    struct Queue *q;
    int cNum;
    int flag;
    pthread_mutex_t* fMutex;
};


//Queue implementation started from G4G.com
struct QNode {
    int pType;  //we need to store product type, product count,
    int pCount; //thread id, and consumer count for each item written to file
    int tId;
    int cCount;
    struct QNode* next;
};

// The queue, front stores the front node of LL and rear
// stores the last node of LL
struct Queue {
    struct QNode *front;
    struct QNode *rear;
    int size, maxSize;
};


//=====================================Functions
void producer(int pType, int fd[2]);
void enQueue(struct Queue* q, int pType, int pCount, struct locks* lock);
struct QNode* deQueue(struct Queue* q);
struct Queue* createQueue();
struct QNode* newNode(int pType, int pCount);
void* consumer(void* cb);

#endif