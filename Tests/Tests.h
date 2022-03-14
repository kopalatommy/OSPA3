#ifndef _TESTS_H_
#define _TESTS_H_

#include <stdlib.h>
#include <stdio.h>

#include "../array.h"

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

char test_array_runAll();

// This makes sure the array is correctly initialized
char test_array_initializeTest();

// Tests adding 1 string to the array
char test_array_put1();

// Tests filling the array
char test_array_put2();

// Tests putting a string > MAX_NAME_LENGTH into the array
char test_array_put3();

// Tests writing > ARRAY_SIZE strings to the array
char test_array_put4();

// Tests getting 1 string from the array
char test_array_get1();

// Tests fully filling and then getting all strings in the array
char test_array_get2();

// Tests the operations: put -> put -> get -> get
char test_array_get3();

char test_thread_concurrent_get_put();

char test_multthread_test();

#endif // _TESTS_H_