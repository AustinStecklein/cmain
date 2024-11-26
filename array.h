#pragma once
#include "arena.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// fixed arrays that don't make sense to be allocated in an arena due to either
// their lifetime or their size. This makes them super simple compared to the
// dynamic array
#define FIXED_ARRAY(type)                                                      \
    struct {                                                                   \
        type *items;                                                           \
        size_t size;                                                           \
        size_t alloc;                                                          \
    }
// used to fill the array with empty references
#define NEW_FIXED_ARRAY()                                                      \
    { 0, 0, 0 }

#define INIT_FIXED_ARRAY(array, array_size)                                    \
    do {                                                                       \
        (array).size = 0;                                                      \
        (array).items = malloc(sizeof(*(array).items) * (array_size));         \
        (array).alloc = (array_size);                                          \
    } while (0)

#define FREE_FIXED_ARRAY(array)                                                \
    do {                                                                       \
        free((array).items);                                                   \
        (array).size = 0;                                                      \
        (array).alloc = 0;                                                     \
        (array).items = NULL;                                                  \
    } while (0)

#define PUSH_FIXED_ARRAY(array, item)                                          \
    do {                                                                       \
        if ((array).alloc > (array).size) {                                    \
            (array).items[(array).size] = item;                                \
            (array).size++;                                                    \
        }                                                                      \
    } while (0)

// dynamically sized array with arenas
// dynamic array struct type
#define ARRAY(type)                                                            \
    struct {                                                                   \
        type *items;                                                           \
        size_t size;                                                           \
        size_t alloc;                                                          \
        struct Arena *arena;                                                   \
    }

// used to fill the array with empty references
#define NEW_ARRAY()                                                            \
    { 0, 0, 0, 0 }
// used to set up the array
#define INIT_ARRAY(array, givenArena)                                          \
    do {                                                                       \
        (array).size = 0;                                                      \
        (array).items = NULL;                                                  \
        (array).alloc = 0;                                                     \
        (array).arena = givenArena;                                            \
    } while (0)

// Set size to zero which will do a lazy clear
#define CLEAR_ARRAY(array)                                                     \
    do {                                                                       \
        (array).size = 0;                                                      \
    } while (0)

// free the array. Since this uses an arena the array is not in charge of the
// memory so just set everything back to the default.
#define FREE_ARRAY(array)                                                      \
    do {                                                                       \
        (array).size = 0;                                                      \
        (array).alloc = 0;                                                     \
        (array).items = NULL;                                                  \
    } while (0)

// push an item to the array. If the size needs to increased attempt to
// get more memory from the arena
#define PUSH_ARRAY(array, item)                                                \
    do {                                                                       \
        if ((array).alloc == (array).size)                                     \
            REALLOC_ARRAY(array);                                              \
        (array).items[(array).size] = item;                                    \
        (array).size++;                                                        \
    } while (0)

// turns true if the array has been initialized
#define ARRAY_INITIALIZED(array) ((array).arena != NULL)

// since this will be with an arena vector there is no way to free the memory.
// This means that memory will be used until the arena is completely free. This
// is why these arrays should be small.
#define REALLOC_ARRAY(array)                                                   \
    do {                                                                       \
        (array).items = reallocArray((array).arena, (array).items,             \
                                     (array).alloc * sizeof(*(array).items),   \
                                     nextArrayAllocSize((array).alloc) *       \
                                         sizeof(*(array).items));              \
        (array).alloc = nextArrayAllocSize((array).alloc);                     \
    } while (0)

static inline size_t nextArrayAllocSize(size_t currentlyAlloced) {
    if (currentlyAlloced != 0) {
        return currentlyAlloced * 2;
    }
    return 1;
}

static inline void *reallocArray(struct Arena *arena, void *oldPointer,
                                 size_t oldAlloc, size_t newAlloc) {
    void *newPointer = zmallocArena(&arena, newAlloc);
    memcpy(newPointer, oldPointer, oldAlloc);
    return newPointer;
}
