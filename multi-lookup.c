#include "multi-lookup.h"

char requester_args_init(RequesterArgs* args, int numberOfSourceFiles)
{
    args->numSourceFiles = numberOfSourceFiles;

    return 0;
}

void requester_args_free(RequesterArgs* args)
{
    free(args->sourceFileNames);
}

void* requester_run(void* pArgs)
{
    //printf("Started requester: %p\n", pArgs);

    struct RequesterArgs * args = (RequesterArgs*)pArgs;

    if (args == NULL)
    {
        printf("Thread received bad args: %p\n", pArgs);
        return (void*)-1;
    }

    char * buffer = (char*)malloc(MAX_NAME_LENGTH);
    size_t len = 0;

    pthread_mutex_t waitMutex;
    pthread_mutexattr_t mutex_attr;
    if(pthread_mutexattr_init(&args->sharedArray->mutex_attr) == -1)
    {
        printf("Producer failed to init mutex attr\n");
        return (void*)-1;
    }
    if(pthread_mutex_init(&waitMutex, &args->sharedArray->mutex_attr) == -1)
    {
        printf("Producer failed to init mutex\n");
        return (void*)-1;
    }

    //printf("Requester finished init: %p\n", pArgs);
    
    //printf("Reading %i files\n", args->numSourceFiles);
    /*for(int i = 0; i < args->numSourceFiles; i++)
        printf("%i : %s\n", i, args->sourceFileNames[i]);*/

    for(int i = 0; i < args->numSourceFiles; i++)
    {
        printf("Opening file: %s\n", args->sourceFileNames[i]);
        FILE * source = fopen(args->sourceFileNames[i], "r");
        if(source == NULL)
        {
            printf("Failed to open source file: %s\n", args->sourceFileNames[i]);
            continue;
        }

        //while (getline(&buffer, &len, source) != -1)
        while(fgets(buffer, MAX_NAME_LENGTH, source) != NULL)
        {
            // if(buffer[len-2] == 10 || buffer[len-2] == 13)
            //     buffer[len - 2] = 0;
            printf("Put: %s from %s\n", buffer, args->sourceFileNames[i]);

            semaphore_wait(&args->sharedArray->produceSemaphore, &waitMutex);

            if(array_put(args->sharedArray, buffer) == -1)
            {
                printf("Producer failed to put\n");
            }
            else
                semaphore_signal(&args->sharedArray->consumeSemaphore);

            semaphore_wait(args->logFileSemaphore, &waitMutex);
            fputs(buffer, args->logFile);
            semaphore_signal(args->logFileSemaphore);
        }
        printf("Finished file\n");
        
        fclose(source);
    }

    printf("Finished requester thread\n");

    free(buffer);
}

char resolver_args_init(ResolverArgs* args)
{
    return 0;
}

void resolver_args_free(ResolverArgs* args)
{

}

void* resolver_run(void* pArgs)
{
    ResolverArgs * args = (ResolverArgs*)pArgs;

    printf("Started resolver\n");

    pthread_mutex_t waitMutex;
    pthread_mutexattr_t mutex_attr;
    if(pthread_mutexattr_init(&args->sharedArray->mutex_attr) == -1)
    {
        printf("Producer failed to init mutex attr\n");
        return (void*)-1;
    }
    if(pthread_mutex_init(&waitMutex, &args->sharedArray->mutex_attr) == -1)
    {
        printf("Producer failed to init mutex\n");
        return (void*)-1;
    }

    pthread_mutex_lock(&waitMutex);

    char * buffer = (char*)malloc(MAX_NAME_LENGTH);
    char ret;
    while ((*args->finished == 0) || (args->sharedArray->count > 0))
    {
        printf("Finished byte: %i, array count: %i\n", *args->finished, args->sharedArray->count);
        printf("Exit: %i\n", (!(*args->finished) || args->sharedArray->count > 0) ? 0 : 1);
        printf("args->finished: %i\n", *args->finished);
        printf("args->sharedArray->count: %i\n", args->sharedArray->count);
        printf("args->sharedArray->count > 0: %i\n", (args->sharedArray->count > 0) ? 1 : 0);

        printf("Consume semaphore value: %i\n", args->sharedArray->consumeSemaphore.counter);
        semaphore_wait(&args->sharedArray->consumeSemaphore, &waitMutex);

        if((ret = array_get(args->sharedArray, &buffer)) != 0)
        {
            printf("Consumer failed to get: %i\n", ret);
            continue;
        }
        semaphore_signal(&args->sharedArray->produceSemaphore);
        
        semaphore_wait(args->logFileSemaphore, &waitMutex);
        if(fputs(buffer, args->logFile) <= 0)
            printf("Resolver failed to write to log file\n");
        semaphore_signal(args->logFileSemaphore);
        printf("Read: %s \n", buffer);
    }
    free(buffer);

    printf("resolver finished\n");
}

void print_args(RequesterArgs * args, int i)
{
    printf("\n");
    printf("\nRequester Args %i : %p\n", i, args);
    printf("Shared array: %p\n", args->sharedArray);
    printf("Log file: %p\n", args->logFile);
    printf("Log file semaphore: %p\n", args->logFileSemaphore);
    printf("Num source files: %i\n", args->numSourceFiles);
    printf("Source files pointer: %p\n", args->sourceFileNames);
    for(int j = 0; j < args->numSourceFiles; j++)
        printf("%s\n", args->sourceFileNames[j]);
    printf("\n");
}

char build_requesters(int* numRequesters, char * logFileName, struct Array * sharedArray, FILE ** requesterFile, pthread_t ** pRequesterThreads, struct RequesterArgs ** pRequesterArgs, int numSource, char**sourceNames, Semaphore * logFileSemaphore)
{
    if(*numRequesters > numSource)
    {
        printf("More requesters than source files. Resizing\n");
        *numRequesters = numSource;
    }

    *requesterFile = fopen(logFileName, "w");
    if(requesterFile == NULL)
    {
        printf("Failed to open requester log file: %s\n", logFileName);
        return -1;
    }
    // else
    //     printf("Opened requester log file\n");

    pthread_t *requesterThreads = (pthread_t*)malloc(sizeof(pthread_t)*(*numRequesters));

    struct RequesterArgs * requesterArgs = (struct RequesterArgs*)malloc(sizeof(struct RequesterArgs)*(*numRequesters));

    //*requesterArgs = (RequesterArgs*)malloc(sizeof(RequesterArgs*)*numRequesters);

    int counter = 0;
    int numFiles = numSource / (*numRequesters);
    int numAdditional = numSource % (*numRequesters);
    int numRequest = 0;
    printf("Num files per thread: %i, remaining: %i\n", numFiles, numAdditional);
    for(int i = 0; i < (*numRequesters); i++)
    {
        //requesterArgs[i] = (RequesterArgs)malloc(sizeof(RequesterArgs));
        //printf("Creating requester %i with args: %p\n", i, &requesterArgs[i]);

        numRequest = numFiles + (i < numAdditional ? 1 : 0);

        requesterArgs[i].logFile = *requesterFile;
        requesterArgs[i].logFileSemaphore = logFileSemaphore;
        requesterArgs[i].sourceFileNames = (char**)malloc(numRequest*sizeof(char*));
        requesterArgs[i].numSourceFiles = numRequest;
        requesterArgs[i].sharedArray = sharedArray;

        for(int j = 0; j < numRequest; j++)
        {
            requesterArgs[i].sourceFileNames[j] = (char*)malloc(MAX_NAME_LENGTH);
            strcpy(requesterArgs[i].sourceFileNames[j], sourceNames[counter]);
            counter++;
        }
    }

    for (int i = 0; i < (*numRequesters); i++)
    {
        //print_args(&requesterArgs[i], i);

        if(pthread_create(&requesterThreads[i], NULL, requester_run, (void*) &requesterArgs[i]) != 0)
        {
            printf("Failed to start requester thread: %i\n", i);
            return -1;
        }
    }

    *pRequesterArgs = requesterArgs;
    *pRequesterThreads = requesterThreads;

    printf("Started all requester threads\n");

    return 0;
}

char build_resolvers(int numResolvers, char * logFileName, Array * sharedArray, FILE ** resolverFile, char * finished, pthread_t ** resolverThreads, ResolverArgs ** resolverArgs, Semaphore * logFileSemaphore)
{
    *resolverFile = fopen(logFileName, "w");
    if(resolverFile == NULL)
    {
        printf("Failed to open resolver log file: %s\n", logFileName);
        return -1;
    }

    *resolverThreads = (pthread_t*)malloc(sizeof(pthread_t)*numResolvers);

    *resolverArgs = (ResolverArgs*)malloc(sizeof(ResolverArgs)*numResolvers);
    for(int i = 0; i < numResolvers; i++)
    {
        resolverArgs[i] = (ResolverArgs*)malloc(sizeof(ResolverArgs));
        if(resolver_args_init(resolverArgs[i]) != 0)
        {
            printf("Failed to init resolver arg: %i\n", i);
            return -1;
        }
        resolverArgs[i]->finished = finished;
        resolverArgs[i]->logFile = *resolverFile;
        resolverArgs[i]->logFileSemaphore = logFileSemaphore;
        resolverArgs[i]->sharedArray = sharedArray;

        if(pthread_create( resolverThreads[i], NULL, resolver_run, (void*) resolverArgs[i]) != 0)
        {
            printf("Failed to start resolver thread: %i\n", i);
            return -1; 
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    FILE * requesterLogFile;
    pthread_t * requesterThreads;
    RequesterArgs * requesterArgs;
    Semaphore requesterSemaphore;

    FILE * resolverLogFile;
    pthread_t * resolverThreads;
    ResolverArgs * resolverArgs;
    Semaphore resolverSemaphore;

    char finishedByte =  0;

    if(argc < 6)
    {
        printf("Not enough args passed. Expecting >= 6. Received: %i\n", argc);
        return -1;
    }

    int numRequesters = atoi(argv[1]);
    int numResolvers = atoi(argv[2]);

    if(numRequesters < 0)
    {
        printf("numRequesters < 0\n");
        return -1;
    }
    if(numResolvers < 0)
    {
        printf("numResolvers < 0\n");
        return -1;
    }

    char * requesterLog = argv[3];
    char * resolverLog = argv[4];

    printf("Num args: %i\n", argc);

    printf("Num requester threads: %i\n", numRequesters);
    printf("Num resolvers threads: %i\n", numResolvers);

    printf("Requester log: %s\n", requesterLog);
    printf("Resolver log: %s\n", resolverLog);

    Array sharedArray;
    if(array_init(&sharedArray) != 0)
    {
        printf("Failed to init shared array\n");
        return -1;
    }
    else
        printf("Initialized shared array\n");

    if(semaphore_init(&requesterSemaphore) != 0)
    {
        printf("Failed to init requester semaphore\n");
        return -1;
    }
    else
        printf("Initialized requester semaphore\n");
    requesterSemaphore.counter = 1;

    if(semaphore_init(&resolverSemaphore) != 0)
    {
        printf("Failed to init resolver semaphore\n");
        return -1;
    }
    else
        printf("Initialized resolver semaphore\n");
    resolverSemaphore.counter = 1;

    if(numRequesters > 0 && build_requesters(&numRequesters, requesterLog, &sharedArray, &requesterLogFile, &requesterThreads, &requesterArgs, (argc-6), argv + 6, &requesterSemaphore) != 0)
    {
        printf("Failed to build requester threads\n");
        // ToDo, free resources
        return -1;
    }
    else
        printf("Built requesters\n");

    if(numResolvers > 0 && build_resolvers(numResolvers, resolverLog, &sharedArray, &resolverLogFile, &finishedByte, &resolverThreads, &resolverArgs, &resolverSemaphore))
    {
        printf("Failed to build resolver threads\n");
        // ToDo, free resources
        return -1;
    }
    else
        printf("Built resolvers\n");

    for (int i = 0; i < numRequesters; i++)
    {
        pthread_join(requesterThreads[i], NULL);
    }
    printf("Finished requesters\n");
    finishedByte = 1;
    printf("Set finished byte\n");

    printf("Semaphore value: %i\n", sharedArray.consumeSemaphore.counter);
    printf("Shared array count: %i\n", sharedArray.count);
    printf("Finished byte pointer: %p\n", &finishedByte);

    pthread_exit(NULL);

    // Wait for all threads to exit
    for (int i = 0; i < numResolvers; i++)
    {
        pthread_join(resolverThreads[i], NULL);
    }

    printf("Finished program\n");

    return 0;
}