#include "array.h"
#include "debug.h"

typedef ARRAY(char) String;

struct StringReturn {
    String string;
    int status;
};

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
struct StringReturn getStringFromChar(char *string, size_t size,
                                      struct Arena *arena) {
    struct StringReturn returnValue = {{string, size, size, arena}, 0};
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `getStringFromChar`");
        returnValue.status = 1;
        return returnValue;
    }
    return returnValue;
}

// this will create a string from a string from
// a char pointer. This means the char * must
// have a lifetime as long as the string.
struct StringReturn getStringFromString(String *string) {
    struct StringReturn returnValue = {
        {string->items, string->size, string->size, string->arena}, 0};
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `getStringFromString`");
        returnValue.status = 1;
        return returnValue;
    }
    return returnValue;
}

// copy the contents of the string. This is using fixed array size.
struct StringReturn copyStringFromChar(char *string, size_t size,
                                       struct Arena *arena) {
    struct StringReturn returnValue = {NEW_ARRAY(), 0};
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `copyStringFromChar`");
        returnValue.status = 1;
        return returnValue;
    }
    int status = 0;
    INIT_ARRAY(returnValue.string, arena, status);
    if (status != OK) {
        returnValue.status = status;
        return returnValue;
    }
    COPY_POINTER(string, size, returnValue.string, status);
    if (status != OK) {
        returnValue.status = status;
        return returnValue;
    }
    return returnValue;
}

struct StringReturn copyStringFromString(String *string) {
    struct StringReturn returnValue = {NEW_ARRAY(), 0};
    if (string == NULL) {
        DEBUG_ERROR("NUll pointer has passed to `copyStringFromString`");
        returnValue.status = 1;
        return returnValue;
    }
    int status = 0;
    INIT_ARRAY(returnValue.string, string->arena, status);
    if (status != OK) {
        returnValue.status = status;
        return returnValue;
    }
    COPY(*string, returnValue.string, status);
    if (status != OK) {
        returnValue.status = status;
        return returnValue;
    }
    return returnValue;
}
