#ifndef ARRAY_H
#define ARRAY_H

#include "arena.h"
#include "debug.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ArrayError {
    OK = 0,
    NULLPOINTER = 1,
    UNINITARRAY = 2,
    FAILEDALLOC = 3,
    INVALIDARGS = 4,
};

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
#define NEW_FIXED_ARRAY() {0, 0, 0}

#define INIT_FIXED_ARRAY(array, array_size, status)                            \
    do {                                                                       \
        (array).size = 0;                                                      \
        (array).items = malloc(sizeof(*(array).items) * (array_size));         \
        if ((array).items != NULL)                                             \
            (array).alloc = (array_size);                                      \
        else                                                                   \
            DEBUG_ERROR("Unable to malloc space for the array\n");             \
        (status) = FAILEDALLOC;                                                \
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
#define ARRAY_DEFINE(type, name)                                               \
    typedef struct {                                                           \
        (type) * items;                                                        \
        size_t size;                                                           \
        size_t alloc;                                                          \
        struct Arena *arena;                                                   \
    } name

// used to fill the array with empty references
#define NEW_ARRAY() {0, 0, 0, 0}
// used to set up the array
#define INIT_ARRAY(array, givenArena, status)                                  \
    do {                                                                       \
        if ((givenArena) == NULL) {                                            \
            DEBUG_ERROR("called INIT_ARRAY with either a null array or arena " \
                        "pointer");                                            \
            (status) = NULLPOINTER;                                            \
            break;                                                             \
        }                                                                      \
        (array).size = 0;                                                      \
        (array).items = NULL;                                                  \
        (array).alloc = 0;                                                     \
        (array).arena = givenArena;                                            \
        (status) = 0;                                                          \
    } while (0)

// Set size to zero which will do a lazy clear
#define CLEAR_ARRAY(array, status)                                             \
    do {                                                                       \
        if (!ARRAY_INITIALIZED(array)) {                                       \
            DEBUG_ERROR("called CLEAR_ARRAY with an unintialized array");      \
            (status) = UNINITARRAY;                                            \
            break;                                                             \
        }                                                                      \
        (array).size = 0;                                                      \
        (status) = 0;                                                          \
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
#define PUSH_ARRAY(array, item, status)                                        \
    do {                                                                       \
        if (!ARRAY_INITIALIZED(array)) {                                       \
            DEBUG_ERROR("called PUSH_ARRAY with an unintialized array");       \
            (status) = UNINITARRAY;                                            \
            break;                                                             \
        }                                                                      \
        if ((array).alloc == (array).size) {                                   \
            REALLOC_ARRAY(array, nextArrayAllocSize((array).size), status);    \
        }                                                                      \
        if ((status) == FAILEDALLOC) {                                         \
            DEBUG_ERROR("cannot add item to array");                           \
            break;                                                             \
        }                                                                      \
        (array).items[(array).size] = item;                                    \
        (array).size++;                                                        \
    } while (0)

// turns true if the array has been initialized
#define ARRAY_INITIALIZED(array) ((array).arena != NULL)

// since this will be with an arena vector there is no way to free the memory.
// This means that memory will be used until the arena is completely free. This
// is why these arrays should be small.
#define REALLOC_ARRAY(array, size, status)                                     \
    do {                                                                       \
        (array).items = reallocArray((array).arena, (array).items,             \
                                     (array).alloc * sizeof(*(array).items),   \
                                     (size) * sizeof(*(array).items));         \
        if ((array).items == NULL) {                                           \
            DEBUG_ERROR("REALLOC_ARRAY failed to realloc the array");          \
            (status) = FAILEDALLOC;                                            \
            break;                                                             \
        }                                                                      \
        (array).alloc = size;                                                  \
    } while (0)

static inline size_t nextArrayAllocSize(size_t currentlyAlloced) {
    if (currentlyAlloced != 0) {
        return currentlyAlloced * 2;
    }
    return 1;
}

#define COPY(array_src, array_dst, status)                                     \
    do {                                                                       \
        if (!ARRAY_INITIALIZED(array_src) || !ARRAY_INITIALIZED(array_dst)) {  \
            DEBUG_ERROR("called COPY with a null array pointer");              \
            (status) = NULLPOINTER;                                            \
            break;                                                             \
        }                                                                      \
        if ((array_dst).alloc < (array_src).size) {                            \
            (array_dst).items =                                                \
                mallocArena(&(array_dst).arena,                                \
                            (array_src).size * sizeof((array_dst).items));     \
            (array_dst).alloc = (array_src).size;                              \
        }                                                                      \
        if ((array_dst).items != NULL) {                                       \
            memmove((array_dst).items, (array_src).items,                      \
                    (array_src).size * sizeof((array_dst).items));             \
            (array_dst).size = (array_src).size;                               \
        }                                                                      \
        else {                                                                 \
            DEBUG_ERROR("array_dst is a null pointer\n");                      \
            (status) = FAILEDALLOC;                                            \
        }                                                                      \
    } while (0)

#define COPY_POINTER(src_pointer, src_size, array_dst, status)                 \
    do {                                                                       \
        if (!ARRAY_INITIALIZED(array_dst) || (src_pointer) == NULL) {          \
            DEBUG_ERROR("called COPY_POINTER with a null pointer");            \
            (status) = NULLPOINTER;                                            \
            break;                                                             \
        }                                                                      \
        if ((array_dst).alloc <= (src_size)) {                                 \
            (array_dst).items = mallocArena(                                   \
                &(array_dst).arena, (src_size) * sizeof((array_dst).items));   \
        }                                                                      \
        if ((array_dst).items != NULL) {                                       \
            (array_dst).size = src_size;                                       \
            memcpy((array_dst).items, src_pointer, src_size);                  \
        }                                                                      \
        else {                                                                 \
            DEBUG_ERROR("array_dst is a null pointer\n");                      \
            (status) = FAILEDALLOC;                                            \
        }                                                                      \
    } while (0)

static inline void *reallocArray(struct Arena *arena, void *oldPointer,
                                 size_t oldAlloc, size_t newAlloc) { // NOLINT
    if (arena == NULL) {
        DEBUG_ERROR("called reallocArray with a null arena pointer");
        return NULL;
    }
    if (oldAlloc != 0 && oldPointer == NULL) {
        DEBUG_ERROR("called reallocArray with a null array pointer");
        return NULL;
    }
    void *newPointer = mallocArena(&arena, newAlloc);
    if (newPointer == NULL) {
        DEBUG_ERROR("mallocArena failed to allocate memory\n");
        return NULL;
    }
    memcpy(newPointer, oldPointer, oldAlloc);
    return newPointer;
}
#endif
