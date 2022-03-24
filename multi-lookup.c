#include "multi-lookup.h"

void producer_args_init(ProducerArgs * pArgs, FILE * logFile, pthread_mutex_t * mutex, Array * pArray, char** fileNames, int numFiles, pthread_t * threadID)
{
    pArgs->fileMutex = mutex;
    pArgs->logFile = logFile;
    pArgs->sharedArray = pArray;

    pArgs->sourceFileNames = fileNames;
    pArgs->numSourceFiles = numFiles;

    pArgs->threadID = threadID;
}

void consumer_args_init(ConsumerArgs * pArgs, FILE * logFile, pthread_mutex_t * mutex, Array * pArray, char* finishedByte, pthread_t * threadID)
{
    pArgs->fileMutex = mutex;
    pArgs->logFile = logFile;
    pArgs->sharedArray = pArray;
    pArgs->finishedByte = finishedByte;

    pArgs->threadID = threadID;
}

void * producer_work(void * pArgs)
{
    ProducerArgs* args = (ProducerArgs*)pArgs;

    if(args == NULL)
    {
        printf("Received bad args\n");
        return (void*)-1;
    }

    char buffer[MAX_NAME_LENGTH];
    memset(buffer, 0, 20);

    FILE * sourceFile = NULL;
    for(int i = 0; i < args->numSourceFiles; i++)
    {
        sourceFile = fopen(args->sourceFileNames[i], "r");

        if(sourceFile == NULL)
        {
            printf("%p failed to open %s\n", args, args->sourceFileNames[i]);
            continue;
        }

        //printf("%p - %s\n", args, args->sourceFileNames[i]);

        int len;
        while(fgets(buffer, MAX_NAME_LENGTH, sourceFile) != NULL)
        {
            len = strlen(buffer);
            if(len > 0)
                buffer[len-1] = 0;

            //printf("%p read %s\n", args, buffer);

            sem_wait(&args->sharedArray->produceSemaphore);

            if(array_put(args->sharedArray, buffer))
            {
                printf("Failed to put in shared array\n");
                continue;
            }

            sem_post(&args->sharedArray->consumeSemaphore);

            if(pthread_mutex_lock(args->fileMutex) != 0)
            {
                printf("Failed to lock mutex\n");
                continue;
            }

            fprintf(args->logFile, "%s\n", buffer);

            pthread_mutex_unlock(args->fileMutex);
        }

        fclose(sourceFile);
    }

    printf("thread %lx serviced %i files\n", *args->threadID, args->numSourceFiles);

    return (void*)-1;
}

void * consumer_work(void * pArgs)
{
    ConsumerArgs* args = (ConsumerArgs*)pArgs;

    if(args == NULL)
    {
        printf("Received bad args\n");
        return (void*)-1;
    }

    char * buffer = (char*)malloc(MAX_NAME_LENGTH);
    memset(buffer, 0, 20);
    char * ipBuffer = (char*)malloc(MAX_IP_LENGTH);
    memset(ipBuffer, 0, 20);

    int servicedCount = 0;
    while((*args->finishedByte == 0) || (array_count(args->sharedArray) > 0))
    {
        //printf("Consumer top of loop\n");
        sem_wait(&args->sharedArray->consumeSemaphore);

        if(array_get(args->sharedArray, &buffer))
        {
            printf("Failed to put in shared array\n");
            continue;
        }

        sem_post(&args->sharedArray->produceSemaphore);

        if (dnslookup(buffer, ipBuffer, MAX_NAME_LENGTH) == UTIL_SUCCESS)
        {
            if(pthread_mutex_lock(args->fileMutex) != 0)
            {
                printf("Failed to lock mutex\n");
                continue;
            }
            
            fprintf(args->logFile, "%s, %s\n", buffer, ipBuffer);
            servicedCount++;

            pthread_mutex_unlock(args->fileMutex);
        }
        else
        {
            if(pthread_mutex_lock(args->fileMutex) != 0)
            {
                printf("Failed to lock mutex\n");
                continue;
            }
            
            fprintf(args->logFile, "%s, NOT_RESOLVED\n", buffer);
            servicedCount++;

            pthread_mutex_unlock(args->fileMutex);
        }
    }

    free(buffer);
    free(ipBuffer);

    printf("thread %lx resolved %i hostnames\n", *args->threadID, servicedCount);

    return (void*)-1;
}

int main(int argc, char *argv[])
{
    struct timeval startTime;
    struct timeval endTime;

    if(argc < 6)
        return -1;

    gettimeofday(&startTime, NULL);

    FILE * producerLog = fopen(argv[3], "w");
    if(producerLog == NULL)
    {
        printf("Failed to open producer log\n");
        exit(1);
    }
    FILE * consumerLog = fopen(argv[4], "w");
    if(consumerLog == NULL)
    {
        printf("Failed to open consumer log\n");
        exit(1);
    }

    char finishedByte = 0;

    pthread_mutexattr_t consumerMutexAttr;
    pthread_mutex_t consumerMutex;

    pthread_mutexattr_t producerMutexAttr;
    pthread_mutex_t producerMutex;

    if(pthread_mutexattr_init(&consumerMutexAttr) != 0)
    {
        printf("Failed to init mutex attr\n");
        exit(-1);
    }
    if(pthread_mutex_init(&consumerMutex, &consumerMutexAttr) != 0)
    {
        printf("Failed to init mutex\n");
        exit(-2);
    }
    if(pthread_mutexattr_init(&producerMutexAttr) != 0)
    {
        printf("Failed to init mutex attr\n");
        exit(-1);
    }
    if(pthread_mutex_init(&producerMutex, &producerMutexAttr) != 0)
    {
        printf("Failed to init mutex\n");
        exit(-2);
    }

    Array sharedArray;
    if(array_init(&sharedArray))
    {
        printf("Failed to init shared array\n");
    }

    int numConsumers = atoi(argv[1]);
    int numProducers = atoi(argv[2]);

    if (numConsumers > MAX_RESOLVER_THREADS)
        numConsumers = MAX_RESOLVER_THREADS;
    if (numProducers > MAX_REQUESTER_THREADS)
        numProducers = MAX_REQUESTER_THREADS;

    int numFiles = argc - 5;
    if (numFiles > MAX_INPUT_FILES)
        numFiles = MAX_INPUT_FILES;

    if (numProducers > numFiles)
        numProducers = numFiles;

    //printf("Num producers: %i\nNum files: %i\n", numProducers, numFiles);

    pthread_t consumerThreads[numConsumers];
    pthread_t producerThreads[numProducers];
    struct ConsumerArgs consumerArgs[numConsumers];
    struct ProducerArgs producerArgs[numProducers];
    for(int i = 0; i < numConsumers; i++)
    {
        consumer_args_init(&consumerArgs[i], consumerLog, &consumerMutex, &sharedArray, &finishedByte, &consumerThreads[i]);
    }
    
    //printf("Num files: %i\n", numFiles);
    int numFilesPerThread = numFiles/numProducers;
    int remainder = numFiles%numProducers;
    //printf("Num files per thread: %i\nRemainder: %i\n", numFilesPerThread, remainder);
    //printf("Num files per thread: %i, remainder: %i\n", numFilesPerThread, numFiles%numProducers);
    argv += 5;
    for(int i = 0; i < numProducers; i++)
    {
        producer_args_init(&producerArgs[i], producerLog, &producerMutex, &sharedArray, argv++, numFilesPerThread, &producerThreads[i]);
        if(i < remainder)
        {
            producerArgs[i].numSourceFiles++;
            argv++;
            producerArgs[i].sourceFileNames++;
        }
    }

    //printf("Initialized thread args\n");

    for(int i = 0; i < numConsumers; i++)
    {
        if(pthread_create(&consumerThreads[i], NULL, consumer_work, (void*) &consumerArgs[i]) != 0)
        {
            printf("Failed to start thread: %i\n", i);
        }
        //printf("Started consumer thread: %X\n", consumerThreads[i]);
    }
    for(int i = 0; i < numProducers; i++)
    {
        if(pthread_create(&producerThreads[i], NULL, producer_work, (void*) &producerArgs[i]) != 0)
        {
            printf("Failed to start thread: %i\n", i);
        }
        //printf("Started producer thread: %X\n", producerThreads[i]);
    }

    for(int i = 0; i < numProducers; i++)
    {
        pthread_join(producerThreads[i], NULL);
        //printf("Producer %i / %i exited\n", i + 1, numProducers);
    }
    //printf("Producers finished\n");
    finishedByte = 1;

    //printf("Produce count: %i\n", produceCount);
    //printf("Consume count: %i\n", consumeCount);

    if(array_count(&sharedArray) == 0)
    {
        // Program is finished
        array_free(&sharedArray);

        //printf("Free-ed array loc A\n");
    }
    else
    {
        // Use the produce semaphore as a way to prevent busy waiting
        while(sem_wait(&sharedArray.produceSemaphore) == 0)
        {
            if(array_count(&sharedArray) == 0)
            {
                // Program is finished
                array_free(&sharedArray);

                //printf("Free-ed array loc B\n");
                break;
            }
        }
        for(int i = 0; i < numConsumers; i++)
        {
            pthread_join(consumerThreads[i], NULL);
            //printf("Producer %i / %i exited\n", i + 1, numProducers);
        }
    }
    
    fclose(producerLog);
    fclose(consumerLog);

    gettimeofday(&endTime, NULL);

    printf("./multi-lookup: total time is %1.6f seconds\n", ((endTime.tv_sec - startTime.tv_sec) + 1e-9*(endTime.tv_usec - startTime.tv_usec)));
}