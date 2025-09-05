#include "processor.h"
#include "pipeline.h"
#include "debugger.h"
#include <stdio.h>

struct processor new_processor(struct ememory* memory) {
	return (struct processor) { .memory = memory };
}

#define CHECK_ERR(STAGE) 														\
	if (STAGE.err.err_code) { 													\
		printf("Pipeline error occurred in %s()\n", STAGE.err.function_name); 	\
		return STAGE.err.err_code; 												\
	}

static inline void flush_stage(void* stage, unsigned int stage_size) {
	memset(stage, 0, stage_size);
}

/**
 * This is more macro abuse. This should never be done.
 */
#define EXECUTE_CTRL(PROC, STAGE, FN_STMT)		\
	if (!proc->pipeline_ctrl.stall) {			\
		STAGE = FN_STMT;						\
	}											\
	if (proc->pipeline_ctrl.flush) {			\
		flush_stage(&STAGE, sizeof(STAGE));		\
	}											\
	CHECK_ERR(STAGE)					

int clock_cycle(struct processor* proc) {

	// Reset ctrl on every cycle
	proc->pipeline_ctrl.flush = 0;
	proc->pipeline_ctrl.stall = 0;

	struct WB_stage wb_stage;
	EXECUTE_CTRL(proc, wb_stage, write_back(proc, proc->mem_stage));
	EXECUTE_CTRL(proc, proc->mem_stage, memory_access(proc, proc->ex_stage));
	EXECUTE_CTRL(proc, proc->ex_stage, execute(proc, proc->id_stage));
	
	if (proc->pipeline_ctrl.bubble_next) {
		flush_stage(&proc->id_stage, sizeof(struct ID_stage));
		proc->pipeline_ctrl.bubble_next = 0;
	} else {
		EXECUTE_CTRL(proc, proc->id_stage, decode(proc, proc->if_stage));
	}
	
	EXECUTE_CTRL(proc, proc->if_stage, fetch(proc));

	char buf[64];
	// instr_to_str(&proc->if_stage.dbg.in, buf);
	// printf("\nIF instruction: %s\n", buf);
	// instr_to_str(&proc->id_stage.dbg.in, buf);
	// printf("ID instruction: %s\n", buf);
	// instr_to_str(&proc->ex_stage.dbg.in, buf);
	// printf("EX instruction: %s\n", buf);
	// instr_to_str(&proc->mem_stage.dbg.in, buf);
	// printf("MEM instruction: %s\n", buf);
	return 0;	
}

int run(struct processor* proc) {
	int status;
	while ((status = clock_cycle(proc)) == 0)
		regs(proc, ALL_REGS);
		;
	return status;
}
