#include "string.h"
#include "unittest.h"

void testStringChar(struct Arena *arena) {
    String string = getStringFromChar("test string", 12, arena);
    ASSERT_TRUE(!memcmp(getChar(&string), "test string", sizeof("test string")),
                "Check the initialization of the fixed string from char");

    String secondString = copyStringFromChar(getChar(&string), 12, arena);
    ASSERT_TRUE(
        !memcmp(getChar(&secondString), "test string", sizeof("test string")),
        "Check the copy the fixed string from char");
    secondString.items[0] = 'T';
    ASSERT_FALSE(
        !memcmp(getChar(&secondString), "test string", sizeof("Test string")),
        "Check the copy changes");
    ASSERT_TRUE(!memcmp(getChar(&string), "test string", sizeof("Test string")),
                "Check the original doesn't change");
}

void testString(struct Arena *arena) {
    String string = getStringFromChar("test string", 12, arena);
    String string_cpy = getStringFromString(&string);
    ASSERT_TRUE(
        !memcmp(getChar(&string_cpy), "test string", sizeof("test string")),
        "Check the initialization of the fixed string from char");

    String secondString = copyStringFromString(&string);
    ASSERT_TRUE(
        !memcmp(getChar(&secondString), "test string", sizeof("test string")),
        "Check the copy the fixed string from char");
    secondString.items[0] = 'T';
    ASSERT_FALSE(
        !memcmp(getChar(&secondString), "test string", sizeof("Test string")),
        "Check the copy changes");
    ASSERT_TRUE(!memcmp(getChar(&string), "test string", sizeof("Test string")),
                "Check the original doesn't change");
}

int main() {
    struct Arena *memory = createArena();
    setUp(memory);
    ADD_TEST(testStringChar);
    ADD_TEST(testString);
    runTest();
    return 0;
}
