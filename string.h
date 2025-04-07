#include "array.h"
#include "debug.h"

typedef ARRAY(char) String;

char *getChar(String *string) {
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `getChar`");
        return NULL;
    }
    return (string->items);
}

// this will create a string froma char *.
// This means the char * must
// have a lifetime as long as the string.
String getStringFromChar(char *string, size_t size, struct Arena *arena) {
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `getStringFromChar`");
    }
    String newString = {string, size, size, arena};
    return newString;
}

// this will create a string from a string from
// a char pointer. This means the char * must
// have a lifetime as long as the string.
String getStringFromString(String *string) {
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `getStringFromString`");
    }
    String newString = {string->items, string->size, string->size,
                        string->arena};
    return newString;
}

// copy the contents of the string. This is using fixed array size.
String copyStringFromChar(char *string, size_t size, struct Arena *arena) {
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `copyStringFromChar`");
    }
    String newString = NEW_ARRAY();
    INIT_ARRAY(newString, arena);
    COPY_POINTER(string, size, newString);
    return newString;
}

String copyStringFromString(String *string) {
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `copyStringFromString`");
    }
    String newString = NEW_ARRAY();
    INIT_ARRAY(newString, string->arena);
    COPY(*string, newString);
    return newString;
}
