#include "Tests.h"

void m_memset(char * arr, char val, size_t len)
{
    while(len--)
    {
        *(arr++) = val;
    }
}

char test_array_runAll()
{
    char count = 0;

    if(test_array_initializeTest())
    {
        count++;
        printf("test_array_initializeTest passed\n");
    }

    if(test_array_put1())
    {
        count++;
        printf("test_array_put1 passed\n");
    }

    if(test_array_put2())
    {
        count++;
        printf("test_array_put2 passed\n");
    }

    if(test_array_put3())
    {
        count++;
        printf("test_array_put3 passed\n");
    }

    if(test_array_put4())
    {
        count++;
        printf("test_array_put4 passed\n");
    }

    if(test_array_get1())
    {
        count++;
        printf("test_array_get1 passed\n");
    }

    if(test_array_get2())
    {
        count++;
        printf("test_array_get2 passed\n");
    }

    if(test_array_get3())
    {
        count++;
        printf("test_array_get3 passed\n");
    }

    printf("Array tests: %i / %i passed\n", count, 8);

    return count == 5;
}

// This makes sure the array is correctly initialized
char test_array_initializeTest()
{
    printf("Starting array init test\n");

    Array * array = (Array*)malloc(sizeof(array));

    char ret = array_init(array);

    if(ret)
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

    return ret;
}

// Tests adding 1 string to the array
char test_array_put1()
{
    printf("Starting array put 1 test\n");

    Array * array = (Array *)malloc(sizeof(Array));

    char testArr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};

    if(!array_init(array))
        return 0;

    if(!array_put(array, testArr))
    {
        printf("Array put test 1 failed when calling funct\n");
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

    if(!array_init(array))
        return 0;

    for (char j = 1; j < 21; j++)
        source[j - 1] = j;

    // Check adding to array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        if(!array_put(array, source))
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

    if(!array_init(array))
        return 0;

    for (char j = 1; j < 21; j++)
        source[j - 1] = j;

    // Check adding to array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        if(!array_put(array, source))
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

    if(!array_init(array))
        return 0;

    for (char j = 1; j < 51; j++)
        source[j - 1] = j;

    // Check adding to array
    if(!array_put(array, source))
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

    if(!array_init(array))
        return 0;

    if(!array_put(array, source))
    {
        printf("Array get test 1 failed when calling put\n");
        array_free(array);
        free(array);
        free(dest);
        return 0;
    }

    if(!array_get(array, &dest))
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

    if(!array_init(array))
        return 0;

    m_memset(array->buffer, -1, (ARRAY_SIZE * MAX_NAME_LENGTH));

    // Completely fill the array
    for(char i = 0; i < ARRAY_SIZE; i++)
    {
        m_memset(source, i, 20);

        if(!array_put(array, source))
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
        if(!array_get(array, &source))
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

    if(!array_init(array))
        return 0;


    for(char i = 1; i < 21; i++)
        source[i-1] = i;

    if(!array_put(array, source))
    {
        printf("Array put test 3 failed when calling put\n");
        array_free(array);
        free(array);
        free(source);
        return 0;
    }

    m_memset(source, 0, 20);

    if(!array_get(array, &source))
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

    if(!array_put(array, source))
    {
        printf("Array put test 3 failed when calling put 2\n");
        array_free(array);
        free(array);
        free(source);
        return 0;
    }


    if(!array_get(array, &source))
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