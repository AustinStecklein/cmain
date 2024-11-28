#include "unittest.h"
#include "string.h"


void testFixedStringChar() {
    struct FixedString string = getStringFromChar("test string", 12);
    ASSERT_TRUE(!memcmp(getChar(string), "test string", sizeof("test string")),
                "Check the initialization of the fixed string from char");

    struct FixedString secondString = copyStringFromChar(getChar(string), 12);
    ASSERT_TRUE(
        !memcmp(getChar(secondString), "test string", sizeof("test string")),
        "Check the copy the fixed string from char");
    secondString.array.items[0] = 'T';
    ASSERT_FALSE(
        !memcmp(getChar(secondString), "test string", sizeof("Test string")),
        "Check the copy changes");
    ASSERT_TRUE(!memcmp(getChar(string), "test string", sizeof("Test string")),
                "Check the original doesn't change");
}

void testFixedString() {
    struct FixedString string = getStringFromChar("test string", 12);
    ASSERT_TRUE(!memcmp(getChar(getStringFromString(string)), "test string",
                        sizeof("test string")),
                "Check the initialization of the fixed string from char");

    struct FixedString secondString = copyStringFromString(string);
    ASSERT_TRUE(
        !memcmp(getChar(secondString), "test string", sizeof("test string")),
        "Check the copy the fixed string from char");
    secondString.array.items[0] = 'T';
    ASSERT_FALSE(
        !memcmp(getChar(secondString), "test string", sizeof("Test string")),
        "Check the copy changes");
    ASSERT_TRUE(!memcmp(getChar(string), "test string", sizeof("Test string")),
                "Check the original doesn't change");
}


int main() {
    struct Arena *memory = createArena(DEFAULT_SIZE);
    setUp(memory);
    ADD_TEST(testFixedStringChar);
    ADD_TEST(testFixedString);
    runTest();
    return 0;
}
