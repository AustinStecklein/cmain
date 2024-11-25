#include "arena.h"
#include "array.h"
#include <stdio.h>

struct UnitTest {
	void (*function)();
	char * functionName;
	char passed;
};

struct Assert {
	char * assertName;
	char passed;
};

// Test state vars that the macros need access to
// This is not the safest way to do this but since it 
// is a unit test framework with defined rules I think it is fine
struct Arena * allocator = NULL;
ARRAY(struct UnitTest) testCollection = NEW_ARRAY();
ARRAY(struct Assert) assertCollection = NEW_ARRAY();


// The macros available for testing
#define ASSERT_TRUE(expression, testName) \
		do { \
			struct Assert newAssert_testName = {testName, (expression)}; \
		 	PUSH_ARRAY(assertCollection, newAssert_testName); \
		} while(0) \

#define ASSERT_FALSE(expression, testName) \
		do { \
			struct Assert newAssert = {testName, !(expression)}; \
		 	PUSH_ARRAY(assertCollection, newAssert); \
		} while(0) \

//#define ASSERT_EQUAL(expression, test) ()
//#define ASSERT_NOT_EQUAL(expression, test) ()

// setup functions
#define ADD_TEST(function) \
		do { \
			struct UnitTest newTest = {(function), "function", 0}; \
			PUSH_ARRAY(testCollection, newTest); \
			printf("the size tests: %d in the macro\n", (int)testCollection.size); \
		} while(0) \

void setUp(struct Arena * currentAllocator) {
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
	
	struct Arena * memory = createArena(DEFAULT_SIZE);
	setUp(memory);
	ADD_TEST(firstTest);
	ADD_TEST(secondTest);
	printf("about to loop through the lists\n");
	printf("the size tests: %d\n", (int)testCollection.size);
	// run through all of the tests and then check if any asserts are fired during the test
	for (int i = 0; i < testCollection.size; i++) {
		// start with clearing assert collection
		CLEAR_ARRAY(assertCollection);
		// the assert collection will be filled by the user defined test function through
		// the ASSERT_* macros
		printf("Running function %s\n", testCollection.items[i].functionName);
		(testCollection.items[i].function)();
		for (int j = 0; j < assertCollection.size; j++) {
			if (assertCollection.items[i].passed)
				printf("Assert %s has PASSED\n", testCollection.items[i].functionName);
			else
				printf("Assert %s has FAILED\n", testCollection.items[i].functionName);
		}
	}

	return 0;
}
