#include <stdint.h>
// defining DEBUG will enable debug logging statements
#define DEBUG

// in the future this should be going to stderr instead of stdout
#ifdef DEBUG
#define DEBUG_PRINT(message) (printf(message))
#else
#define DEBUG_PRINT(message)
#endif

/*
 * notes on other features
 * look into what marcos make sense to create in the .h for easier use of the
 * improvements that could be made with this for other data structures
 * - have a list of free memory of a fixed size that can be used to "free"
 * memory in the middle of the stack
 * -- this might have to be just a data structure built off of the arena
 * - look into claiming virtual memory from the os that way the data structure
 * can always get contiguous memory. ball out
 */

// base arena struct
struct Arena;

// arena creation
// Since size is a uint32 the max total arena size is 2^32. If you need more
// than that then you are crazy
struct Arena *createArena(uint32_t size);

// destroy the arena. The arena pointer will be returned as null
void burnItDown(struct Arena **arena);

// frees the memory but doesn't destroy the memory
void freeWholeArena(struct Arena **arena);
int freeArena(struct Arena **arena, uint32_t size);

// memory allocs on the arena
void *mallocArena(struct Arena **arena, int32_t size);
void *zmallocArena(struct Arena **arena, uint32_t size);

// scratch pad methods
void *startScratchPad(struct Arena *arena);
int restoreSratchPad(struct Arena **arena, void *restorePoint);
