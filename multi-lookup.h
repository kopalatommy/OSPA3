#ifndef _MULTI_LOOKUP_H_
#define _MULTI_LOOKUP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "array.h"

typedef struct RequesterArgs
{
    char ** sourceFileNames;
    int numSourceFiles;

    Array * sharedArray;
    
    FILE * logFile;
    Semaphore * logFileSemaphore;
} RequesterArgs;

typedef struct ResolverArgs
{
    FILE * logFile;
    Semaphore * logFileSemaphore;
    
    Array * sharedArray;

    char * finished;
} ResolverArgs;

char requester_args_init(RequesterArgs* args, int numSourceFiles);
void requester_args_free(RequesterArgs* args);
void* requester_run(void* args);

char resolver_args_init(ResolverArgs* args);
void resolver_args_free(ResolverArgs* args);
void* resolver_run(void* args);

#endif