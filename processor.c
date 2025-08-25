#include "processor.h"
#include "pipeline.h"
#include <stdio.h>

struct processor new_processor(struct ememory* memory) {
	return (struct processor) { .memory = memory };
}

#define CHECK_ERR(STAGE) if (STAGE.err_code) { return STAGE.err_code; }

int clock_cycle(struct processor* proc) {
	struct WB_stage wb_stage = write_back(proc, proc->mem_stage);
	CHECK_ERR(wb_stage);
	proc->mem_stage = memory_access(proc, proc->ex_stage);
	CHECK_ERR(proc->mem_stage);
	proc->ex_stage = execute(proc, proc->id_stage);
	CHECK_ERR(proc->ex_stage);
	proc->id_stage = decode(proc, proc->if_stage);
	CHECK_ERR(proc->id_stage);
	proc->if_stage = fetch(proc);
	CHECK_ERR(proc->if_stage);
	return 0;
}

int run(struct processor* proc) {
	int status;
	while ((status = clock_cycle(proc)) == 0)
		;
	return status;
}
