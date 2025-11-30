#ifndef TEST_UNITTEST_H
#define TEST_UNITTEST_H

#include "../arena.h"
#include "../array.h"
#include "../debug.h"
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
typedef ARRAY(struct UnitTest) UnitestList;
typedef ARRAY(struct Assert) AssertList;
extern struct Arena *allocator;
extern UnitestList testCollection;
extern AssertList assertCollection;

extern int setupFailed;

// The macros available for testing
#define ASSERT_TRUE(expression, testName)                                      \
    do {                                                                       \
        int unittest_status = 0;                                               \
        struct Assert newAssert_testName = {testName, (expression)};           \
        PUSH_ARRAY(assertCollection, newAssert_testName, unittest_status);     \
        if (unittest_status != OK)                                             \
            DEBUG_ERROR("`ASSERT_TRUE` failed to add assertion");              \
    } while (0)

#define ASSERT_FALSE(expression, testName)                                     \
    do {                                                                       \
        int unittest_status = 0;                                               \
        struct Assert newAssert = {testName, !(expression)};                   \
        PUSH_ARRAY(assertCollection, newAssert, unittest_status);              \
        if (unittest_status != OK)                                             \
            DEBUG_ERROR("`ASSERT_FALSE` failed to add assertion");             \
    } while (0)

// setup functions
#define ADD_TEST(function)                                                     \
    do {                                                                       \
        int unittest_status = 0;                                               \
        struct UnitTest newTest = {(function), #function, 0};                  \
        PUSH_ARRAY(testCollection, newTest, unittest_status);                  \
        if (unittest_status != OK)                                             \
            DEBUG_ERROR("`ADD_TEST` failed to add assertion");                 \
    } while (0)

int setUp(struct Arena *currentAllocator);
int runTest(void);

#endif
