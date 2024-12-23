#include "arena.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct Arena {
    struct Arena *prevNode;
    struct Arena *nextNode;
    void *start;
    uint32_t currentOffset;
    uint32_t size;
};

struct Arena *createArena(uint32_t size) {
    // We have to allocate this on the heap that way the life time lasts longer
    // then this function call. We could have the user pass in an arena type
    // that way the caller is in charge of lifetime.
    struct Arena *arena = malloc(sizeof(struct Arena));
    if (arena == NULL) {
        DEBUG_PRINT("Fatal: Initial arena alloc failed\n");
        return NULL;
    }
    // build the arena!
    // again a single malloc is used for the node and size so the start is just
    // after the header
    arena->start = malloc(size);
    arena->currentOffset = 0;
    arena->size = 0;
    arena->prevNode = NULL;
    arena->nextNode = NULL;
    if (arena->start != NULL) {
        arena->size = size;
    } else {
        DEBUG_PRINT("Fatal: Internal arena alloc failed\n");
    }
    return arena;
}

// private function used to create additional nodes
struct Arena *createArenaNode(struct Arena *prev) {
    // This is the same as createArena yet it is building a node
    // on a linked list
    struct Arena *arena = createArena(prev->size);
    if (arena == NULL) {
        DEBUG_PRINT(
            "Fatal: createArenaNode was unable to allocate another node to "
            "the arena\n");
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
        free((*arena)->start);
        (*arena)->start = NULL;
    } else {
        DEBUG_PRINT("Warning the start pointer passed to `burnItDown` was "
                    "already freed");
    }
    free(*arena);
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
int freeArena(struct Arena **arena, uint32_t size) {
    if (arena == NULL || *arena == NULL) {
        DEBUG_PRINT(
            "Warning `freeArena` was called with a bad arena pointer\n");
        return -1;
    }
    if (size <= 0) {
        return 0;
    }

    if ((*arena)->currentOffset < size) {
        // if the prev node is null and there is not enough size to continue to
        // free memory then the free size is larger than expected.
        if ((*arena)->prevNode == NULL) {
            DEBUG_PRINT("Warning `freeArena` is not large enough to free that "
                        "many bytes\n");
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
    } else {
        // no need to update the arena pointer
        (*arena)->currentOffset -= size;
        return 0;
    }
}

void *mallocArena(struct Arena **arena, int32_t size) {
    if (arena == NULL || *arena == NULL) {
        DEBUG_PRINT(
            "fatal `mallocArena` was called with a bad arena pointer\n");
        return NULL;
    }
    if (size > (*arena)->size) {
        DEBUG_PRINT("fatal `mallocArena` was called with a size that is larger "
                    "than one arena node\n");
        return NULL;
    }

    // already room in this node. Lets use it.
    if ((*arena)->size >= (size + (*arena)->currentOffset)) {
        void *startOfRegion = (*arena)->start + (*arena)->currentOffset;
        (*arena)->currentOffset += size;
        return startOfRegion;
    }

    // check if the next node exists and if it does use that before creating
    // another one
    if ((*arena)->nextNode != NULL) {
        *arena = (*arena)->nextNode;
        return mallocArena(arena, size);
    }

    // The arena is not able to allocate that much memory in this arena.
    // The current arena will not contain any of this data due to memory of
    // one allocation having to be continuous.
    struct Arena *newArena = createArenaNode(*arena);
    if (newArena == NULL) {
        return NULL;
    }

    *arena = newArena;
    return mallocArena(&newArena, size);
}

// the same as mallocArena but it will call memset 0 on the memory
void *zmallocArena(struct Arena **arena, uint32_t size) {
    void *memoryLocation = mallocArena(arena, size);
    if (memoryLocation != NULL) {
        memset(memoryLocation, 0, size);
    }
    return memoryLocation;
}

void *startScratchPad(struct Arena *arena) {
    return arena->start + arena->currentOffset;
}

int restoreSratchPad(struct Arena **arena, void *restorePoint) {
    // this needs to check if the pointer is within this range and
    // if not then go to the prev node. Check going until the node is found
    // if it is not found then return -1
    if (arena == NULL || *arena == NULL) {
        DEBUG_PRINT(
            "Warning `restoreSratchPad` was called with a bad arena pointer\n");
        return -1;
    }

    if ((*arena)->start <= restorePoint &&
        restorePoint <= ((*arena)->start + (*arena)->size)) {
        // The restore point is within this node
        (*arena)->currentOffset = restorePoint - (*arena)->start;
        return 0;
    } else {
        if ((*arena)->prevNode == NULL) {
            DEBUG_PRINT(
                "Warning `restoreStrachPad` was unable to find the node that "
                "contains the restorePoint\n");
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
        *arena = localArenaPointer;
        return status;
    }
}

// Throwing unit tests in the c file for now. Not sure how I feel about it but
// here we are.
//
// UNIT TESTS
//
//
#ifdef ARENA_DEBUG
#include "unittest.h"
void testCreateArena() {
    // test the creation of the arena
    struct Arena *arena = createArena(50 * sizeof(int));
    ASSERT_TRUE(arena->size == (50 * sizeof(int)), "check arena size");
    ASSERT_TRUE(arena->currentOffset == 0, "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");
    ASSERT_TRUE(arena->start != NULL, "check arena start pointer");

    // check clean up
    burnItDown(&arena);
    ASSERT_TRUE(arena == NULL, "check cleanup");
}

void testAllocMemory() {
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check arena size");

    // test with a single node
    float *a = mallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check arena size");
    ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");
    ASSERT_TRUE(arena->start != NULL, "check arena start");
    ASSERT_TRUE(a != NULL, "check malloc'ed pointer status");

    // attempt to alloc memory at the last index to check that the memory is
    // valid if this fails then the code will code dump
    a[19] = 1.0f;

    // alloc four more times to show that the arena builds a linked list
    float *b = mallocArena(&arena, 10 * sizeof(float));
    ASSERT_TRUE(b != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE(arena->currentOffset == (30 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");

    float *c = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(c != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check arena size");
    // the last node should not have any space at this point so this should be
    // the first private node
    ASSERT_TRUE(arena->currentOffset == (40 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    // one more time!
    float *d = mallocArena(&arena, 50 * sizeof(float));
    ASSERT_TRUE(d != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check arena size");
    // the last node should not have any space at this point so this should be
    // the first private node
    ASSERT_TRUE(arena->currentOffset == (50 * sizeof(float)),
                "check the current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    burnItDown(&arena);
}

void testZAllocMemory() {
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check size");

    // test with a single node
    float *a = zmallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check malloc size");
    ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");
    ASSERT_TRUE(arena->start != NULL, "check arena start");
    ASSERT_TRUE(a != NULL, "check malloc pointer status");
    ASSERT_TRUE(a[0] == 0.0f, "check first value");
    ASSERT_TRUE(a[19] == 0.0f, "check 20th value");

    burnItDown(&arena);
}

void testFreeArena() {
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    ASSERT_TRUE(arena->size == (50 * sizeof(float)), "check initial size");

    // test with a single node
    float *a = mallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(a != NULL, "check that a is not null to start");
    ASSERT_TRUE(arena->size == (50 * sizeof(float)),
                "check that the size is correct");
    ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)),
                "check the offset now");

    float *b = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(b != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE(arena->size == (50 * sizeof(float)),
                "check that the size increased");
    // the last node should not have any space at this point so this should be
    // the first private node
    ASSERT_TRUE(arena->currentOffset == (40 * sizeof(float)), "chelc offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    // First test that the sized free works in the same node and across nodes
    freeArena(&arena, 15 * sizeof(float));
    ASSERT_TRUE(arena->currentOffset == (25 * sizeof(float)),
                "check the offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    freeArena(&arena, 35 * sizeof(float));
    ASSERT_TRUE(arena->currentOffset == (10 * sizeof(float)),
                "check the offset");
    ASSERT_TRUE(arena->nextNode != NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");

    // add back in the other node
    float *c = mallocArena(&arena, 40 * sizeof(float));
    float *d = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(c != NULL, "check malloc status");
    ASSERT_TRUE(d != NULL, "check malloc status");
    // free the whole arena
    freeWholeArena(&arena);
    ASSERT_TRUE(arena->currentOffset == 0, "check that offset has cleared");
    ASSERT_TRUE(arena->nextNode != NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");

    burnItDown(&arena);
}

void testScratchPad() {
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    ASSERT_TRUE(arena->size == (50 * sizeof(float)),
                "check initial size of the arena");

    // alloc some mem and use that as the start of the pad
    float *a = mallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(a != NULL, "check that given pointer is not null");
    ASSERT_TRUE(arena->size == (50 * sizeof(float)),
                "check that size is the same");
    ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)),
                "check that the offset is equal to the alloc");
    void *oldStart = arena->start;

    void *returnPoint = startScratchPad(arena);

    float *b = mallocArena(&arena, 40 * sizeof(float));
    float *c = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(b != NULL, "check that the new alloc is not null");
    ASSERT_TRUE(c != NULL, "check that the other new alloc is not null");

    ASSERT_TRUE(arena->currentOffset == (40 * sizeof(float)),
                "check arena current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check that next node is null");
    ASSERT_TRUE(arena->prevNode != NULL,
                "check that the prev node is not null");

    restoreSratchPad(&arena, returnPoint);
    ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)),
                "check that the offset still makes sense");
    ASSERT_TRUE(arena->nextNode != NULL, "check that the next node is not null "
                                         "since we should have gone back one");
    ASSERT_TRUE(arena->prevNode == NULL,
                "check that there is no prev node now");
    ASSERT_TRUE(arena->start == oldStart,
                "check that the arena is back to where is started");

    burnItDown(&arena);
}

int main() {
    struct Arena *memory = createArena(DEFAULT_SIZE);
    setUp(memory);
    ADD_TEST(testCreateArena);
    ADD_TEST(testAllocMemory);
    ADD_TEST(testZAllocMemory);
    ADD_TEST(testFreeArena);
    ADD_TEST(testScratchPad);
    return runTest();
}
#endif
