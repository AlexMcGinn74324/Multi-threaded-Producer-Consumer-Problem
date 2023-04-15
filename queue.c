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
/* Enqueue is a thread-safe way to add nodes to our queue. It
 * uses mutex and conditional statements to do so. The lock->
 * empty condition variable lets the consumer threads know that
 * they have something to consume, and it also waits on the consumer
 * processes if the queue is full.
 */
//Input: Buffer Queue, Product type/count of item to be added,
//locks for queue.

//Output: Adds item to queue
void enQueue(struct Queue* q, int pType, int pCount, struct locks* lock){

    pthread_mutex_lock(lock->mutex);
    //=======================================LOCKED

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

        //signal the consumer that the queue is no longer empty
        if(pthread_cond_signal(lock->empty) != 0){
            perror("cond signal in enqueue");
            exit(1);
        }
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
    //==================================UNLOCKED
    pthread_mutex_unlock(lock->mutex);
}

/* DeQueue removes an item from the queue from a
 * consumer thread which is already locked and decre
 * -ments the size of the queue.

 * Input: Buffer Queue
 * Output: Returned node */
struct QNode* deQueue(struct Queue* q){
//======================================locked already in consumer

    // If queue is empty, return NULL.
    if (q->front == NULL){
        return NULL;
    }


    struct QNode* temp = q->front;

    //if it's not pointing to anything it's the last element
    //clear the queue and return the node
    if(q->front->next == NULL){
        q->front = NULL;
        q->rear = NULL;
        q->size--;
        return temp;
    }

        q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL){
        q->rear = NULL;
    }

    //decrement size of queue
    q->size--;
    //Returned null should no longer be pointing to anything
    temp->next = NULL;
    return(temp);
}