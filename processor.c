#include "processor.h"

// Forward declarations
int dispatch(struct processor* proc, struct instr in);
struct instr fetch(struct processor* proc);

struct processor new_processor(struct ememory* memory) {
	return (struct processor) { .memory = memory };
}

int execute_instr(struct processor* proc) {
	struct instr in = fetch(proc);
	int res = dispatch(proc, in);
	proc->regs[PC] += sizeof(struct instr);
	return res;
}

int run(struct processor* proc) {
	int status;
	while ((status = execute_instr(proc)) == 0)
		;
	return status;
}

struct instr fetch(struct processor* proc) {
	struct instr res;
	memcpy(&res, proc->memory->data + proc->regs[PC], sizeof(struct instr));
	return res;
}

#define VERIFY_REG(VALUE) do { 				\
	if ((VALUE < 0 || VALUE >= NUM_REGS)) 	\
		return INVALID_REG_ERR; 			\
	} while (0)	

#define VERIFY_IN_BOUNDS(VALUE) do {						\
	if (VALUE < STARTING_OFFSET || VALUE >= MEM_SIZE)		\
		return SEGFAULT;									\
	} while (0)

int load(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	word_t base = proc->regs[in.src1];
	word_t offset = proc->regs[in.imm];
	VERIFY_IN_BOUNDS(base + offset);

	memcpy(&proc->regs[in.dest], &proc->memory->data[base + offset], sizeof(word_t));
	return 0;
}

int store(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	word_t base = proc->regs[in.src1];
	word_t offset = proc->regs[in.imm];
	VERIFY_IN_BOUNDS(base + offset);

	memcpy(&proc->memory->data[base + offset], &proc->regs[in.dest], sizeof(word_t));
	return 0;
}

int add(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	proc->regs[in.dest] = proc->regs[in.src1] + proc->regs[in.imm];
	return 0;
}

int sub(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	proc->regs[in.dest] = proc->regs[in.src1] - proc->regs[in.imm];
	return 0;
} 

int and(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	proc->regs[in.dest] = proc->regs[in.src1] & proc->regs[in.imm];
	return 0;
}

int or(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	proc->regs[in.dest] = proc->regs[in.src1] | proc->regs[in.imm];
	return 0;
}

int xor(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);
	VERIFY_REG(in.imm);

	proc->regs[in.dest] = proc->regs[in.src1] ^ proc->regs[in.imm];
	return 0;
} 

int movi(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);

	proc->regs[in.dest] = in.imm;
	return 0;
} 

int movr(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.imm);

	proc->regs[in.dest] = proc->regs[in.imm];
	return 0;
} 

int branch(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.imm);
	proc->regs[PC] = proc->regs[in.imm] - sizeof(struct instr);
	return 0;
}

int beq(struct processor* proc, struct instr in) {
	if (proc->regs[in.dest] == 0) {
		return branch(proc, in);
	}
	return 0;
}

int bne(struct processor* proc, struct instr in) {
	if (proc->regs[in.dest] != 0) {
		return branch(proc, in);
	}
	return 0;
}

int dispatch(struct processor* proc, struct instr in) {
	switch (in.opcode) {
		case LOAD:
			return load(proc, in);
		case STORE:
			return store(proc, in);
		case ADD:
			return add(proc, in);
		case SUB:
			return sub(proc, in);
		case AND:
			return and(proc, in);
		case OR:
			return or(proc, in);
		case XOR:
			return xor(proc, in);
		case MOVI:
			return movi(proc, in);
		case MOVR:
			return movr(proc, in);
		case BEQ: 
			return beq(proc, in);
		case BNE:
			return bne(proc, in);
		case BRN:
			return branch(proc, in);
		default:
			return INVALID_OPCODE_ERR;
	}
}