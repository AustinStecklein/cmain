#include "arena.h"
#include "debug.h"
#include <math.h>
#include <stdalign.h> // alignof, max_align_t
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> // asprintf
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> // mmap
#include <unistd.h>   // getpagesize

// The size passed in is a reference to the size of the object that will
// get allocated. This allows for arena nodes to be larger than a page
// size in the case that happens.
struct Arena *createSizedArena(uint32_t size) {
    uint32_t pageSize = getpagesize();
    uint32_t allocableSpace = pageSize - sizeof(struct Arena);
    // if the page size is zero here then we got more problems then allocating
    // memory
    uint32_t arenaSize =
        pageSize * (int)ceill(((float)size / (float)allocableSpace));
    // build the arena! "is that a freaking void pointer - mike"
#ifdef VALGRIND
    // it is just easier to use the heap with valgrind
    void *pageStart = malloc(arenaSize);
#else
    void *pageStart = mmap(NULL, arenaSize, PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#endif

    struct Arena *arena = pageStart;
    if (arena == NULL) {
        DEBUG_ERROR("Initial arena alloc failed");
        return NULL;
    }
    arena->start = pageStart + sizeof(struct Arena);
    arena->currentOffset = 0;
    arena->size = 0;
    arena->prevNode = NULL;
    arena->nextNode = NULL;
    if (arena->start != NULL) {
        arena->size = arenaSize - (arena->start - pageStart);
    }
    else {
        DEBUG_ERROR("Internal arena alloc failed");
    }
    return arena;
}

struct Arena *createArena() {
    // just pass in 1 since we don't really care about the size. We just want
    // the arena to be at the default page size.
    return createSizedArena(1);
}

// private function used to create additional nodes
struct Arena *createArenaNode(struct Arena *prev, int size) {
    // if the arena pointers are null then it is at the end of the tree of nodes
    if (prev == NULL) {
        DEBUG_ERROR("`createArenaNode` was called with a bad arena pointer");
        return NULL;
    }
    // This is the same as createArena yet it is building a node
    // on a linked list
    struct Arena *arena = createSizedArena(size);
    if (arena == NULL) {
        DEBUG_ERROR(
            "createArenaNode was unable to allocate another node to the arena");
        return NULL;
    }
    prev->nextNode = arena;
    arena->prevNode = prev;
    return arena;
}

// This is a true free. As in the memory is should be full released
void burnItDown(struct Arena **arena) {
    // if the arena pointers are null then it is at the end of the tree of nodes
    if (arena == NULL || *arena == NULL) {
        return;
    }
    // Free forward nodes first
    if ((*arena)->nextNode != NULL) {
        (*arena)->nextNode->prevNode = NULL;
        burnItDown(&((*arena)->nextNode));
    }

    // now this node is free it is safe to free the prev nodes
    if ((*arena)->prevNode != NULL) {
        (*arena)->prevNode->nextNode = NULL;
        burnItDown(&((*arena)->prevNode));
    }
    // free all of the pointers now
    if ((*arena)->start != NULL) {

#ifdef VALGRIND
        free((*arena)->start - sizeof(struct Arena));
        int error_code = 0;
#else
        int error_code = munmap((*arena)->start - sizeof(struct Arena),
                                (*arena)->size + sizeof(struct Arena));
#endif
        // this will allocate memory from the heap instead of from the arena so
        // this is hidden behind the debug flag
        if (error_code != 0) {
            char *log_message = NULL;
            if (asprintf(&log_message,
                         "Fatal error %d occured while attempting to free "
                         "arena memory\n",
                         error_code) > 0) {
                DEBUG_ERROR(log_message);
                if (log_message != NULL)
                    free(log_message);
            }
            else
                // if asprintf fails still log a message
                DEBUG_ERROR("Fatal error occured while attempting to free "
                            "arena memory");
        }
    }
    else
        DEBUG_ERROR(
            "The start pointer passed to `burnItDown` was already freed");
    *arena = NULL;
}

// This is not a true free. The Arena holds onto the memory but we give out
// memory that was once used
void freeWholeArena(struct Arena **arena) {
    if (arena == NULL || *arena == NULL) {
        return;
    }
    (*arena)->currentOffset = 0;
    if ((*arena)->prevNode != NULL) {
        *arena = (*arena)->prevNode;
        freeWholeArena(arena);
    }
}

// free only some amount of bytes from the allocator
// This function WILL assume that it is passed the most right most node
int freeArena(struct Arena **arena, size_t size) {
    if (arena == NULL || *arena == NULL) {
        DEBUG_ERROR("`freeArena` was called with a bad arena pointer");
        return -1;
    }

    if ((*arena)->currentOffset < size) {
        // if the prev node is null and there is not enough size to continue to
        // free memory then the free size is larger than expected.
        if ((*arena)->prevNode == NULL) {
            DEBUG_ERROR(
                "`freeArena` is not large enough to free that many bytes");
            return -1;
        }
        // only start to "free" the arena if every node has agreed that the free
        // is possible
        struct Arena *local_arena_pointer = *arena;
        *arena = (*arena)->prevNode;
        int status =
            freeArena(arena, size - local_arena_pointer->currentOffset);
        if (!status) {
            // only start to free if there is enough room
            local_arena_pointer->currentOffset = 0;
            return status;
        }
        *arena = local_arena_pointer;
        return status;
    }
    else {
        // no need to update the arena pointer
        (*arena)->currentOffset -= size;
        return 0;
    }
}

void *mallocArena(struct Arena **arena, size_t size) {
    if (arena == NULL || *arena == NULL) {
        DEBUG_ERROR("`mallocArena` was called with a bad arena pointer");
        return NULL;
    }
    // get the alignment offset
    uint64_t alignment = size;
    if (size > alignof(max_align_t))
        alignment = alignof(max_align_t);
    // already room in this node. Lets use it.
    if ((*arena)->size >= (size + (*arena)->currentOffset)) {
        // check that there is still room if we add the alignment offset
        void *currentFree = (*arena)->start + (*arena)->currentOffset;
        void *startOfRegion =
            (void *)(((uint64_t)currentFree + (alignment - 1)) &
                     ~(alignment - 1));
        if (startOfRegion <= ((*arena)->start + (*arena)->size)) {
            (*arena)->currentOffset += size + (startOfRegion - currentFree);
            return startOfRegion;
        }
    }

    // check if the next node exists and if it does use that before creating
    // another one
    if ((*arena)->nextNode != NULL) {
        *arena = (*arena)->nextNode;
        // add some buffer to allow for alighment
        return mallocArena(arena, size + (alignof(max_align_t) - 1));
    }

    // The arena is not able to allocate that much memory in this arena.
    // The current arena will not contain any of this data due to memory of
    // one allocation having to be continuous.
    struct Arena *newArena = createArenaNode(*arena, size);
    if (newArena == NULL) {
        DEBUG_ERROR("`mallocArena` was unable to create another node");
        return NULL;
    }

    *arena = newArena;
    return mallocArena(&newArena, size);
}

// the same as mallocArena but it will call memset 0 on the memory
void *zmallocArena(struct Arena **arena, size_t size) {
    void *memoryLocation = mallocArena(arena, size);
    if (memoryLocation != NULL) {
        memset(memoryLocation, 0, size);
    }
    return memoryLocation;
}

void *startScratchPad(struct Arena *arena) {
    if (arena == NULL) {
        DEBUG_ERROR("`startScratchPad` was called with a bad arena pointer");
        return NULL;
    }
    return arena->start + arena->currentOffset;
}

int restoreSratchPad(struct Arena **arena, void *restorePoint) {
    // this needs to check if the pointer is within this range and
    // if not then go to the prev node. Check going until the node is found
    // if it is not found then return -1
    if (arena == NULL || *arena == NULL || restorePoint == NULL) {
        DEBUG_ERROR("`restoreSratchPad` was called with a bad arena pointer");
        return -1;
    }

    if ((*arena)->start <= restorePoint &&
        restorePoint <= ((*arena)->start + (*arena)->size)) {
        // The restore point is within this node
        (*arena)->currentOffset = restorePoint - (*arena)->start;
        return 0;
    }
    else {
        if ((*arena)->prevNode == NULL) {
            DEBUG_ERROR("`restoreStrachPad` was unable to find the node that "
                        "contains the restorePoint");
            return -1;
        }
        struct Arena *localArenaPointer = *arena;
        *arena = (*arena)->prevNode;
        int status = restoreSratchPad(arena, restorePoint);
        if (!status) {
            localArenaPointer->currentOffset = 0;
            return status;
        }
        // reset this back to what it was originally since there was an error
        // if this has been reached
        *arena = localArenaPointer;
        return status;
    }
}
