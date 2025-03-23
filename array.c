#include "array.h"
#include "unittest.h"
#include <stdio.h>

void testDynamicArray(struct Arena *arrayArena) {
    ARRAY(int) collection = NEW_ARRAY();
    INIT_ARRAY(collection, arrayArena);
    PUSH_ARRAY(collection, 5);
    PUSH_ARRAY(collection, 7);
    PUSH_ARRAY(collection, 9);
    PUSH_ARRAY(collection, 1);
    PUSH_ARRAY(collection, 2);
    PUSH_ARRAY(collection, 3);
    ASSERT_TRUE(collection.items[0] == 5, "check first item");
    ASSERT_TRUE(collection.items[1] == 7, "check second item");
    ASSERT_TRUE(collection.items[2] == 9, "check third item");
    ASSERT_TRUE(collection.items[3] == 1, "check fourth item");
    ASSERT_TRUE(collection.items[4] == 2, "check fifth item");
    ASSERT_TRUE(collection.items[5] == 3, "check sixth item");
    ASSERT_TRUE(collection.size == 6, "check size");
    ASSERT_TRUE(collection.alloc == 8, "check alloc'ed size");
}

void testStaticArray(struct Arena*) {
    FIXED_ARRAY(float) collection = NEW_FIXED_ARRAY();
    INIT_FIXED_ARRAY(collection, 1024);
    PUSH_FIXED_ARRAY(collection, 1.0f);
    PUSH_FIXED_ARRAY(collection, 5.0f);
    ASSERT_TRUE(collection.items[0] == 1.0f, "check first item");
    ASSERT_TRUE(collection.items[1] == 5.0f, "check second item");
    ASSERT_TRUE(collection.size == 2, "check size");
    ASSERT_TRUE(collection.alloc == 1024, "check alloc'ed size");
    FREE_FIXED_ARRAY(collection);
}

void testClearArray(struct Arena *arrayArena) {
    ARRAY(int) collection = NEW_ARRAY();
    INIT_ARRAY(collection, arrayArena);
    PUSH_ARRAY(collection, 5);
    PUSH_ARRAY(collection, 7);
    PUSH_ARRAY(collection, 9);
    PUSH_ARRAY(collection, 1);
    PUSH_ARRAY(collection, 2);
    ASSERT_TRUE(collection.size == 5,
                "Check that the initial size is expected");
    CLEAR_ARRAY(collection);
    ASSERT_TRUE(collection.size == 0, "Check that clear worked");
}

void testCheckInitializedArray(struct Arena *arrayArena) {
    ARRAY(int) collection = NEW_ARRAY();
    ASSERT_FALSE((ARRAY_INITIALIZED(collection)),
                 "Check that array is not showing as initialized");
    INIT_ARRAY(collection, arrayArena);
    ASSERT_TRUE((ARRAY_INITIALIZED(collection)),
                "Check that array is showing as initialized");
}

int main() {
    struct Arena *memory = createArena();
    setUp(memory);
    ADD_TEST(testDynamicArray);
    ADD_TEST(testStaticArray);
    ADD_TEST(testClearArray);
    ADD_TEST(testCheckInitializedArray);
    return runTest();
}
