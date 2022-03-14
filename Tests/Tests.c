#include "Tests.h"

void m_memset(char * arr, char val, size_t len)
{
    while(len--)
    {
        *(arr++) = val;
    }
}

void m_memcpy(char * dest, char * source, int len)
{
    while (len--)
    {
        *(dest++) = *(source++);
    }
}

char test_array_runAll()
{
    char count = 0;

    if(test_array_initializeTest() == 1)
    {
        count++;
        printf("test_array_initializeTest passed\n");
    }

    if(test_array_put1() == 1)
    {
        count++;
        printf("test_array_put1 passed\n");
    }

    if(test_array_put2() == 1)
    {
        count++;
        printf("test_array_put2 passed\n");
    }

    if(test_array_put3() == 1)
    {
        count++;
        printf("test_array_put3 passed\n");
    }

    if(test_array_put4() == 1)
    {
        count++;
        printf("test_array_put4 passed\n");
    }

    if(test_array_get1() == 1)
    {
        count++;
        printf("test_array_get1 passed\n");
    }

    if(test_array_get2() == 1)
    {
        count++;
        printf("test_array_get2 passed\n");
    }

    if(test_array_get3() == 1)
    {
        count++;
        printf("test_array_get3 passed\n");
    }

    if(test_thread_concurrent_get_put() == 1)
    {
        count++;
        printf("test_thread_concurrent_get_put passed\n");
    }

    if(test_multthread_test() == 1)
    {
        count++;
        printf("test_multthread_test passed\n");
    }

    printf("Array tests: %i / %i passed\n", count, 10);

    return count == 10;
}

// This makes sure the array is correctly initialized
char test_array_initializeTest()
{
    printf("Starting array init test\n");

    Array * array = (Array*)malloc(sizeof(Array));

    char ret = array_init(array);

    if(ret == 0)
    {
        if(array->buffer == NULL)
        {
            ret = 0;
            printf("Array init test failed. Array is null\n");
        }
        if(array->count != 0)
        {
            ret = 0;
            printf("Array init test failed. Array count not 0\n");
        }
        if(array->head != 0)
        {
            ret = 0;
            printf("Array init test failed. Array head != 0\n");
        }
        if(array->tail != 0)
        {
            ret = 0;
            printf("Array init test failed. Array tail != 0\n");
        }

        array_free(array);
    }

    free(array);

    return ret == 0;
}

// Tests adding 1 string to the array
char test_array_put1()
{
    printf("Starting array put 1 test\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char testArr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};

    if(array_init(array) != 0)
    {
        printf("Failed to init\n");
        return 0;
    }

    if(array_put(array, testArr) != 0)
    {
        printf("Array put test 1 failed when calling put\n");
        array_free(array);
        free(array);
        return 0;
    }

    for(char i = 0; i < 20; i++)
    {
        if(array->buffer[i] != (i + 1))
        {
            printf("Array put test 1 failed on check. Ind: %i\n", i);
            array_free(array);
            free(array);
            return 0;
        }
    }

    array_free(array);
    free(array);

    return 1;
}

// Tests filling the array
char test_array_put2()
{
    printf("Starting array put 2 test\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char * source = (char*)malloc(21);

    if(array_init(array) != 0)
        return 0;

    for (char j = 1; j < 21; j++)
        source[j - 1] = j;

    // Check adding to array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        if(array_put(array, source) != 0)
        {
            printf("Array put test 2 failed when calling funct on iter: %i\n", i);
            array_free(array);
            free(array);
            return 0;
        }
    }

    // Verify the put actually happened
    for(char i = 0; i < 20; i++)
    {
        m_memset(source, 0, 21);
        if(array->buffer[i] != (i + 1))
        {
            printf("Array put test 1 failed on check. Ind: %i\n", i);
            array_free(array);
            free(array);
            free(source);
            return 0;
        }
    }

    array_free(array);
    free(array);
    free(source);

    return 1;
}

// Tests putting more than ARRAY_SIZE strings into the array
// Fail case
char test_array_put3()
{
    printf("Starting array put 3 test\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char * source = (char*)malloc(21);

    if(array_init(array) != 0)
        return 0;

    for (char j = 1; j < 21; j++)
        source[j - 1] = j;

    // Check adding to array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        if(array_put(array, source) != 0)
        {
            printf("Array put test 2 failed when calling funct on iter: %i\n", i);
            array_free(array);
            free(array);
            return 0;
        }
    }

    //printf("Trying over write\n");

    // The next put should fail because the array is full
    char success = array_put(array, source);

    //printf("Exit: %i\n", success);

    array_free(array);
    free(array);
    free(source);

    return success == -1;
}

// Tests writing a string > MAX_NAME_LENGTH into the array
char test_array_put4()
{
    printf("Starting array put 4 test\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char * source = (char*)malloc(51);

    if(array_init(array) != 0)
        return 0;

    for (char j = 1; j < 51; j++)
        source[j - 1] = j;

    // Check adding to array
    if(array_put(array, source) != 0)
    {
        printf("Array put test 4 failed when calling put\n");
        array_free(array);
        free(array);
        return 0;
    }

    // Check that only the first MAX_NAME_SIZE (20) bytes were written
    for(char i = 0; i < 50; i++)
    {
        // Verify the first 20 bytes were properly written
        if(i < 20 && *(array->buffer + i) != (i + 1))
        {
            printf("Array put test 4 failed. Did not properly write\n");
            array_free(array);
            free(array);
            return 0;
        }
        else if(i >= 20 && *(array->buffer + i) != 0)
        {
            printf("Array put test 4 failed. Wrote past buffer\n");
            array_free(array);
            free(array);
            return 0;
        }
    }

    array_free(array);
    free(array);
    return 1;
}

// Tests getting 1 string from the array
char test_array_get1()
{
    printf("Starting array get test 1\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char source[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};
    char * dest = (char*)malloc(21);
    m_memset(dest, 0, 21);

    if(array_init(array) != 0)
        return 0;

    if(array_put(array, source) != 0)
    {
        printf("Array get test 1 failed when calling put\n");
        array_free(array);
        free(array);
        free(dest);
        return 0;
    }

    if(array_get(array, &dest) != 0)
    {
        printf("Array get test 1 failed when calling get\n");
        array_free(array);
        free(array);
        free(dest);
        return 0;
    }

    for(char i = 0; i < 20; i++)
    {
        if(source[i] != dest[i])
        {
            printf("Array get test 1 failed. Source != dest\n");
            for (char j = 0; j < 20; j++)
            {
                printf("%i : %i != %i\n", j, dest[j], dest[j]);
            }
            array_free(array);
            free(array);
            free(dest);
            return 0;
        }
    }

    array_free(array);
    free(array);
    free(dest);

    return 1;
}

// Tests fully filling and then getting all strings in the array
char test_array_get2()
{
    printf("Starting array get test 2\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char * source = (char*)malloc(20);

    if(array_init(array) != 0)
        return 0;

    m_memset(array->buffer, -1, (ARRAY_SIZE * MAX_NAME_LENGTH));

    // Completely fill the array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        m_memset(source, i, 20);

        if(array_put(array, source) != 0)
        {
            printf("Array put test 2 failed when calling put on iter: %i\n", i);
            array_free(array);
            free(array);
            free(source);
            return 0;
        }
    }
    // for(unsigned char k = 0; k < (MAX_NAME_LENGTH * ARRAY_SIZE); k++)
    // {
    //     printf("%i : %i\n", k, array->buffer[k]);
    // }
    // array_free(array);
    // free(array);
    // free(source);
    // return 1;

    // Completely read from the array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        m_memset(source, 0, 20);
        if(array_get(array, &source) != 0)
        {
            printf("Array put test 2 failed when calling get on iter: %i\n", i);
            array_free(array);
            free(array);
            free(source);
            return 0;
        }

        for(char j = 0; j < 20; j++)
        {
            if(source[j] != i)
            {
                printf("Array get test 2 failed. Received unexpectd value\n");
                printf("Offender: source[%i] on iter %i\n", j, i);
                printf("Received: ");
                for(char k = 0; k < 20; k++)
                {
                    printf("%i ", source[k]);
                }
                printf("\n");

                for(unsigned char k = 0; k < (MAX_NAME_LENGTH * ARRAY_SIZE); k++)
                {
                    printf("%i : %i\n", k, array->buffer[k]);
                }
                array_free(array);
                free(array);
                free(source);
                return 0;
            }
        }
    }

    array_free(array);
    free(array);
    free(source);
    return 1;
}

// Tests the operations: put -> get -> put -> get
char test_array_get3()
{
    printf("Starting array get test 3\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char * source = (char*)malloc(21);

    if(array_init(array) != 0)
        return 0;


    for(char i = 1; i < 21; i++)
        source[i-1] = i;

    if(array_put(array, source) != 0)
    {
        printf("Array put test 3 failed when calling put\n");
        array_free(array);
        free(array);
        free(source);
        return 0;
    }

    m_memset(source, 0, 20);

    if(array_get(array, &source) != 0)
    {
        printf("Array put test 3 failed when calling get\n");
        array_free(array);
        free(array);
        free(source);
        return 0;
    }

    for(char i = 1; i < 21; i++)
        if(source[i - 1] != i)
        {
            printf("Array put test 3 failed while checking first\n");
            array_free(array);
            free(array);
            free(source);
            return 0;
        }

    for(char i = 21; i < 41; i++)
        source[i - 21] = i;

    if(array_put(array, source) != 0)
    {
        printf("Array put test 3 failed when calling put 2\n");
        array_free(array);
        free(array);
        free(source);
        return 0;
    }

    if(array_get(array, &source) != 0)
    {
        printf("Array put test 3 failed when calling get\n");
        array_free(array);
        free(array);
        free(source);
        return 0;
    }

    for(char i = 21; i < 41; i++)
        if(source[i - 21] != i)
        {
            printf("Array put test 3 failed while checking second\n");
            array_free(array);
            free(array);
            free(source);
            return 0;
        }

    array_free(array);
    free(array);
    free(source);
    return 1;
}

void *test_producer_funct(void * pArgs)
{
    struct ProducerArgs * args = (ProducerArgs*)pArgs;

    for(char i = 0; i < args->count; i++)
    {
        while(array_put(args->pArray, *(args->source + i)) == -1);
    }

    printf("Producer finished\n");
}

void *test_consumer_funct(void * pArgs)
{
    struct ConsumerArgs * args = (ConsumerArgs*)pArgs;

    char * buffer = (char*)malloc(21);
    while (args->count > 0)
    {
        while(array_get(args->pArray, &buffer) == -1);
        args->count--;
        printf("%s\n", buffer);
    }
    free(buffer);

    printf("Consumer finished\n");
}

char test_thread_concurrent_get_put()
{
    pthread_t producer_thread, consumer_thread;

    Array * sharedArray = (Array*)malloc(sizeof(Array));

    printf("Starting test_thread_concurrent_get_put\n");

    if(array_init(sharedArray) != 0)
    {
        printf("Failed to initialize array\n");
        return -1;
    }

    char testMessage[] = {'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A', ':', ' ', 0};
    char ** testData = (char**)malloc(sizeof(char*)*8);
    for(char i = 0; i < 8; i++)
    {
        testData[i] = (char*)malloc(MAX_NAME_LENGTH);
        m_memset(testData[i], 0, MAX_NAME_LENGTH);
        m_memcpy(testData[i], testMessage, 11);
        testData[i][11] = i + 0x30;
    }

    struct ProducerArgs * prodArgs = (ProducerArgs*)malloc(sizeof(ProducerArgs));
    prodArgs->source = testData;
    prodArgs->pArray = sharedArray;
    prodArgs->count = 8;

    struct ConsumerArgs * consArgs = (ConsumerArgs*)malloc(sizeof(ConsumerArgs));
    consArgs->count = 8;
    consArgs->pArray = sharedArray;

    if(pthread_create( &consumer_thread, NULL, test_consumer_funct, (void*) consArgs) != 0)
    {
        printf("Failed to start producer thread\n");
        return -1;
    }
    if(pthread_create( &producer_thread, NULL, test_producer_funct, (void*) prodArgs) != 0)
    {
        printf("Failed to start producer thread\n");
        return -1;
    }

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    free(testData);
    free(prodArgs);
    free(consArgs);
    array_free(sharedArray);
    free(sharedArray);

    return 1;
}

char test_multthread_test()
{
    printf("Starting multi thread test\n");

    pthread_t producerThreads[8];
    pthread_t consumerThreads[8];

    struct ProducerArgs * prodArgs[8];
    struct ConsumerArgs * consArgs[8];
    
    Array * sharedArray = (Array*)malloc(sizeof(Array));    

    if(array_init(sharedArray) != 0)
    {
        printf("Failed to initialize array\n");
        return -1;
    }

    printf("Before build producers\n");

    char testMessage[] = {'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A', ':', ' ', 0, '_', 0, 0};
    char ** testData;
    // Build all producer threads
    for (char i = 0; i < 8; i++)
    {
        //printf("%i\n", i);
        prodArgs[i] = (ProducerArgs *)malloc(sizeof(ProducerArgs));
        //printf("A\n");
        testData = (char**)malloc(sizeof(char*)*8);
        //printf("B\n");
        testMessage[11] = i + 0x30;
        //printf("C\n");
        for(char j = 0; j < 8; j++)
        {
            testData[j] = (char*)malloc(MAX_NAME_LENGTH);
            m_memset(testData[j], 0, MAX_NAME_LENGTH);
            m_memcpy(testData[j], testMessage, 14);
            testData[j][13] = j + 0x30;
            //printf("Producer %i str %i: %s\n", i, j, testData[j]);
        }
        //printf("D\n");
        prodArgs[i]->source = testData;
        //printf("E\n");
        prodArgs[i]->pArray = sharedArray;
        //printf("F\n");
        prodArgs[i]->count = 8;
        //printf("G\n");
    }

    printf("Before build consumers\n");

    // Build all consumer threads
    for (char i = 0; i < 8; i++)
    {
        //printf("%i\n", i);
        consArgs[i] = (ConsumerArgs*)malloc(sizeof(ConsumerArgs));
        consArgs[i]->count = 8;
        consArgs[i]->pArray = sharedArray;
    }

    printf("Starting threads\n");

    for(char i = 0; i < 8; i++)
    {
        if(pthread_create( &consumerThreads[i], NULL, test_consumer_funct, (void*) consArgs[i]) != 0)
        {
            printf("Failed to start producer thread\n");
            return -1;
        }
        if(pthread_create( &producerThreads[i], NULL, test_producer_funct, (void*) prodArgs[i]) != 0)
        {
            printf("Failed to start producer thread\n");
            return -1;
        }
    }

    printf("Waiting for threads to exit\n");

    for(char i = 0; i < 8; i++)
    {
        pthread_join(producerThreads[i], NULL);
        pthread_join(consumerThreads[i], NULL);

        printf("%i exited\n", i);
    }

    printf("Freeing test resources\n");
    
    for(char i = 0; i < 8; i++)
    {
        //printf("%i\n", i);
        free(prodArgs[i]->source);
        //printf("A\n");
        free(prodArgs[i]);
        //printf("B\n");
        free(consArgs[i]);
        //printf("C\n");
    }

    printf("Freeing array\n");

    array_free(sharedArray);
    free(sharedArray);

    return 1;
}