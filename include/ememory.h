#ifndef EMEMORY
#define EMEMORY

#include <stdint.h>
#include <string.h>

// Constants
#define MEM_SIZE            65536
#define STARTING_OFFSET     0x10

// Memory structure
struct ememory {
    char* data;
    uint16_t free_head;
};

// Null pointer for emulated memory
#define ENULL (struct eptr) { 0 }

// Pointer structure
struct eptr {
    uint16_t ptr;
    uint16_t size;
};

/**
 * Checks if a given eptr is null
 */
int is_null(struct eptr ptr);

/**
 * Initializes emulated memory to be used by emalloc() and efree()
 */
void init_ememory(struct ememory* memory, int size);

/**
 * Allocates a new chunk of emulated memory with the given size
 * 
 * @return 	A valid eptr if allocation succeeds, else ENULL
 */
struct eptr emalloc(struct ememory* memory, uint16_t size);

/**
 * Frees memory at the given eptr. 
 * 
 * The argument eptr must be the exact same as the eptr originally returned
 * by a call to emalloc(). Otherwise, this function leads to undefind behavior.
 */
int efree(struct ememory* memory, struct eptr ptr);

#endif // EMEMORY
