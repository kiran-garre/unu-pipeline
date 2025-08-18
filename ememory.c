#include <stdint.h>
#include <string.h>
#include "processor.h"
#include "ememory.h"

#define HEADER_SIZE (2 * sizeof(uint16_t))

int is_null(struct eptr ptr) {
	return (ptr.ptr == 0);
}

/**
 * Emulated memory header format:
 * 
 * +++++++++++++++ +++++++++++++++ 
 * | size (word) | | next (word) |
 * +++++++++++++++ +++++++++++++++ 
 * 
 * where: 
 *  - `size` contains the size of the free space (including the header)
 *  - `next` contains the index of the start of the next free section 
 */

static void write_header(char* data, uint16_t size, uint16_t next, uint16_t offset) {
	memcpy(data + offset, &size, sizeof(uint16_t));
	memcpy(data + offset + sizeof(uint16_t), &next, sizeof(uint16_t));
}

static void read_header(char* data, uint16_t* size, uint16_t* next, uint16_t offset) {
	memcpy(size, data + offset, sizeof(uint16_t));
	memcpy(next, data + offset + sizeof(uint16_t), sizeof(uint16_t));
}

void init_ememory(struct ememory* memory, int size) {
	write_header(memory->data, size - STARTING_OFFSET, 0, STARTING_OFFSET);
	memory->free_head = STARTING_OFFSET;
}

struct eptr emalloc(struct ememory* memory, uint16_t request) {
	if (request == 0) {
		return ENULL;
	}
	struct eptr res;
	char* data = memory->data;
	uint16_t block_size, next;
	uint16_t prev = 0;
	uint16_t curr = memory->free_head;
	
	while (curr) {
		read_header(data, &block_size, &next, curr);

		if (request > block_size) {
			prev = curr;
			curr = next;
			continue;
		}

		if (block_size > request + HEADER_SIZE) {
			// Shrinking will leave enough space for a header + at least 1 byte
			res.ptr = curr;
			res.size = request;

			curr += request;
			write_header(data, block_size - request, next, curr);
			if (prev == 0) {
				memory->free_head = curr;
			} else {
				memcpy(data + prev + sizeof(uint16_t), &curr, sizeof(uint16_t));
			}
			return res;
		} else {
			// Either it's a perfect fit, or the block is big enough but the
			// leftover space is too small to store a header + 1 byte.
			// In both cases, allocate the whole block.
			res.ptr = curr;
			res.size = block_size;

			if (prev == 0) {
				memory->free_head = next;
			} else {
				memcpy(data + prev + sizeof(uint16_t), &next, sizeof(uint16_t));
			}
			return res;
		}
	}
	return ENULL;
}

/**
 * Helper function for efree()
 * 
 * Handles the case where the pointer to be freed is before the first free 
 * header.
 */
static int free_before_head(struct ememory *memory, struct eptr ptr) {
    char* data = memory->data;
    uint16_t head_size, head_next;

    // Read the header of the current head
    read_header(data, &head_size, &head_next, memory->free_head);

    // If new block is directly before the head, merge
    if (ptr.ptr + ptr.size == memory->free_head) {
        write_header(data, ptr.size + head_size, head_next, ptr.ptr);
    } else {
        write_header(data, ptr.size, memory->free_head, ptr.ptr);
    }

    // Update head pointer
    memory->free_head = ptr.ptr;
    return 0;
}

/**
 * Helper function for efree()
 * 
 * Handles the case where the pointer to be freed is in between the first and
 * last free headers.
 * 
 * This function looks horrendous without some spacing.
 */
static int free_in_middle(struct ememory *memory, struct eptr ptr,
                          uint16_t curr, uint16_t curr_size,
                          uint16_t curr_next, uint16_t next_size,
                          uint16_t next_next) {
	char* data = memory->data;
	if (curr + curr_size == ptr.ptr && ptr.ptr + ptr.size == curr_next) {
		write_header(data, curr_size + ptr.size + next_size, next_next, curr);
	} 
	
	else if (curr + curr_size < ptr.ptr && ptr.ptr + ptr.size == curr_next) {
		write_header(data, curr_size, ptr.ptr, curr);
		write_header(data, ptr.size + next_size, next_next, ptr.ptr);
	} 
	
	else if (curr + curr_size == ptr.ptr && ptr.ptr + ptr.size < curr_next) {
		write_header(data, curr_size + ptr.size, curr_next, curr);
	}
	
	else if (curr + curr_size < ptr.ptr && ptr.ptr + ptr.size < curr_next) {
		write_header(data, curr_size, ptr.ptr, curr);
		write_header(data, ptr.size, curr_next, ptr.ptr);
	}
	return 0;
}

/**
 * Helper function for efree()
 * 
 * Handles the case where the pointer to be freed is after the last free header.
 */
static int free_at_tail(struct ememory *memory, struct eptr ptr,
                        uint16_t curr, uint16_t curr_size) {
    char* data = memory->data;
	if (curr + curr_size == ptr.ptr) {
		write_header(data, curr_size + ptr.size, 0, curr);
	} else {
		write_header(data, curr_size, ptr.ptr, curr);
		write_header(data, ptr.size, 0, ptr.ptr);
	}
	return 0;
}

/**
 * Free a block at ptr in a free list of memory blocks
 */
int efree(struct ememory* memory, struct eptr ptr) {
	char* data = memory->data;
	uint16_t curr_size, curr_next, next_size, next_next;
	uint16_t curr = memory->free_head;

	if (ptr.ptr < curr) {
		return free_before_head(memory, ptr);
	}

	while (curr) {
		read_header(data, &curr_size, &curr_next, curr);

		// Our ptr is somewhere past the last free header
		if (curr_next == 0) {
			break;
		}

		read_header(data, &next_size, &next_next, curr_next);
		if (curr <= ptr.ptr && ptr.ptr <= curr_next) {
			return free_in_middle(memory, ptr, curr, curr_size, curr_next, next_size, next_next);
		}
		curr = curr_next;
	}

	return free_at_tail(memory, ptr, curr, curr_size);
} 

// In the future, implement the function below. This will allow us to simplify 
// our free logic to  mark block as free -> merge, rather than doing it all at 
// once.
//
// This function will most likely be very similar to the helper methods above.
int coalesce(struct ememory* memory, uint16_t ptr, uint16_t next, uint16_t prev) {
	return -1;
}

/**
 * IMPLEMENTATION DETAILS:
 * 
 * efree():
 *      Although it looks like a long function, the logic is mostly just insertion 
 * 		into a linked list with extra cases handled when free sections are perfectly 
 * 		adjacent. The linked list nodes are stored as headers in the memory blocks 
 * 		themselves and can be overwritten after allocation.
 * 
 * 		The full implementation logic is below.
 *      
 *      1. If ptr is before the current free head:
 *           a. If ptr + size == free_head:
 *                  - Merge with head: write header at ptr with
 *                    next = free_head.next, size = size + free_head.size
 *           b. Else:
 *                  - Insert before head: write header at ptr with
 *                    next = free_head, size = size
 *           c. Update free_head = ptr
 *      
 *      2. While iterating through the free list, for a position where curr <= ptr <= next,
 * 		   there are 4 cases:
 *           a. curr + curr.size == ptr && ptr + size == next
 *                  - Merge prev, ptr, and next into one block
 *                  - curr.next = next.next
 *                  - curr.size += ptr.size + next.size
 *           b. curr + curr.size < ptr && ptr + size == next
 *                  - Merge ptr with next
 *                  - curr.next = ptr
 *                  - write header at ptr with next = next.next, size = size + next.size
 *           c. curr + curr.size == ptr && ptr + size < next
 *                  - Merge curr with ptr
 *                  - curr.size += size
 *           d. curr + curr.size < ptr && ptr + size < next
 *                  - Insert without merging
 *                  - curr.next = ptr
 *                  - write header at ptr with next = next, size = size
 *      
 *      3. If iteration reaches the end of the list without returning:
 *           a. If ptr is adjacent to the last block (ptr == last + last.size):
 *                  - Merge with last: last.size += ptr.size
 *           b. Else:
 *                  - Append new block: last.next = ptr
 *                  - write header at ptr with next = 0, size = size
 */