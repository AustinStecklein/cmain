#include "test_array.h"

void testDynamicArray(struct Arena *arrayArena) {
    ARRAY(int) collection = NEW_ARRAY();
    int status = 0;
    INIT_ARRAY(collection, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 9, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 2, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 3, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection.items[0] == 5, "check first item");
    ASSERT_TRUE(collection.items[1] == 7, "check second item");
    ASSERT_TRUE(collection.items[2] == 9, "check third item");
    ASSERT_TRUE(collection.items[3] == 1, "check fourth item");
    ASSERT_TRUE(collection.items[4] == 2, "check fifth item");
    ASSERT_TRUE(collection.items[5] == 3, "check sixth item");
    ASSERT_TRUE(collection.size == 6, "check size");
    ASSERT_TRUE(collection.alloc == 8, "check alloc'ed size");
}

void testStaticArray(struct Arena *arrayArena) {
    FIXED_ARRAY(float) collection = NEW_FIXED_ARRAY();
    int status = 0;
    INIT_FIXED_ARRAY(collection, 1024, status);
    ASSERT_TRUE(status == OK, "status check");
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
    int status = 0;
    INIT_ARRAY(collection, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 9, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection, 2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection.size == 5,
                "Check that the initial size is expected");
    CLEAR_ARRAY(collection, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection.size == 0, "Check that clear worked");
}

void testCheckInitializedArray(struct Arena *arrayArena) {
    ARRAY(int) collection = NEW_ARRAY();
    ASSERT_FALSE((ARRAY_INITIALIZED(collection)),
                 "Check that array is not showing as initialized");
    int status = 0;
    INIT_ARRAY(collection, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE((ARRAY_INITIALIZED(collection)),
                "Check that array is showing as initialized");
}

void testCopy(struct Arena *arrayArena) {
    ARRAY(int) collection_1 = NEW_ARRAY();
    int status = 0;
    INIT_ARRAY(collection_1, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 9, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection_1.size == 5,
                "Check that the initial size is expected");
    ARRAY(int) collection_2 = NEW_ARRAY();
    INIT_ARRAY(collection_2, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    COPY(collection_1, collection_2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection_1.size == collection_2.size,
                "Check that both collections are the same size now");
    ASSERT_TRUE(collection_1.items[0] == collection_2.items[0],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1.items[1] == collection_2.items[1],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1.items[2] == collection_2.items[2],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1.items[3] == collection_2.items[3],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1.items[4] == collection_2.items[4],
                "Check that both collections have the same values");
}

void testCopyPointer(struct Arena *arrayArena) {
    char collection_1[5] = {'a', 'k', 's', 'q', 'i'};
    ARRAY(char) collection_2 = NEW_ARRAY();
    int status = 0;
    INIT_ARRAY(collection_2, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    COPY_POINTER(collection_1, 5, collection_2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection_1[0] == collection_2.items[0],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1[1] == collection_2.items[1],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1[2] == collection_2.items[2],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1[3] == collection_2.items[3],
                "Check that both collections have the same values");
    ASSERT_TRUE(collection_1[4] == collection_2.items[4],
                "Check that both collections have the same values");
}

void testFaults(struct Arena *arrayArena) {
    DEBUG_PRINT("`testFaults` will trigger many Error prints. As long as "
                "there is not seg faults this is expected");
    ARRAY(int) collection_1 = NEW_ARRAY();
    int status = 0;
    INIT_ARRAY(collection_1, NULL, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_FALSE(ARRAY_INITIALIZED(collection_1),
                 "Check that collection is still not initialized");
    // No aserts here but checking that the program does not seg fault
    CLEAR_ARRAY(collection_1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 5, status);
    ASSERT_TRUE(status == OK, "status check");

    ARRAY(int) collection_2 = NEW_ARRAY();
    COPY(collection_1, collection_2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_FALSE(ARRAY_INITIALIZED(collection_2),
                 "Check that collection is still not initialized");

    INIT_ARRAY(collection_1, arrayArena, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_ARRAY(collection_1, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    COPY(collection_1, collection_2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_FALSE(ARRAY_INITIALIZED(collection_2),
                 "Check that collection is still not initialized");

    char collection_4[5] = {'a', 'k', 's', 'q', 'i'};
    COPY_POINTER(&collection_4, 5, collection_2, status);
    ASSERT_TRUE(status == OK, "status check");
}

int runArrayTests() {
    struct Arena *memory = createArena();
    int status = 0;
    status = setUp(memory);
    if (status != 0) {
        printf("Failed to setup the test\n");
        return status;
    }
    ADD_TEST(testDynamicArray);
    ADD_TEST(testStaticArray);
    ADD_TEST(testClearArray);
    ADD_TEST(testCheckInitializedArray);
    ADD_TEST(testCopy);
    ADD_TEST(testCopyPointer);
    ADD_TEST(testFaults);
    return runTest();
}
