#include "array.h"

int array_init(Array * pArray)
{
    if(pArray == NULL)
    {
        printf("array_init passed null pointer\n");
        return -1;
    }
    printf("Array loc: %p\n", pArray);
    printf("Mutex loc: %p\n", &pArray->mutex);
    printf("Mutex attr loc: %p\n", &pArray->mutex_attr);
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

    printf("Initialized mutex\n");

    printf("Allocating of size: %i\n", (ARRAY_SIZE * MAX_NAME_LENGTH));

    // Allocate buffer array
    pArray->buffer = (char*)malloc(160);

    printf("Initialized buffer\n");

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
    // If there is no space available in the array, then return -1 for a bad read
    if (pArray->count == ARRAY_SIZE)
    {
        printf("Aray already full\n");
        return -1;
    }

    // Attempt to lock the mutex
    if(pthread_mutex_lock(&pArray->mutex) != 0)
    {
        printf("Failed to lock mutex: %i\n", pthread_mutex_lock(&pArray->mutex));
        return -1;
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

    // Release the mutex
    pthread_mutex_unlock(&pArray->mutex);
    pthread_cond_signal(&pArray->items_available_cond);

    return 0;
}

int array_get(Array * pArray, char ** pStr)
{
    // If the array is empty, then return the fail value
    if (pArray->count == 0)
        return -1;

    // Attempt to lock the mutex
    if(pthread_mutex_lock(&pArray->mutex) != 0)
    {
        printf("Failed to lock mutex: %i\n", pthread_mutex_lock(&pArray->mutex));
        return -1;
    }

    char * buffer = pArray->buffer + (pArray->tail * MAX_NAME_LENGTH);
    char * dest = *pStr;
    for(char i = 0; i < MAX_NAME_LENGTH; i++)
        *dest++ = *buffer++;

    pArray->tail = pArray->tail + 1 % (ARRAY_SIZE - 1);
    pArray->count--;

    // Release the mutex
    pthread_mutex_unlock(&pArray->mutex);
    pthread_cond_signal(&pArray->space_available_cond);

    return 0;
}

void array_free(Array * pArray)
{
    pthread_mutex_destroy(&pArray->mutex);
    free(pArray->buffer);
}
