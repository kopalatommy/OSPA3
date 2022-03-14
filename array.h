#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Max size was 17 chars, this leaves some white space 
// at the end and makes the math easy
#define MAX_NAME_LENGTH 20
#define ARRAY_SIZE 8

typedef struct Array
{
    char * buffer;
    char head;
    char tail;
    char count;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;
    pthread_cond_t space_available_cond;
    pthread_cond_t items_available_cond;
} Array;

typedef struct ProducerArgs
{
    char ** source;
    int count;
    Array * pArray;
} ProducerArgs;

typedef struct ConsumerArgs
{
    Array * pArray;
    int count;
} ConsumerArgs;

int array_init(Array * pArray);

int array_put(Array * pArray, char * str);

int array_get(Array * pArray, char ** pStr);

void array_free(Array * pArray);

#endif // _ARRAY_H_