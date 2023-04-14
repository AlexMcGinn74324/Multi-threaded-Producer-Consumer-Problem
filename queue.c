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
//    if(pType == 1){
//        printf("Lock Address in enQueue: %p\n", &(lock->mutex));
//        printf("Empty Address in enQueue: %p\n", &(lock->empty));
//        printf("Filled Address in enQueue: %p\n", &(lock->filled));
//    }

    printf("Size of Queue: %d\n", q->size);

//    printf("in Enqueue %p\n", lock->empty);
//    puts("Test enQueue Bro");
    //lock the buffer
    pthread_mutex_lock(lock->mutex);
    //=======================================LOCKED
    //if we're at max capacity
//    printf("pType: %d, pCount: %d\n", pType, pCount);
    if(q->size >= q->maxSize){
//        puts("waiting in enqueue");
//        printf("Empty Address in Enqueue: %p\n", &(lock->empty));
//        printf("Mutex Address in Enqueue: %p\n\n", &(lock->mutex));
//        printf("Filled Address in Enqueue: %p\n", &(lock->filled));
        pthread_cond_wait(lock->filled,lock->mutex);    //wait if it's filled
    }


    // Create a new LL node
    struct QNode* temp = newNode(pType, pCount);

    // If queue is empty, then new node is front and rear
    if (q->rear == NULL) {
//        printf("Rear is null at %d\n", pCount);
        q->front = q->rear = temp;
        q->size++;
        //signal the consumer that the queue is no longer empty
        if(pthread_cond_signal(lock->empty) != 0){
            perror("cond signal in enqueue");
            exit(1);
        }
//        puts("Signaled empty");
        pthread_mutex_unlock(lock->mutex);
        //==================================UNLOCKED
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    q->size++;
//    printf("Size after Enqueue Type %d: %d\n",q->rear->pType, q->size);
    //signal the consumer that the queue is no longer empty

    if(pthread_cond_signal(lock->empty) != 0){
        perror("cond signal in enqueue");
        exit(1);
    }
//    puts("Signaled empty");
    //==================================UNLOCKED
    pthread_mutex_unlock(lock->mutex);
}

// Function to remove a key from given queue q
//======================================locked already in consumer
struct QNode* deQueue(struct Queue* q){


    // If queue is empty, return NULL.
    if (q->front == NULL){
//        puts("Queue is empty stupid...");
        return NULL;
    }


    struct QNode* temp = q->front;

    //if it's not pointing to anything it's the last element
    //clear the queue and return the node
    if(q->front->next == NULL){
//        puts("You're taking the last one! THIEF!");
//        printf("Head: %d %d\n", temp->pType, temp->pCount);
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
//    printf("Qsize deQueue: Before %d", q->size);
    q->size--;
//    printf("Qsize deQueue: After %d", q->size);
    //Returned null should no longer be pointing to anything
    temp->next = NULL;
    return(temp);
}