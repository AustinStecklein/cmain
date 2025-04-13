#include "unittest.h"

void firstTest(struct Arena *arena) {
    ASSERT_TRUE(1, "test true");
    ASSERT_FALSE(0, "test false");
    return;
}

void secondTest(struct Arena *arena) {
    ASSERT_TRUE(0, "test true");
    ASSERT_FALSE(1, "test false");
    return;
}

void thirdTest(struct Arena *arena) {
    ASSERT_TRUE(1, "test true");
    ASSERT_FALSE(1, "test false");
    return;
}

int main() {
    struct Arena *memory = createArena();
    int status = 0;
    status = setUp(memory);
    if (status != 0) {
        printf("Failed to setup the test\n");
        return status;
    }
    ADD_TEST(firstTest);
    ADD_TEST(secondTest);
    ADD_TEST(thirdTest);
    return runTest();
}
