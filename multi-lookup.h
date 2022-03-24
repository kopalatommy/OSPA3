#ifndef _MULTI_LOOKUP_H_
#define _MULTI_LOOKUP_H_

// printf, fgets
#include <stdio.h>
// strlen
#include <string.h>
// 
#include <stdlib.h>
// threading funct
#include <pthread.h>
// gettimeofday
#include <time.h>
#include <sys/time.h>

#include "array.h"
#include "util.h"

// #define NUM_PRODUCERS 16
// #define NUM_CONSUMERS 16

#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

typedef struct ProducerArgs
{
    FILE * logFile;
    pthread_mutex_t * fileMutex;
    Array * sharedArray;

    char** sourceFileNames;
    int numSourceFiles;
    
    pthread_t * threadID;
} ProducerArgs;

typedef struct ConsumerArgs
{
    FILE * logFile;
    pthread_mutex_t * fileMutex;
    Array * sharedArray;

    char * finishedByte;

    pthread_t * threadID;
} ConsumerArgs;

void consumer_args_init(ConsumerArgs * pArgs, FILE * logFile, pthread_mutex_t * fileMutex, Array * sharedArray, char* finishedByte, pthread_t* threadID);
void producer_args_init(ProducerArgs * pArgs, FILE * logFile, pthread_mutex_t * fileMutex, Array * sharedArray, char** fileNames, int numFiles, pthread_t* threadID);

#endif // _MULTI_LOOKUP_H_