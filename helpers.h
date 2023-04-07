#ifndef MAKEFILE_HELPERS_H
#define MAKEFILE_HELPERS_H

typedef struct{
    int pType;
    int pCount;
    pid_t threadId;
    int cCount;
}data;
//=====================================Functions
void producer(int pType, int fd[2]);

#endif