#ifndef STRING_H
#define STRING_H
#include "arena.h"
#include "array.h"
#include <stddef.h>

typedef ARRAY(char) String;

struct StringReturn {
    String string;
    int status;
};

char *getChar(String *string);

// this will create a string froma char *.
// This means the char * must
// have a lifetime as long as the string.
struct StringReturn getStringFromChar(char *string, size_t size,
                                      struct Arena *arena);

// this will create a string from a string from
// a char pointer. This means the char * must
// have a lifetime as long as the string.
struct StringReturn getStringFromString(String *string);

// copy the contents of the string. This is using fixed array size.
struct StringReturn copyStringFromChar(char *string, size_t size,
                                       struct Arena *arena);

struct StringReturn copyStringFromString(String *string);
#endif
