#include "array.h"
#include <string.h> // for memcpy

typedef FIXED_ARRAY(char) FixedString;

char *getChar(FixedString string) { return (string.items); }

FixedString getStringFromChar(char *string, size_t size) {
    FixedString newString = {string, size, size};
    return newString;
}

FixedString getStringFromString(FixedString string) {
    FixedString newString = {string.items, string.size, string.size};
    return newString;
}

// copy the contents of the string. This is using fixed array size.
FixedString copyStringFromChar(char *string, size_t size) {
    FixedString newString = NEW_FIXED_ARRAY();
    INIT_FIXED_ARRAY(newString, size);
    memcpy(newString.items, string, size);
    return newString;
}

FixedString copyStringFromString(FixedString string) {
    FixedString newString = NEW_FIXED_ARRAY();
    INIT_FIXED_ARRAY(newString, string.size);
    memcpy(newString.items, string.items, string.size);
    return newString;
}
