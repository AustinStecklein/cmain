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

struct Arena {
    struct Arena *prevNode;
    struct Arena *nextNode;
    void *start;
    size_t currentOffset;
    size_t size;
};

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
    } else {
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
#ifdef ARENA_DEBUG
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
            } else {
                // if asprintf fails still log a message
                DEBUG_ERROR("Fatal error occured while attempting to free "
                            "arena memory");
            }
        }
#else
#ifdef VALGRIND
        free((*arena)->start - sizeof(struct Arena));
#else
        munmap((*arena)->start - sizeof(struct Arena),
               (*arena)->size + sizeof(struct Arena));
#endif
#endif
    } else {
        DEBUG_ERROR(
            "The start pointer passed to `burnItDown` was already freed");
    }
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
    } else {
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
    } else {
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

// Throwing unit tests in the c file for now. Not sure how I feel about it but
// here we are.
//
// UNIT TESTS
//
//
#ifdef ARENA_DEBUG
#include "unittest.h"
void testCreateArena(struct Arena *testArena) {
    // test the creation of the arena
    uint32_t size = getpagesize() - sizeof(struct Arena);
    struct Arena *arena = createArena();
    ASSERT_TRUE(arena->size == size, "check arena size");
    ASSERT_TRUE(arena->currentOffset == 0, "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");
    ASSERT_TRUE(arena->start != NULL, "check arena start pointer");

    // check clean up
    burnItDown(&arena);
    ASSERT_TRUE(arena == NULL, "check cleanup");
}

void testAllocMemory(struct Arena *testArena) {
    uint32_t size = getpagesize() - sizeof(struct Arena);
    struct Arena *arena = createArena();
    // sanity check
    ASSERT_TRUE(arena->size == size, "check arena size");

    // test with a single node
    float *a = mallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE((uint64_t)a % alignof(float) == 0,
                "check returned memory is aligned");
    ASSERT_TRUE(arena->size == size, "check arena size");
    ASSERT_TRUE(arena->currentOffset >= (20 * sizeof(float)),
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
    ASSERT_TRUE((uint64_t)b % alignof(float) == 0,
                "check returned memory is aligned");
    ASSERT_TRUE(arena->currentOffset >= (30 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");

    // throwaway alloc to fill the current node
    float *x = mallocArena(&arena, arena->size - arena->currentOffset);
    ASSERT_TRUE(x != NULL, "check malloc'ed pointer status");

    float *c = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(c != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE((uint64_t)c % alignof(float) == 0,
                "check returned memory is aligned");
    ASSERT_TRUE(arena->size == size, "check arena size");
    // the last node should not have any space at this point so this should be
    // the first private node
    ASSERT_TRUE(arena->currentOffset >= (40 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    // one more time!
    // throwaway alloc to fill the current node
    float *y = mallocArena(&arena, arena->size - arena->currentOffset);
    ASSERT_TRUE(y != NULL, "check malloc'ed pointer status");

    float *d = mallocArena(&arena, 50 * sizeof(float));
    ASSERT_TRUE(d != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE((uint64_t)d % alignof(float) == 0,
                "check returned memory is aligned");
    ASSERT_TRUE(arena->size == size, "check arena size");
    // the last node should not have any space at this point so this should be
    // the first private node
    ASSERT_TRUE(arena->currentOffset >= (50 * sizeof(float)),
                "check the current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    float *e = mallocArena(&arena, 2 * getpagesize());
    uint32_t largeSize = (3 * getpagesize()) - sizeof(struct Arena);
    ASSERT_TRUE(d != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE((uint64_t)e % alignof(float) == 0,
                "check returned memory is aligned");
    ASSERT_TRUE(arena->size == largeSize, "check arena size");

    burnItDown(&arena);
}

void testZAllocMemory(struct Arena *testArena) {
    uint32_t size = getpagesize() - sizeof(struct Arena);
    struct Arena *arena = createArena();
    // sanity check
    ASSERT_TRUE(arena->size == size, "check size");

    // test with a single node
    float *a = zmallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(arena->size == size, "check malloc size");
    ASSERT_TRUE(arena->currentOffset >= (20 * sizeof(float)),
                "check current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");
    ASSERT_TRUE(arena->start != NULL, "check arena start");
    ASSERT_TRUE(a != NULL, "check malloc pointer status");
    ASSERT_TRUE(a[0] == 0.0f, "check first value");
    ASSERT_TRUE(a[19] == 0.0f, "check 20th value");

    burnItDown(&arena);
}

void testFreeArena(struct Arena *testArena) {
    uint32_t size = getpagesize() - sizeof(struct Arena);
    struct Arena *arena = createArena();
    // sanity check
    ASSERT_TRUE(arena->size == size, "check initial size");

    // test with a single node
    float *a = mallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(a != NULL, "check that a is not null to start");
    ASSERT_TRUE(arena->size == size, "check that the size is correct");
    ASSERT_TRUE(arena->currentOffset >= (20 * sizeof(float)),
                "check the offset now");

    // throwaway alloc to fill the current node
    uint32_t bufferMemory = arena->size - arena->currentOffset;
    float *y = mallocArena(&arena, bufferMemory);
    ASSERT_TRUE(y != NULL, "check malloc'ed pointer status");

    float *b = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(b != NULL, "check malloc'ed pointer status");
    ASSERT_TRUE(arena->size == size, "check that the size increased");

    // the last node should not have any space at this point so this should be
    // the first private node
    ASSERT_TRUE(arena->currentOffset >= (40 * sizeof(float)), "check offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    // First test that the sized free works in the same node and across nodes
    freeArena(&arena, 15 * sizeof(float));
    ASSERT_TRUE(arena->currentOffset >= (25 * sizeof(float)),
                "check the offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check next status");
    ASSERT_TRUE(arena->prevNode != NULL, "check prev status");

    freeArena(&arena, 35 * sizeof(float) + (bufferMemory));
    ASSERT_TRUE(arena->currentOffset >= (10 * sizeof(float)),
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

void testScratchPad(struct Arena *testArena) {
    uint32_t size = getpagesize() - sizeof(struct Arena);
    struct Arena *arena = createArena();
    // sanity check
    ASSERT_TRUE(arena->size == size, "check initial size of the arena");

    // alloc some mem and use that as the start of the pad
    float *a = mallocArena(&arena, 20 * sizeof(float));
    ASSERT_TRUE(a != NULL, "check that given pointer is not null");
    ASSERT_TRUE(arena->size == size, "check that size is the same");
    ASSERT_TRUE(arena->currentOffset >= (20 * sizeof(float)),
                "check that the offset is greater than or equal to the alloc");
    void *oldStart = arena->start;

    void *returnPoint = startScratchPad(arena);

    float *b = mallocArena(&arena, 40 * sizeof(float));

    uint32_t bufferMemory = arena->size - arena->currentOffset;
    float *x = mallocArena(&arena, bufferMemory);
    ASSERT_TRUE(x != NULL, "check malloc'ed pointer status");

    float *c = mallocArena(&arena, 40 * sizeof(float));
    ASSERT_TRUE(b != NULL, "check that the new alloc is not null");
    ASSERT_TRUE(c != NULL, "check that the other new alloc is not null");

    ASSERT_TRUE(arena->currentOffset >= (40 * sizeof(float)),
                "check arena current offset");
    ASSERT_TRUE(arena->nextNode == NULL, "check that next node is null");
    ASSERT_TRUE(arena->prevNode != NULL,
                "check that the prev node is not null");

    restoreSratchPad(&arena, returnPoint);
    ASSERT_TRUE(arena->currentOffset >= (20 * sizeof(float)),
                "check that the offset still makes sense");
    ASSERT_TRUE(arena->nextNode != NULL, "check that the next node is not null "
                                         "since we should have gone back one");
    ASSERT_TRUE(arena->prevNode == NULL,
                "check that there is no prev node now");
    ASSERT_TRUE(arena->start == oldStart,
                "check that the arena is back to where is started");

    burnItDown(&arena);
}

void testMemoryAlignment(struct Arena *testArena) {
    // show that memory will be auto aligned
    uint32_t size = getpagesize() - sizeof(struct Arena);
    struct Arena *arena = createArena();
    // sanity check
    ASSERT_TRUE(arena->size == size, "check initial size of the arena");

    uint32_t *a = mallocArena(&arena, sizeof(uint32_t));
    ASSERT_TRUE((uint64_t)a % alignof(uint32_t) == 0,
                "check returned memory is aligned");
    ASSERT_TRUE(arena->size == size, "check arena size");
    ASSERT_TRUE(arena->currentOffset >= sizeof(uint32_t),
                "check current offset");

    char *b = mallocArena(&arena, sizeof(char));
    ASSERT_TRUE((uint64_t)b % alignof(char) == 0,
                "check returned memory is aligned");

    char *c = mallocArena(&arena, sizeof(char));
    ASSERT_TRUE((uint64_t)c % alignof(char) == 0,
                "check returned memory is aligned");

    uint64_t *d = mallocArena(&arena, sizeof(uint64_t));
    ASSERT_TRUE((uint64_t)d % alignof(uint64_t) == 0,
                "check returned memory is aligned");
    burnItDown(&arena);
}

void testArenaFaults(struct Arena *testArena) {
    DEBUG_PRINT("`testArenaFaults` will trigger many Error prints. As long as "
                "there is not seg faults this is expected");
    struct Arena *arena_a = createArenaNode(NULL, 0);
    ASSERT_TRUE(arena_a == NULL, "Check safe null returns");

    // cannot assert anything for these but the fact that
    // the program did not seg fault is good enough
    struct Arena *arena_b = NULL;
    burnItDown(NULL);
    burnItDown(&arena_b);
    freeWholeArena(NULL);
    freeWholeArena(&arena_b);

    struct Arena *arena_c = createArena();
    int a = freeArena(NULL, 1);
    ASSERT_TRUE(a == -1, "Check safe null returns");
    int b = freeArena(&arena_b, 1);
    ASSERT_TRUE(b == -1, "Check safe null returns");
    int c = freeArena(&arena_c, 100);
    ASSERT_TRUE(c == -1, "Check freeing to much memory");

    void *pa = mallocArena(NULL, 1);
    ASSERT_TRUE(pa == NULL, "Check safe null returns");
    void *pb = mallocArena(&arena_b, 1);
    ASSERT_TRUE(pb == NULL, "Check safe null returns");

    void *stored = startScratchPad(arena_c);
    int d = restoreSratchPad(NULL, stored);
    ASSERT_TRUE(d == -1, "Check safe null returns");
    int e = restoreSratchPad(&arena_b, stored);
    ASSERT_TRUE(e == -1, "Check safe null returns");
    int f = restoreSratchPad(&arena_c, stored - 1);
    ASSERT_TRUE(f == -1, "Check bad stored pointer");

    burnItDown(&arena_c);
}

int main() {
    struct Arena *memory = createArena();
    int status = 0;
    status = setUp(memory);
    if (status != 0) {
        printf("Failed to setup the test\n");
        return status;
    }
    ADD_TEST(testCreateArena);
    ADD_TEST(testAllocMemory);
    ADD_TEST(testZAllocMemory);
    ADD_TEST(testFreeArena);
    ADD_TEST(testScratchPad);
    ADD_TEST(testMemoryAlignment);
    ADD_TEST(testArenaFaults);
    return runTest();
}
#endif
