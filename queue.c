#include "helpers.h"
#include <pthread.h>

// A utility function to create a new linked list node.
struct QNode* newNode(int pType, int pCount)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));   //create Queue node
    temp->pType = pType;  //assign a pType and pCount
    temp->pCount = pCount;
    temp->tId = 0;
    temp->cCount = 0;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue* createQueue(int max){
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    q->maxSize = max;
    return q;
}
// The function to add a key k to q
void enQueue(struct Queue* q, int pType, int pCount, struct locks* lock){

    pthread_mutex_lock(lock->mutex);
    //=======================================LOCKED

//    printf("Count:%d\n", pCount);
    //if we're at max capacity
    if(q->size == q->maxSize){
        pthread_cond_wait(lock->filled,lock->mutex);    //wait if it's filled
    }


    // Create a new LL node
    struct QNode* temp = newNode(pType, pCount);

    // If queue is empty, then new node is front and rear
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        q->size++;
//        printf("Enqueue Front: %p\n", q->front);
        //signal the consumer that the queue is no longer empty
        if(pthread_cond_signal(lock->empty) != 0){
            perror("cond signal in enqueue");
            exit(1);
        }
//        printf("Thread %zu:%d:%d signaled empty 1\n", pthread_self(), pCount, pType);
        pthread_mutex_unlock(lock->mutex);
        //==================================UNLOCKED
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    q->size++;
    //signal the consumer that the queue is no longer empty

    if(pthread_cond_signal(lock->empty) != 0){
        perror("cond signal in enqueue");
        exit(1);
    }
//        printf("Thread %zu:%d:%d signaled empty 2\n", pthread_self(), pCount, pType);
    //==================================UNLOCKED
    pthread_mutex_unlock(lock->mutex);
}

// Function to remove a key from given queue q
struct QNode* deQueue(struct Queue* q){
//======================================locked already in consumer

//    printf("Front %p\n", q->front);
//    puts("deQueue 1");

    // If queue is empty, return NULL.
    if (q->front == NULL){
//        puts("Queue is empty stupid...");
        return NULL;
    }

//    puts("deQueue 1.5");
    struct QNode* temp = q->front;

//    puts("deQueue 2");
    //if it's not pointing to anything it's the last element
    //clear the queue and return the node
    if(q->front->next == NULL){
        q->front = NULL;
        q->rear = NULL;
        q->size--;
        return temp;
    }
//    puts("deQueue 3");
        q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL){
        q->rear = NULL;
    }
//    puts("deQueue 4");
    //decrement size of queue
    q->size--;
    //Returned null should no longer be pointing to anything
    temp->next = NULL;
    return(temp);
}