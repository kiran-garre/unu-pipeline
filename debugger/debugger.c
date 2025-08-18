#include "debugger.h"

#define CONTEXT_RELATIVE_START 		-1
#define CONTEXT_LEN					4	

static word_t adjusted_pc(word_t offset) {
	return offset - sizeof(struct instr);
}

char (*gather_context(struct processor* proc))[32] {
	word_t initial_offset = adjusted_pc(proc->regs[PC]);
	char (*bufs)[32] = malloc(sizeof(char[CONTEXT_LEN][32]));

	for (int i = 0; i < CONTEXT_LEN; i++) {
		int offset = initial_offset + (i + CONTEXT_RELATIVE_START) * sizeof(struct instr);
		struct instr in = get_instr(proc->memory->data, offset);
		instr_to_str(&in, bufs[i]);
	}
	return bufs;
}

void raw_context(struct processor* proc) {
	char (*bufs)[32] = gather_context(proc);
	for (int i = 0; i < CONTEXT_LEN; i++) {
		printf("%s\n", bufs[i]);
	}
	free(bufs);
}

void context(struct processor* proc) {
	char (*bufs)[32] = gather_context(proc);
	word_t initial_offset = adjusted_pc(proc->regs[PC]);

	printf("CONTEXT: ----------------------------\n");
	for (int i = 0; i < CONTEXT_LEN; i++) {
		word_t offset = initial_offset + (i + CONTEXT_RELATIVE_START) * sizeof(struct instr);
		if (offset == initial_offset) {
			printf("->  ");
		} else {
			printf("    ");
		}
		printf("0x%08x:\t%s\n", offset, bufs[i]);
	}
	printf("-------------------------------------\n");
	free(bufs);
}

word_t* get_reg_values(struct processor* proc, int num_args, va_list argptr) {
	word_t* values = malloc(num_args);

	for (int i = 0; i < num_args; i++) {
		int reg = va_arg(argptr, int);
		if (reg < 0 || reg > NUM_REGS) {
			return NULL;
		}
		values[i] = proc->regs[reg];
	}

	va_end(argptr);
	return values;
}

void raw_regs_(struct processor* proc, int num_args, ...) {
	va_list argptr;
 	va_start(argptr, num_args);
	word_t* regs = get_reg_values(proc, num_args, argptr);
	if (!regs) {
		printf("raw_regs(): Invalid register requested\n");
		return;
	}
	va_end(argptr);

	for (int i = 0; i < num_args; i++) {
		printf("%d\n", regs[i]);
	}
}

void regs_(struct processor* proc, int num_args, ...) {
	va_list argptr;
 	va_start(argptr, num_args);
	word_t* regs = get_reg_values(proc, num_args, argptr);
	if (!regs) {
		printf("regs(): Invalid register requested\n");
		return;
	}
	va_end(argptr);

 	va_start(argptr, num_args);

	printf("REGISTERS: --------------------------\n");
	for (int i = 0; i < num_args; i++) {
		printf("    %s: %-4d (0x%x)\n", reg_to_str(va_arg(argptr, int)), regs[i], regs[i]);
	}
	printf("-------------------------------------\n");
	
	va_end(argptr);
	free(regs);
}