#include "array.h"

int array_init(Array * pArray)
{
    //printf("Starting array init\n");

    if(pArray == NULL)
    {
        //printf("array_init passed null pointer\n");
        return -1;
    }
    if(pthread_mutexattr_init(&pArray->mutex_attr) == -1)
    {
        return -1;
    }
    char init;
    if((init = pthread_mutex_init(&pArray->mutex, &pArray->mutex_attr)) != 0)
    {
        //printf("Failed to init mutex: %i\n", init);
        return -1;
    }

    if(sem_init(&pArray->produceSemaphore, 0, ARRAY_SIZE - 1))
    {
        //printf("Failed to init producer semaphore\n");
        return -1;
    }
    //printf("initializing producer semaphore to %i\n", ARRAY_SIZE - 1);
    if(sem_init(&pArray->consumeSemaphore, 0, 0))
    {
        //printf("Failed to init producer semaphore\n");
        return -1;
    }

    // Allocate buffer array
    pArray->buffer = (char*)malloc(ARRAY_SIZE * MAX_NAME_LENGTH);
    //printf("Allocated array of size(char): %i\n", ARRAY_SIZE * MAX_NAME_LENGTH);

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
        //printf("Failed to lock mutex put: %i\n", pthread_mutex_lock(&pArray->mutex));
        return -1;
    }

    // If there is no space available in the array, then return -1 for a bad read
    if (pArray->count == ARRAY_SIZE)
    {
        //printf("Array full\n");
        pthread_mutex_unlock(&pArray->mutex);
        return -1;
    }

    //printf("Put after locking mutex\n");
    //printf("put open\n");

    if(pArray->tail * MAX_NAME_LENGTH >= (MAX_NAME_LENGTH * ARRAY_SIZE))
    {
        //printf("Array out of bounds\n");
        pthread_mutex_unlock(&pArray->mutex);
        return -1;
    }

    // Go to the next write location in the buffer
    char * buffer = pArray->buffer + (pArray->head * MAX_NAME_LENGTH);

    int count = MAX_NAME_LENGTH;
    while (count-- && str)
        *buffer++ = *str++;

    if(pArray->count == ARRAY_SIZE)
        pArray->tail = pArray->tail + 1 % (ARRAY_SIZE);
    else
        pArray->count++;

    pArray->head = (pArray->head + 1) % (ARRAY_SIZE);

    //printf("put close\n");

    // printf("Head: %i\n", pArray->head);
    // printf("Tail: %i\n", pArray->tail);
    // printf("Count: %i\n", pArray->count);
    // printf("SIZE: %i\n", ARRAY_SIZE);

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
        //printf("Failed to lock mutex get: %i\n", pthread_mutex_lock(&pArray->mutex));
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
        //printf("Array out of bounds\n");
        pthread_mutex_unlock(&pArray->mutex);
        return -3;
    }
    char * buffer = pArray->buffer + (pArray->tail * MAX_NAME_LENGTH);
    char * dest = *pStr;
    for(int i = 0; i < MAX_NAME_LENGTH; i++)
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

int array_count(Array * pArray)
{
    int ret = 0;

    if(pArray == NULL)
        return -1;

    if(pthread_mutex_lock(&pArray->mutex) != 0)
    {
        //printf("Failed to lock mutex count: %i\n", pthread_mutex_lock(&pArray->mutex));
        return -1;
    }

    ret = pArray->count;

    pthread_mutex_unlock(&pArray->mutex);

    return ret;
}

void array_free(Array * pArray)
{
    pthread_mutex_destroy(&pArray->mutex);
    sem_close(&pArray->consumeSemaphore);
    sem_close(&pArray->produceSemaphore);
    free(pArray->buffer);
}