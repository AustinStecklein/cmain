#include <string.h> // for memcmp
#include "array.h"

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
