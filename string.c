#include "array.h"
#include "unittest.h"
#include <stdio.h>

struct FixedString {
    FIXED_ARRAY(char) array;
};

char *getChar(struct FixedString string) { return (string.array.items); }

struct FixedString getStringFromChar(char *string, size_t size) {
    struct FixedString newString = {{string, size, size}};
    return newString;
}

struct FixedString getStringFromString(struct FixedString string) {
    struct FixedString newString = {
        {string.array.items, string.array.size, string.array.size}};
    return newString;
}

// copy the contents of the string. This is using fixed array size.
struct FixedString copyStringFromChar(char *string, size_t size) {
    struct FixedString newString = {NEW_FIXED_ARRAY()};
    INIT_FIXED_ARRAY(newString.array, size);
    memcpy(newString.array.items, string, size);
    return newString;
}

struct FixedString copyStringFromString(struct FixedString string) {
    struct FixedString newString = {NEW_FIXED_ARRAY()};
    INIT_FIXED_ARRAY(newString.array, string.array.size);
    memcpy(newString.array.items, string.array.items, string.array.size);
    return newString;
}

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
