#include <stdio.h>

#include "array.h"

#define TEST_BUILD 1

#if TEST_BUILD == 1
#include "Tests/Tests.h"
#endif

int main()
{
    printf("Hello\n");

#if TEST_BUILD == 1
    test_array_runAll();
#endif

    Array * sharedArray = (Array*)malloc(sizeof(Array));

    if(!array_init(sharedArray))
    {
        printf("Failed to initialize array\n");
        return 1;
    }

    array_free(sharedArray);

    return 0;
}