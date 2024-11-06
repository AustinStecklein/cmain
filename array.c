#include "array.h"
#include <stdio.h>

// Throwing unit tests in the c file for now. Not sure how I feel about it but
// here we are.
//
// UNIT TESTS
//
//
#define ASSERT_TRUE(expression, status) ((expression) ? status : 1)

int testDynamicArray() {
    int status = 0;
    struct Arena *arrayArena = createArena(DEFAULT_SIZE);
    ARRAY(int) collection = NEW_ARRAY();
    INIT_ARRAY(collection, arrayArena);
    PUSH_ARRAY(collection, 5);
    PUSH_ARRAY(collection, 7);
    PUSH_ARRAY(collection, 9);
    PUSH_ARRAY(collection, 1);
    PUSH_ARRAY(collection, 2);
    PUSH_ARRAY(collection, 3);
    status = ASSERT_TRUE(collection.items[0] == 5, status);
    status = ASSERT_TRUE(collection.items[1] == 7, status);
    status = ASSERT_TRUE(collection.items[2] == 9, status);
    status = ASSERT_TRUE(collection.items[3] == 1, status);
    status = ASSERT_TRUE(collection.items[4] == 2, status);
    status = ASSERT_TRUE(collection.items[5] == 3, status);
    status = ASSERT_TRUE(collection.size == 6, status);
    status = ASSERT_TRUE(collection.alloc == 8, status);

    // check clean up
    burnItDown(&arrayArena);
    status = ASSERT_TRUE(arrayArena == NULL, status);
    return status;
}

int testStaticArray() {
    int status = 0;
    FIXED_ARRAY(float) collection = NEW_FIXED_ARRAY();
    INIT_FIXED_ARRAY(collection, 1024);
    PUSH_FIXED_ARRAY(collection, 1.0f);
    PUSH_FIXED_ARRAY(collection, 5.0f);
    status = ASSERT_TRUE(collection.items[0] == 1.0f, status);
    status = ASSERT_TRUE(collection.items[1] == 5.0f, status);
    status = ASSERT_TRUE(collection.size == 2, status);
    status = ASSERT_TRUE(collection.alloc == 1024, status);
    FREE_FIXED_ARRAY(collection);

    return status;
}
int main() {

    int totalPassed = 0;
    int status = 0;
    status = testDynamicArray();
    totalPassed += status == 0 ? 1 : 0;
    printf("testDynamicArray: returned with %d\n", status);
    status = testStaticArray();
    totalPassed += status == 0 ? 1 : 0;
    printf("testStaticArray: returned with %d\n", status);
    printf("number of tests passed %d out of %d \n", totalPassed, 2);
    return 0;
}
