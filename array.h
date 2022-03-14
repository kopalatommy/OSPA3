#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Max size was 17 chars, this leaves some white space 
// at the end and makes the math easy
#define MAX_NAME_LENGTH 20
#define ARRAY_SIZE 8
#define MAX_THREADS 16

typedef struct Semaphore {
    int counter;
    pthread_mutex_t ** waiting;
    int waitingCount;
} Semaphore;

typedef struct Array
{
    char * buffer;
    char head;
    char tail;
    char count;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;
    Semaphore produceSemaphore;
    Semaphore consumeSemaphore;
} Array;

int array_init(Array * pArray);

int array_put(Array * pArray, char * str);

int array_get(Array * pArray, char ** pStr);

void array_free(Array * pArray);

char semaphore_init(Semaphore * semaphore);

char semaphore_wait(Semaphore * semaphore, pthread_mutex_t * waitMutex);

char semaphore_signal(Semaphore * semaphore);

void semaphore_free(Semaphore * semaphore);

#endif // _ARRAY_H_