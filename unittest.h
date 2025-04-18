#pragma once
#include "arena.h"
#include "array.h"
#include "debug.h"
#include <stdio.h>

struct UnitTest {
    void (*function)(struct Arena *);
    char *functionName;
    char passed;
};

struct Assert {
    char *assertName;
    char passed;
};

// Test state vars that the macros need access to
// This is not the safest way to do this but since it
// is a unit test framework with defined rules I think it is fine
struct Arena *allocator = NULL;
ARRAY(struct UnitTest) testCollection = NEW_ARRAY();
ARRAY(struct Assert) assertCollection = NEW_ARRAY();

int setupFailed = 0;

// The macros available for testing
#define ASSERT_TRUE(expression, testName)                                      \
    do {                                                                       \
        int status = 0;                                                        \
        struct Assert newAssert_testName = {testName, (expression)};           \
        PUSH_ARRAY(assertCollection, newAssert_testName, status);              \
        if (status != OK) {                                                    \
            DEBUG_ERROR("`ASSERT_TRUE` failed to add assertion");              \
            setupFailed = 1;                                                   \
        }                                                                      \
    } while (0)

#define ASSERT_FALSE(expression, testName)                                     \
    do {                                                                       \
        int status = 0;                                                        \
        struct Assert newAssert = {testName, !(expression)};                   \
        PUSH_ARRAY(assertCollection, newAssert, status);                       \
        if (status != OK) {                                                    \
            DEBUG_ERROR("`ASSERT_FALSE` failed to add assertion");             \
            setupFailed = 1;                                                   \
        }                                                                      \
    } while (0)

// setup functions
#define ADD_TEST(function)                                                     \
    do {                                                                       \
        int status = 0;                                                        \
        struct UnitTest newTest = {(function), #function, 0};                  \
        PUSH_ARRAY(testCollection, newTest, status);                           \
        if (status != OK) {                                                    \
            DEBUG_ERROR("`ADD_TEST` failed to add assertion");                 \
            setupFailed = 1;                                                   \
        }                                                                      \
    } while (0)

int setUp(struct Arena *currentAllocator) {
    allocator = currentAllocator;
    int status = 0;
    INIT_ARRAY(testCollection, allocator, status);
    return status;
}

int runTest() {
    if (!ARRAY_INITIALIZED(testCollection) || allocator == NULL) {
        printf(
            "The unit test file must call 'setUp' before calling 'runTest'\n");
        return -1;
    }
    int passedTestCount = 0;
    // run through all of the tests and then check if any asserts are fired
    // during the test
    for (int i = 0; i < testCollection.size; i++) {
        void *testStartingPoint = startScratchPad(allocator);
        int status = 0;
        // start with clearing assert collection
        INIT_ARRAY(assertCollection, allocator, status);
        if (status != OK) {
            printf("\e[1;31m FAILED:\e[0m Unable to kick off the test");
            break;
        }
        // the assert collection will be filled by the user defined test
        // function through the ASSERT_* macros
        char allTestsPassed = 1;
        (testCollection.items[i].function)(allocator);
        printf("%s: ", testCollection.items[i].functionName);
        for (int j = 0; j < assertCollection.size; j++) {
            if (!assertCollection.items[j].passed)
                allTestsPassed = 0;
        }
        if (allTestsPassed) {
            printf("\e[1;32m PASSED\e[0m\n");
            passedTestCount++;
        } else {
            // only going to reloop if there has been a single failure
            printf("\e[1;31m FAILED\e[0m\n");
            for (int j = 0; j < assertCollection.size; j++) {
                if (assertCollection.items[j].passed)
                    printf("\tAssert %s has \e[1;32mPASSED\e[0m\n",
                           assertCollection.items[j].assertName);
                else
                    printf("\tAssert %s has \e[1;31mFAILED\e[0m\n",
                           assertCollection.items[j].assertName);
            }
        }
        FREE_ARRAY(assertCollection);
        restoreSratchPad(&allocator, testStartingPoint);
    }
    printf("%d test(s) passed out of %d\n", passedTestCount,
           (int)testCollection.size);
    burnItDown(&testCollection.arena);
    return passedTestCount == (int)testCollection.size;
}
