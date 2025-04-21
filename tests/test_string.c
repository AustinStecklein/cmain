#include "test_string.h"

void testStringChar(struct Arena *arena) {
    struct StringReturn stringReturn =
        getStringFromChar("test string", 12, arena);
    if (stringReturn.status != 0) {
        ASSERT_TRUE(0, "`getStringFromChar` failed fatally");
        return;
    }
    String string = stringReturn.string;
    ASSERT_TRUE(!memcmp(getChar(&string), "test string", sizeof("test string")),
                "Check the initialization of the fixed string from char");

    struct StringReturn secondStringReturn =
        copyStringFromChar(getChar(&string), 12, arena);
    if (secondStringReturn.status != 0) {
        ASSERT_TRUE(0, "`getStringFromChar` failed fatally");
        return;
    }
    String secondString = secondStringReturn.string;
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
    struct StringReturn stringReturn =
        getStringFromChar("test string", 12, arena);
    if (stringReturn.status != 0) {
        ASSERT_TRUE(0, "`getStringFromChar` failed fatally");
        return;
    }
    String string = stringReturn.string;

    struct StringReturn string_cpy_return = getStringFromString(&string);
    if (string_cpy_return.status != 0) {
        ASSERT_TRUE(0, "`getStringFromString` failed fatally");
        return;
    }

    String string_cpy = string_cpy_return.string;
    ASSERT_TRUE(
        !memcmp(getChar(&string_cpy), "test string", sizeof("test string")),
        "Check the initialization of the fixed string from char");

    struct StringReturn secondStringReturn = copyStringFromString(&string);
    if (secondStringReturn.status != 0) {
        ASSERT_TRUE(0, "`getStringFromString` failed fatally");
        return;
    }
    String secondString = secondStringReturn.string;
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

int runStringTests() {
    struct Arena *memory = createArena();
    int status = 0;
    status = setUp(memory);
    if (status != 0) {
        printf("Failed to setup the test\n");
        return status;
    }
    ADD_TEST(testStringChar);
    ADD_TEST(testString);
    runTest();
    return 0;
}
