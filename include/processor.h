#ifndef PROCESSOR
#define PROCESSOR

#include "instructions.h"
#include "ememory.h"
#include "stages.h"
#include "pipeline.h"

struct processor {
	word_t regs[NUM_REGS];
	struct ememory* memory;
	uint64_t flag;

	struct IF_stage if_stage;
	struct ID_stage id_stage;
	struct MEM_stage mem_stage;
	struct EX_stage ex_stage;

	struct pipeline_ctrl pipeline_ctrl;
};

/**
 * Returns a new processor
 */
struct processor new_processor(struct ememory* memory);

/**
 * Begins execution of processor, starting at the ememory location loaded into
 * the program counter (proc->regs[PC])
 */
int run(struct processor* proc);

#endif // PROCESSOR