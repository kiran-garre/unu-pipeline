#include "processor.h"

// Forward declarations
int dispatch(struct processor* proc, struct instr in);
struct instr fetch(struct processor* proc);

struct processor new_processor(struct ememory* memory) {
	return (struct processor) { .memory = memory };
}

int execute_instr(struct processor* proc) {
	if (proc->regs[PC] < STARTING_OFFSET || MEM_SIZE <= proc->regs[PC]) {
		return SEGFAULT;
	}
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

struct instr read_be_instr(char* buf) {
	struct instr in;
	in.opcode = (buf[0] >> 1) & 0x7F;
	in.imm_flag = buf[0] & 0x1;
	in.dest = (buf[1] >> 4) & 0x0F;
	in.src1 = buf[1] & 0x0F;
	in.src2 = ((int16_t) buf[2] << 8) | buf[3];

	return in;
}

struct instr fetch(struct processor* proc) {
	char instr_buf[32];
	memcpy(instr_buf, proc->memory->data + proc->regs[PC], sizeof(struct instr));
	return read_be_instr(instr_buf);
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

	word_t base, offset;
	base = proc->regs[in.src1];
	if (in.imm_flag) {
		offset = in.src2;
	} else {
		VERIFY_REG(in.src2);
		offset = proc->regs[in.src2];
	}

	VERIFY_IN_BOUNDS(base + offset);
	memcpy(&proc->regs[in.dest], &proc->memory->data[base + offset], sizeof(word_t));
	return 0;
}

int store(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	VERIFY_REG(in.src1);

	word_t base, offset;
	base = proc->regs[in.src1];
	if (in.imm_flag) {
		offset = in.src2;
	} else {
		VERIFY_REG(in.src2);
		offset = proc->regs[in.src2];
	}

	VERIFY_IN_BOUNDS(base + offset);
	memcpy(&proc->memory->data[base + offset], &proc->regs[in.dest], sizeof(word_t));
	return 0;
}

int branch(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	word_t branch_to;
	if (in.imm_flag) {
		branch_to = proc->regs[in.dest] + in.src2;
	} else {
		VERIFY_REG(in.src2);
		branch_to = proc->regs[in.dest] + proc->regs[in.src2];
	}

	// We offset by instruction size because PC is updated after dispatch
	proc->regs[PC] = branch_to - sizeof(struct instr);
	return 0;
}

/**
 * This allows us to define binop functions and switch out the operator.
 * 
 * This is DISGUSTING and should never be done again. Except like 15 lines down.
 */
#define DEFINE_BINOP(NAME, OP) 													\
	int NAME(struct processor* proc, struct instr in) { 						\
		VERIFY_REG(in.dest); 													\
		VERIFY_REG(in.src1); 													\
		if (in.imm_flag) {														\
			proc->regs[in.dest] = proc->regs[in.src1] OP in.src2;				\
		} else {																\
			VERIFY_REG(in.src2);												\
			proc->regs[in.dest] = proc->regs[in.src1] OP proc->regs[in.src2];	\
		}																		\
		return 0; 																\
	}

#define DEFINE_BRANCH(NAME, CMP)							\
	int NAME(struct processor* proc, struct instr in) {		\
		if (proc->flag CMP 0) {								\
			return branch(proc, in);						\
		}													\
		return 0;											\
	}

DEFINE_BINOP(add, +)
DEFINE_BINOP(sub, -)
DEFINE_BINOP(and, &)
DEFINE_BINOP(or,  |)
DEFINE_BINOP(xor, ^)

DEFINE_BRANCH(beq, ==)
DEFINE_BRANCH(bne, !=)

int mov(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	if (in.imm_flag) {														
		proc->regs[in.dest] = in.src2;			
	} else {																					
		VERIFY_REG(in.src2);												
		proc->regs[in.dest] = proc->regs[in.src2];
	}		
	return 0;
} 

int cmp(struct processor* proc, struct instr in) {
	VERIFY_REG(in.dest);
	if (in.imm_flag) {														
		proc->flag = proc->regs[in.dest] - in.src2;			
	} else {																					
		VERIFY_REG(in.src2);												
		proc->flag = proc->regs[in.dest] - proc->regs[in.src2];	
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
		case MOV:
			return mov(proc, in);
		case CMP:
			return cmp(proc, in);
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