#include "test_buffer.h"
#include "unittest.h"

void testBuffer(struct Arena *arrayArena) {
    BUFFER(int) collection = NEW_BUFFER();
    int status = 0;
    INIT_BUFFER(collection, arrayArena, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 9, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 2, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection.array.items[0] == 5, "check first item");
    ASSERT_TRUE(collection.array.items[1] == 7, "check second item");
    ASSERT_TRUE(collection.array.items[2] == 9, "check third item");
    ASSERT_TRUE(collection.array.items[3] == 1, "check fourth item");
    ASSERT_TRUE(collection.array.items[4] == 2, "check fifth item");
    ASSERT_TRUE(collection.array.size == 5, "check size");
    ASSERT_TRUE(collection.array.alloc == 5, "check alloc'ed size");
}

void testWrapBuffer(struct Arena *arrayArena) {
    BUFFER(int) collection = NEW_BUFFER();
    int status = 0;
    INIT_BUFFER(collection, arrayArena, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 9, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 2, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 3, status);
    ASSERT_TRUE(status == OK, "status check");
    ASSERT_TRUE(collection.array.items[0] == 3, "check first item");
    ASSERT_TRUE(collection.array.items[1] == 7, "check second item");
    ASSERT_TRUE(collection.array.items[2] == 9, "check third item");
    ASSERT_TRUE(collection.array.items[3] == 1, "check fourth item");
    ASSERT_TRUE(collection.array.items[4] == 2, "check fifth item");
    ASSERT_TRUE(collection.array.size == 5, "check size");
    ASSERT_TRUE(collection.array.alloc == 5, "check alloc'ed size");
}

void testWrapBufferWithGet(struct Arena *arrayArena) {
    BUFFER(int) collection = NEW_BUFFER();
    int status = 0;
    INIT_BUFFER(collection, arrayArena, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 5, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 7, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 9, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 1, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 2, status);
    ASSERT_TRUE(status == OK, "status check");
    PUSH_BUFFER(collection, 3, status);
    ASSERT_TRUE(status == OK, "status check");
    int item = 0;
    GET_ITEM(collection, 0, &item, status);
    ASSERT_TRUE(item == 7, "check first item");
    GET_ITEM(collection, 1, &item, status);
    ASSERT_TRUE(item == 9, "check second item");
    GET_ITEM(collection, 2, &item, status);
    ASSERT_TRUE(item == 1, "check third item");
    GET_ITEM(collection, 3, &item, status);
    ASSERT_TRUE(item == 2, "check fourth item");
    GET_ITEM(collection, 4, &item, status);
    ASSERT_TRUE(item == 3, "check fifth item");
    ASSERT_TRUE(collection.array.size == 5, "check size");
    ASSERT_TRUE(collection.array.alloc == 5, "check alloc'ed size");
}

int runBufferTests() {
    struct Arena *memory = createArena();
    int status = 0;
    status = setUp(memory);
    if (status != 0) {
        printf("Failed to setup the test\n");
        return status;
    }
    ADD_TEST(testBuffer);
    ADD_TEST(testWrapBuffer);
    ADD_TEST(testWrapBufferWithGet);
    return runTest();
}
