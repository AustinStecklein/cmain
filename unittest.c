#include "arena.h"
#include "array.h"
#include <stdio.h>

struct UnitTest {
    void (*function)();
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

// The macros available for testing
#define ASSERT_TRUE(expression, testName)                                      \
    do {                                                                       \
        struct Assert newAssert_testName = {testName, (expression)};           \
        PUSH_ARRAY(assertCollection, newAssert_testName);                      \
    } while (0)

#define ASSERT_FALSE(expression, testName)                                     \
    do {                                                                       \
        struct Assert newAssert = {testName, !(expression)};                   \
        PUSH_ARRAY(assertCollection, newAssert);                               \
    } while (0)

//#define ASSERT_EQUAL(expression, test) ()
//#define ASSERT_NOT_EQUAL(expression, test) ()

// setup functions
#define ADD_TEST(function)                                                     \
    do {                                                                       \
        struct UnitTest newTest = {(function), #function, 0};                  \
        PUSH_ARRAY(testCollection, newTest);                                   \
    } while (0)

void setUp(struct Arena *currentAllocator) {
    allocator = currentAllocator;
    INIT_ARRAY(testCollection, allocator);
    INIT_ARRAY(assertCollection, allocator);
}

void firstTest() {
    ASSERT_TRUE(1, "test true");
    ASSERT_FALSE(0, "test false");
    return;
}

void secondTest() {
    ASSERT_TRUE(0, "test true");
    ASSERT_FALSE(1, "test false");
    return;
}

int main() {
    struct Arena *memory = createArena(DEFAULT_SIZE);
    setUp(memory);
    ADD_TEST(firstTest);
    ADD_TEST(secondTest);
    int passedTestCount = 0;
    // run through all of the tests and then check if any asserts are fired
    // during the test
    for (int i = 0; i < testCollection.size; i++) {
        // start with clearing assert collection
        CLEAR_ARRAY(assertCollection);
        // the assert collection will be filled by the user defined test
        // function through the ASSERT_* macros
        char allTestsPassed = 1;
        printf("%s: ", testCollection.items[i].functionName);
        (testCollection.items[i].function)();
        for (int j = 0; j < assertCollection.size; j++) {
            if (!assertCollection.items[i].passed)
                allTestsPassed = 0;
        }
        if (allTestsPassed) {
            printf("\e[1;32m PASSED\e[0m\n");
            passedTestCount++;
        } else {
            // only going to reloop if there has been a single failure
            printf("\e[1;31m FAILED\e[0m\n");
            for (int j = 0; j < assertCollection.size; j++) {
                if (assertCollection.items[i].passed)
                    printf("\tAssert %s has \e[1;32mPASSED\e[0m\n",
                           assertCollection.items[i].assertName);
                else
                    printf("\tAssert %s has \e[1;31mFAILED\e[0m\n",
                           assertCollection.items[i].assertName);
            }
        }
    }
    printf("%d test(s) passed out of %d\n", passedTestCount,
           (int)testCollection.size);
    return 0;
}
