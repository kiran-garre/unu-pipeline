#ifndef PROCESSOR
#define PROCESSOR

#include "instructions.h"
#include "ememory.h"
#include "errors.h"

struct processor {
	word_t regs[NUM_REGS];
	struct ememory* memory;
	uint64_t flag;
};

/**
 * Returns a new processor with uninitialized ememory
 */
struct processor new_processor(struct ememory* memory);

/**
 * Begins execution of processor, starting at the ememory location loaded into
 * the program counter (proc->regs[PC])
 */
int run(struct processor* proc);

#endif // PROCESSOR