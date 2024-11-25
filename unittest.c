#include "unittest.h"

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

void thirdTest() {
    ASSERT_TRUE(1, "test true");
    ASSERT_FALSE(1, "test false");
    return;
}

int main() {
    struct Arena *memory = createArena(DEFAULT_SIZE);
    setUp(memory);
    ADD_TEST(firstTest);
    ADD_TEST(secondTest);
    ADD_TEST(thirdTest);
    return runTest();
}
