#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

struct Arena {
    struct Arena *prevNode;
    struct Arena *nextNode;
    void *start;
    size_t currentOffset;
    size_t size;
};

// arena creation
// Since size is a uint32 the max total arena size is 2^32. If you need more
// than that then you are crazy
struct Arena *createArena();

// destroy the arena. The arena pointer will be returned as null
void burnItDown(struct Arena **arena);

// frees the memory but doesn't destroy the memory
void freeWholeArena(struct Arena **arena);
int freeArena(struct Arena **arena, size_t size);

// memory allocs on the arena
void *mallocArena(struct Arena **arena, size_t size);
void *zmallocArena(struct Arena **arena, size_t size);

// scratch pad methods
void *startScratchPad(const struct Arena *arena);
int restoreSratchPad(struct Arena **arena, void *restorePoint);
#endif
