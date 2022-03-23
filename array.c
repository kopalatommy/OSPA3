#include "array.h"

int array_init(Array * pArray)
{
    if(pArray == NULL)
    {
        printf("array_init passed null pointer\n");
        return -1;
    }
    if(pthread_mutexattr_init(&pArray->mutex_attr) == -1)
    {
        return -1;
    }
    char init;
    if((init = pthread_mutex_init(&pArray->mutex, &pArray->mutex_attr)) != 0)
    {
        printf("Failed to init mutex: %i\n", init);
        return -1;
    }

    semaphore_init(&pArray->consumeSemaphore);
    pArray->consumeSemaphore.counter = 0;
    semaphore_init(&pArray->produceSemaphore);
    pArray->produceSemaphore.counter = ARRAY_SIZE - 1;

    // Allocate buffer array
    pArray->buffer = (char*)malloc(160);

    // Initialize object values
    pArray->head = 0;
    pArray->tail = 0;
    pArray->count = 0;

    if(pArray != NULL)
    {
        // Clear all bytes in the array
        for(short i = ARRAY_SIZE * MAX_NAME_LENGTH - 1; i >= 0; i--)
            pArray->buffer[i] = 0;
    }
    
    // Was successfull if the buffer is not null
    return pArray->buffer != NULL ? 0 : -1;
}

int array_put(Array * pArray, char * str)
{
    //printf("Put locking mutex\n");
    // Attempt to lock the mutex
    if(pthread_mutex_lock(&pArray->mutex) != 0)
    {
        printf("Failed to lock mutex: %i\n", pthread_mutex_lock(&pArray->mutex));
        return -1;
    }

    // If there is no space available in the array, then return -1 for a bad read
    if (pArray->count == ARRAY_SIZE)
    {
        pthread_mutex_unlock(&pArray->mutex);
        return -1;

    //printf("Put after locking mutex\n");
    //printf("put open\n");

    if(pArray->tail * MAX_NAME_LENGTH >= (MAX_NAME_LENGTH * ARRAY_SIZE))
    {
        printf("Array out of bounds\n");
    }

    // Go to the next write location in the buffer
    char * buffer = pArray->buffer + (pArray->head * MAX_NAME_LENGTH);

    char count = MAX_NAME_LENGTH;
    while (count-- && str)
        *buffer++ = *str++;

    if(pArray->count == ARRAY_SIZE)
        pArray->tail = pArray->tail + 1 % (ARRAY_SIZE);
    else
        pArray->count++;

    pArray->head = (pArray->head + 1) % (ARRAY_SIZE);

    //printf("put close\n");

    // Release the mutex
    pthread_mutex_unlock(&pArray->mutex);

    return 0;
}

int array_get(Array * pArray, char ** pStr)
{
    //printf("Get locking mutex\n");
    // Attempt to lock the mutex
    if(pthread_mutex_lock(&pArray->mutex) != 0)
    {
        printf("Failed to lock mutex: %i\n", pthread_mutex_lock(&pArray->mutex));
        return -1;
    }
    //printf("Get after locking mutex\n");

    // If the array is empty, then return the fail value
    if (pArray->count == 0)
    {
        pthread_mutex_unlock(&pArray->mutex);
        return -2;
    }

    //printf("Get open\n");

    if(pArray->tail * MAX_NAME_LENGTH >= (MAX_NAME_LENGTH * ARRAY_SIZE))
    {
        printf("Array out of bounds\n");
    }
    char * buffer = pArray->buffer + (pArray->tail * MAX_NAME_LENGTH);
    char * dest = *pStr;
    for(char i = 0; i < MAX_NAME_LENGTH; i++)
        *dest++ = *buffer++;

    //pArray->tail = pArray->tail - 1 >= 0 ? pArray->tail - 1 : (ARRAY_SIZE - 1);
    pArray->tail = (pArray->tail + 1) % (ARRAY_SIZE);
    pArray->count--;

    //printf("get close\n");

    // Release the mutex
    //printf("Get unlocking mutex\n");
    pthread_mutex_unlock(&pArray->mutex);

    return 0;
}

void array_free(Array * pArray)
{
    pthread_mutex_destroy(&pArray->mutex);
    semaphore_free(&pArray->consumeSemaphore);
    semaphore_free(&pArray->produceSemaphore);
    free(pArray->buffer);
}

char semaphore_init(Semaphore * semaphore)
{
    semaphore->waiting = (pthread_mutex_t**)malloc(sizeof(pthread_mutex_t*));
    semaphore->counter = 0;
    semaphore->waitingCount = 0;
    semaphore->arrayCount = 1;

    return semaphore->waiting != NULL ? 0 : -1;
}

char semaphore_wait(Semaphore * semaphore, pthread_mutex_t * waitMutex)
{
    printf("Entered semaphore_wait: %i\n", semaphore->counter);
    semaphore->counter--;
    if(semaphore->counter < 0)
    {
        printf("Blocking thread\n");
        if(pthread_mutex_lock(waitMutex) == -1)
        {
            printf("Semaphore failed to lock mutex\n");
        }
        // else
        //     printf("Semaphore locked mutex\n");
        if(semaphore->waitingCount + 1 > semaphore->arrayCount)
        {
            semaphore->arrayCount++;
            semaphore->waiting = (pthread_mutex_t **)realloc(semaphore->waiting, sizeof(pthread_mutex_t*)*semaphore->arrayCount);
        }
        semaphore->waiting[semaphore->waitingCount++] = waitMutex;
        printf("Finished waiting\n");
        //printf("Blocking thread. Count: %i\n", semaphore->waitingCount);
    }
    return 0;
}

char semaphore_signal(Semaphore * semaphore)
{
    printf("Entered semaphore_signal\n");
    semaphore->counter++;
    if(semaphore->waitingCount > 0)
    {
        pthread_mutex_t * toAwake = semaphore->waiting[0];
        for(char i = 1; i < semaphore->waitingCount; i++)
            semaphore->waiting[i - 1] = semaphore->waiting[i];
        semaphore->waitingCount--;
        //printf("Waking thread. Count: %i\n", semaphore->waitingCount);
        pthread_mutex_unlock(toAwake);
    }
    return 0;
}

void semaphore_free(Semaphore * semaphore)
{
    free(semaphore->waiting);
}