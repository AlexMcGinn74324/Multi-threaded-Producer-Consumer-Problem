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
void enQueue(struct Queue* q, int pType, int pCount, struct locks lock){

    //lock the buffer
    pthread_mutex_lock(&lock.mutex);
    //if we're at max capacity
    if(q->size >= q->maxSize){
        puts("waiting in enqueue");
        pthread_cond_wait(&lock.filled,&lock.mutex);    //wait if it's filled
    }
//    printf("Test enQueue: Type: %d, Count: %d\n", pType, pCount);


    // Create a new LL node
    struct QNode* temp = newNode(pType, pCount);

    // If queue is empty, then new node is front and rear
    // both
    if (q->rear == NULL) {
        printf("Rear is null at %d\n", pCount);
        q->front = q->rear = temp;

        //signal the consumer that the queue is no longer empty
        if(pthread_cond_signal(&lock.empty) != 0){
            perror("condition signal enqueue");
            exit(1);
        }
        pthread_mutex_unlock(&lock.mutex);
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    q->size++;
    pthread_cond_signal(&lock.empty); //signal the consumer that the queue is no longer empty
    pthread_mutex_unlock(&lock.mutex);
}

// Function to remove a key from given queue q
struct QNode* deQueue(struct Queue* q,struct locks lock){

    //lock the buffer
//    pthread_mutex_lock(&lock.mutex);

    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;

    // Store previous front and move frohttps://github.com/amcginn92/ServiceInClassActivity/tree/master/app/src/main/java/edu/temple/myapplicationthrent one node ahead
    struct QNode* temp = q->front;

    //if it's not pointing to anything it's the last element
    //clear the queue and return the node
    if(q->front->next == NULL){
        q->front = NULL;
        q->rear = NULL;
//        pthread_mutex_unlock(&lock.mutex);
        return temp;
    }
    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL){
        q->rear = NULL;
    }

    q->size--;
    temp->next = NULL;
//    pthread_mutex_unlock(&lock.mutex);
    return(temp);
}