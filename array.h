#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

// Max size was 17 chars, this leaves some white space 
// at the end and makes the math easy
#define MAX_NAME_LENGTH 20
#define ARRAY_SIZE 8

typedef struct Array
{
    char * buffer;
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;
    sem_t produceSemaphore;
    sem_t consumeSemaphore;
} Array;

int array_init(Array * pArray);

int array_put(Array * pArray, char * str);

int array_get(Array * pArray, char ** pStr);

int array_count(Array * pArray);

void array_free(Array * pArray);

#endif // _ARRAY_H_