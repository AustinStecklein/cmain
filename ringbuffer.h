#ifndef BUFFER_H
#define BUFFER_H

#include "arena.h"
#include "array.h"
#include "debug.h"

#define BUFFER(type)                                                           \
    struct buffer {                                                            \
        ARRAY(type) array;                                                     \
        type *head;                                                            \
        type *tail;                                                            \
        uint32_t offset_index;                                                 \
    }

#define NEW_BUFFER() {NEW_ARRAY(), 0, 0}
// init a ring buffer. This is different from the array in that it wont play
// that nice with the arena allocator. This is just do to how reallocs have to
// happen. Due to this a size is required and larger buffers will not be
// allocated at this time. I might change my mind in the future but right now I
// am fine with fixed buffer sizes
#define INIT_BUFFER(buffer, arena, buffer_size, status)                        \
    do {                                                                       \
        if (arena == NULL) {                                                   \
            DEBUG_ERROR(                                                       \
                "called INIT_BUFFER with either a null array or arena "        \
                "pointer");                                                    \
            (status) = NULLPOINTER;                                            \
            break;                                                             \
        }                                                                      \
        if (buffer_size <= 1) {                                                \
            DEBUG_ERROR("called INIT_BUFFER with a size that is <= 1");        \
            (status) = INVALIDARGS;                                            \
            break;                                                             \
        }                                                                      \
        INIT_ARRAY((buffer).array, arena, status);                             \
        REALLOC_ARRAY((buffer).array, buffer_size, status);                    \
        (buffer).array.size = 0;                                               \
        (buffer).head = (buffer).array.items;                                  \
        (buffer).tail = (buffer).array.items;                                  \
        (buffer).offset_index = 0;                                             \
        (status) = 0;                                                          \
    } while (0)

// Since this is a fixed buffer size I am opting to allow the ring buffer to eat
// itself without issue. This is very much just do to my use case for graphics
// but probably not great for other cases.
#define PUSH_BUFFER(buffer, item)                                              \
    do {                                                                       \
        if ((buffer).array.size == 0) {                                        \
            *((buffer).head) = item;                                           \
            (buffer).array.size += 1;                                          \
        }                                                                      \
        else {                                                                 \
            if ((buffer).tail !=                                               \
                (buffer).array.items + ((buffer).array.alloc - 1)) {           \
                (buffer).tail = (buffer).tail + 1;                             \
            }                                                                  \
            else {                                                             \
                (buffer).tail = (buffer).array.items;                          \
            }                                                                  \
            *((buffer).tail) = item;                                           \
            if ((buffer).tail == (buffer).head) {                              \
                DEBUG_PRINT("Buffer's tail is eating the head");               \
                if ((buffer).head !=                                           \
                    (buffer).array.items + ((buffer).array.alloc - 1)) {       \
                    (buffer).head = (buffer).head + 1;                         \
                    (buffer).offset_index += 1;                                \
                }                                                              \
                else {                                                         \
                    (buffer).head = (buffer).array.items;                      \
                    (buffer).offset_index = 0;                                 \
                }                                                              \
            }                                                                  \
            else {                                                             \
                (buffer).array.size += 1;                                      \
            }                                                                  \
        }                                                                      \
    } while (0)

#define POP_FRONT_BUFFER(buffer)                                               \
    do {                                                                       \
        if ((buffer).array.size == 0) {                                        \
            break;                                                             \
        }                                                                      \
        else {                                                                 \
            *((buffer).head) = 0;                                              \
            if ((buffer).head !=                                               \
                (buffer).array.items + ((buffer).array.alloc - 1)) {           \
                (buffer).head = (buffer).head + 1;                             \
                (buffer).offset_index += 1;                                    \
            }                                                                  \
            else {                                                             \
                (buffer).head = (buffer).array.items;                          \
                (buffer).offset_index = 0;                                     \
            }                                                                  \
            (buffer).array.size -= 1;                                          \
        }                                                                      \
    } while (0)

#define GET_ITEM(buffer, index, item, status)                                  \
    do {                                                                       \
        if ((buffer).array.alloc <= index) {                                   \
            DEBUG_ERROR(                                                       \
                "Called GET_ITEM with an index that was larger than buffer");  \
            (status) = INVALIDARGS;                                            \
            break;                                                             \
        }                                                                      \
        if ((index + (buffer).offset_index) >= (buffer).array.alloc) {         \
            (item) = (buffer).array.items +                                    \
                     (((buffer).offset_index + index) - (buffer).array.alloc); \
        }                                                                      \
        else {                                                                 \
            (item) = (buffer).array.items + (index + (buffer).offset_index);   \
        }                                                                      \
    } while (0)

#endif
