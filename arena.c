#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"

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
	// again a single malloc is used for the node and size so the start is just after the header
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
	}
	else {
		DEBUG_PRINT("Warning the start pointer passed to `burnItDown` was already freed");
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
#define ASSERT_TRUE(expression, status) ((expression) ? status : 1)

int testCreateArena() {
    int status = 0;
    // test the creation of the arena
    struct Arena *arena = createArena(50 * sizeof(int));
    status = ASSERT_TRUE(arena->size == (50 * sizeof(int)), status);
    status = ASSERT_TRUE(arena->currentOffset == 0, status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);
    status = ASSERT_TRUE(arena->start != NULL, status);

    // check clean up
    burnItDown(&arena);
    status = ASSERT_TRUE(arena == NULL, status);
    return status;
}

int testAllocMemory() {
    int status = 0;
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);

    // test with a single node
    float *a = mallocArena(&arena, 20 * sizeof(float));
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);
    status = ASSERT_TRUE(arena->start != NULL, status);
    status = ASSERT_TRUE(a != NULL, status);

    // attempt to alloc memory at the last index to check that the memory is
    // valid if this fails then the code will code dump
    a[19] = 1.0f;

    // alloc four more times to show that the arena builds a linked list
    float *b = mallocArena(&arena, 10 * sizeof(float));
    status = ASSERT_TRUE(b != NULL, status);
    status = ASSERT_TRUE(arena->currentOffset == (30 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);

    float *c = mallocArena(&arena, 40 * sizeof(float));
    status = ASSERT_TRUE(c != NULL, status);
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    // the last node should not have any space at this point so this should be
    // the first private node
    status = ASSERT_TRUE(arena->currentOffset == (40 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode != NULL, status);

    // one more time!
    float *d = mallocArena(&arena, 50 * sizeof(float));
    status = ASSERT_TRUE(d != NULL, status);
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    // the last node should not have any space at this point so this should be
    // the first private node
    status = ASSERT_TRUE(arena->currentOffset == (50 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode != NULL, status);

    burnItDown(&arena);
    return status;
}

int testZAllocMemory() {
    int status = 0;
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);

    // test with a single node
    float *a = zmallocArena(&arena, 20 * sizeof(float));
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);
    status = ASSERT_TRUE(arena->start != NULL, status);
    status = ASSERT_TRUE(a != NULL, status);
    status = ASSERT_TRUE(a[0] == 0.0f, status);
    status = ASSERT_TRUE(a[19] == 0.0f, status);

    burnItDown(&arena);
    return status;
}

int testFreeArena() {
    int status = 0;
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);

    // test with a single node
    float *a = mallocArena(&arena, 20 * sizeof(float));
    status = ASSERT_TRUE(a != NULL, status);
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)), status);

    float *b = mallocArena(&arena, 40 * sizeof(float));
    status = ASSERT_TRUE(b != NULL, status);
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    // the last node should not have any space at this point so this should be
    // the first private node
    status = ASSERT_TRUE(arena->currentOffset == (40 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode != NULL, status);

    // First test that the sized free works in the same node and across nodes
    freeArena(&arena, 15 * sizeof(float));
    status = ASSERT_TRUE(arena->currentOffset == (25 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode != NULL, status);

    freeArena(&arena, 35 * sizeof(float));
    status = ASSERT_TRUE(arena->currentOffset == (10 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode != NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);

    // add back in the other node
    float *c = mallocArena(&arena, 40 * sizeof(float));
    float *d = mallocArena(&arena, 40 * sizeof(float));
    status = ASSERT_TRUE(c != NULL, status);
    status = ASSERT_TRUE(d != NULL, status);
    // free the whole arena
    freeWholeArena(&arena);
    status = ASSERT_TRUE(arena->currentOffset == 0, status);
    status = ASSERT_TRUE(arena->nextNode != NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);

    burnItDown(&arena);
    return status;
}

int testScratchPad() {
    int status = 0;
    struct Arena *arena = createArena(50 * sizeof(float));
    // sanity check
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);

    // alloc some mem and use that as the start of the pad
    float *a = mallocArena(&arena, 20 * sizeof(float));
    status = ASSERT_TRUE(a != NULL, status);
    status = ASSERT_TRUE(arena->size == (50 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)), status);
    void *oldStart = arena->start;

    void *returnPoint = startScratchPad(arena);

    float *b = mallocArena(&arena, 40 * sizeof(float));
    float *c = mallocArena(&arena, 40 * sizeof(float));
    status = ASSERT_TRUE(b != NULL, status);
    status = ASSERT_TRUE(c != NULL, status);

    status = ASSERT_TRUE(arena->currentOffset == (40 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode == NULL, status);
    status = ASSERT_TRUE(arena->prevNode != NULL, status);

    restoreSratchPad(&arena, returnPoint);
    status = ASSERT_TRUE(arena->currentOffset == (20 * sizeof(float)), status);
    status = ASSERT_TRUE(arena->nextNode != NULL, status);
    status = ASSERT_TRUE(arena->prevNode == NULL, status);
    status = ASSERT_TRUE(arena->start == oldStart, status);

    burnItDown(&arena);
    return status;
}

int main() {
    int totalPassed = 0;
    int status = 0;
    status = testCreateArena();
    totalPassed += status == 0 ? 1 : 0;
    printf("testCreateArena: returned with %d\n", status);
    status = testAllocMemory();
    totalPassed += status == 0 ? 1 : 0;
    printf("testAllocMemory: returned with %d\n", status);
    status = testZAllocMemory();
    totalPassed += status == 0 ? 1 : 0;
    printf("testZAllocMemory: returned with %d\n", status);
    status = testFreeArena();
    totalPassed += status == 0 ? 1 : 0;
    printf("testFreeArena: returned with %d\n", status);
    status = testScratchPad();
    totalPassed += status == 0 ? 1 : 0;
    printf("testScratchPad: returned with %d\n", status);
    printf("number of tests passed %d out of %d\n", totalPassed, 5);
}
#endif
