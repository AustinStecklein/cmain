#include "test_arena.h"
#include "unittest.h"
#include <stdalign.h> // alignof, max_align_t

struct Arena *createArenaNode(struct Arena *prev, int size);

static void testCreateArena(struct Arena *testArena) {
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

static void testAllocMemory(struct Arena *testArena) {
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

static void testZAllocMemory(struct Arena *testArena) {
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

static void testFreeArena(struct Arena *testArena) {
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

    b[25] = 5.5;
    // First test that the sized free works in the same node and across nodes
    freeArena(&arena, 15 * sizeof(float));
    ASSERT_TRUE(b[25] == 0, "Check that the memory is free'ed");
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
    d[5] = 26;
    freeWholeArena(&arena);
    ASSERT_TRUE(d[5] == 0, "Check that the memory is free'ed");
    ASSERT_TRUE(arena->currentOffset == 0, "check that offset has cleared");
    ASSERT_TRUE(arena->nextNode != NULL, "check next status");
    ASSERT_TRUE(arena->prevNode == NULL, "check prev status");

    burnItDown(&arena);
}

static void testScratchPad(struct Arena *testArena) {
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

    b[5] = 5;
    restoreSratchPad(&arena, returnPoint);
    ASSERT_TRUE(b[5] == 0, "Check that the memory is free'ed");
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

static void testMemoryAlignment(struct Arena *testArena) {
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

static void testArenaFaults(struct Arena *testArena) {
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
    int f = restoreSratchPad(&arena_c, (char *)stored - 1);
    ASSERT_TRUE(f == -1, "Check bad stored pointer");

    burnItDown(&arena_c);
}

int runArenaTests() {
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
