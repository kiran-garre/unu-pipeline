#include "pipeline.h"
#include "processor.h"
#include <stdio.h>

// ============================
//		 HELPER FUNCTIONS
// ============================

#define RETURN_ALU_SIGNAL(OP)		\
	return (struct signal) { 		\
		.reg_write = 1, 			\
		.mem_read = 0, 				\
		.mem_write = 0, 			\
		.alu_op = OP, 				\
		.wb_src = 0, 				\
		.branch = 0					\
	}

#define ASSIGN_REG_OR_PROP_PC(VAR, REG)		\
	if (REG == PC) {						\
		VAR = fetched.prop_pc;				\
	} else {								\
		VAR = READ_REG(proc, REG);			\
	}

#define CHECK_STAGE_ERR(STAGE, ERR_CODE)					\
	if (ERR_CODE) {											\
		return (STAGE) { 									\
			.err = PIPELINE_ERR(ERR_CODE, __func__, NULL)	\
		};													\
	}						
	
static inline char verify_reg(word_t value) {
	if (value < 0 || value >= NUM_REGS) {
		return INVALID_REG; 			
	}
	return 0;
}

static inline char verify_in_bounds(word_t value) {
	if (value < STARTING_OFFSET || value >= MEM_SIZE) {
		return SEGFAULT;
	}
	return 0;
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

struct signal instr_to_signal(struct instr* in) {
	switch (in->opcode) {
		// Reorder instruction arguments
		// This is a hacky solution for letting CMP results to go to the 
		// flag register and unifying syntax between CMP, MOV, BRN, etc.
		case BRN:
		case BNE:
		case BEQ:
			in->src1 = in->dest;
			break;
		case CMP:
			in->src1 = in->dest;
			in->dest = FLAG;
			break;
	}

	switch (in->opcode) {
		case LOAD:
			return (struct signal) { 
				.reg_write = 1, 
				.mem_read = 1, 
				.mem_write = 0, 
				.alu_op = ALU_ADD, 
				.wb_src = 1,
				.branch = 0 
			};
		case STORE:
			return (struct signal) { 
				.reg_write = 0, 
				.mem_read = 0, 
				.mem_write = 1, 
				.alu_op = ALU_ADD,  
				.wb_src = 0,
				.branch = 0
			};
		case ADD:
			RETURN_ALU_SIGNAL(ALU_ADD);
		case SUB:
			RETURN_ALU_SIGNAL(ALU_SUB);
		case AND:
			RETURN_ALU_SIGNAL(ALU_AND);
		case OR:
			RETURN_ALU_SIGNAL(ALU_OR);
		case XOR:
			RETURN_ALU_SIGNAL(ALU_XOR);
		case MOV:
			RETURN_ALU_SIGNAL(ALU_PASS);
		case CMP:
			RETURN_ALU_SIGNAL(ALU_SUB);
		case BRN:
		case BNE:
		case BEQ:
			return (struct signal) { 
				.reg_write = 0, 
				.mem_read = 0, 
				.mem_write = 0, 
				.alu_op = ALU_ADD, 
				.wb_src = 0, 
				.branch = 1
			};
	}
} 

char evaluate_cmp(int64_t flag, unsigned char branch_type) {
	switch (branch_type) {
		case BRN:
			return 1;
		case BEQ:
			return (flag == 0);
		case BNE:
			return (flag != 0);
	}
	return 0;
}


// =============================
//		 PIPELINE HANDLERS
// =============================

struct IF_stage fetch(struct processor* proc) {
	CHECK_STAGE_ERR(struct IF_stage, verify_in_bounds(proc->regs[PC]));
	struct instr in = read_be_instr(proc->memory->data + proc->regs[PC]);
	proc->regs[PC] += sizeof(struct instr);
	return (struct IF_stage) { .fetched_instr = in, .prop_pc = proc->regs[PC] - sizeof(struct instr) };
}

struct ID_stage decode(struct processor* proc, struct IF_stage fetched) {
	struct instr* in = &fetched.fetched_instr;

	if (in->opcode < 0 || in->opcode >= NUM_OPCODES) {
		return (struct ID_stage) { .err = PIPELINE_ERR(INVALID_OPCODE, __func__, NULL)};
	}

	CHECK_STAGE_ERR(struct ID_stage, verify_reg(in->dest));
	CHECK_STAGE_ERR(struct ID_stage, verify_reg(in->src1));

	struct signal sig = instr_to_signal(in);
	
	word_t dest_data, src1_data, src2_data;
	ASSIGN_REG_OR_PROP_PC(dest_data, in->dest);
	ASSIGN_REG_OR_PROP_PC(src1_data, in->src1);
	if (in->imm_flag) {
		src2_data = in->src2;
	} else {
		CHECK_STAGE_ERR(struct ID_stage, verify_reg(in->src2));
		ASSIGN_REG_OR_PROP_PC(src2_data, in->src2);
	}

	if (in->opcode == CMP) {
		printf("See CMP!\n");
		proc->pipeline_ctrl.stall = 1;
		proc->pipeline_ctrl.bubble_next = 1;
	}

	return (struct ID_stage) {
		.write_reg = in->dest, 
		.branch_type = in->opcode,
		.dest_data = dest_data,
		.src1_data = src1_data,
		.src2_data = src2_data,
		.sig = sig,
	};
}

struct EX_stage execute(struct processor* proc, struct ID_stage decoded) {
	int64_t alu_result;
	switch (decoded.sig.alu_op) {
		case ALU_PASS:
			alu_result = decoded.src2_data;
			break;
		case ALU_ADD:
			alu_result = decoded.src1_data + decoded.src2_data;
			break;
		case ALU_SUB:
			alu_result = decoded.src1_data - decoded.src2_data;
			break;
		case ALU_AND:
			alu_result = decoded.src1_data & decoded.src2_data;
			break;
		case ALU_OR:
			alu_result = decoded.src1_data | decoded.src2_data;
			break;
		case ALU_XOR:
			alu_result = decoded.src1_data ^ decoded.src2_data;
			break;	
	}

	if (decoded.sig.mem_read) {
		printf("ALU result on load: %lld\n", alu_result);
	}

	if (decoded.sig.branch) {
		printf("Branching to %d (flag = %d)\n", alu_result, proc->flag);
		if (evaluate_cmp(proc->flag, decoded.branch_type)) {
			WRITE_REG(proc, PC, alu_result);
			proc->pipeline_ctrl.flush = 1;
		}
	}

	return (struct EX_stage) { 
		.write_reg = decoded.write_reg, 
		.dest_data = decoded.dest_data, 
		.alu_result = alu_result, 
		.sig = decoded.sig
	};
}

struct MEM_stage memory_access(struct processor* proc, struct EX_stage executed) {
	word_t mem_result = 0;
	if (executed.sig.mem_read) {
		CHECK_STAGE_ERR(struct MEM_stage, verify_in_bounds(executed.alu_result));
		memcpy(&mem_result, &proc->memory->data[executed.alu_result], sizeof(word_t));
	} else if (executed.sig.mem_write) {
		CHECK_STAGE_ERR(struct MEM_stage, verify_in_bounds(executed.alu_result));
		memcpy(&proc->memory->data[executed.alu_result], &executed.dest_data, sizeof(word_t));
	}

	return (struct MEM_stage) {
		.write_reg = executed.write_reg, 
		.dest_data = executed.dest_data, 
		.mem_result = mem_result, 
		.alu_result = executed.alu_result,
		.sig = executed.sig,
	};
}

struct WB_stage write_back(struct processor* proc, struct MEM_stage accessed) {
	if (accessed.sig.reg_write) {
		CHECK_STAGE_ERR(struct WB_stage, verify_reg(accessed.write_reg));
		if (accessed.sig.wb_src) {
			WRITE_REG(proc, accessed.write_reg, accessed.mem_result);
		} else {
			WRITE_REG(proc, accessed.write_reg, accessed.alu_result);
		}
	}
	return (struct WB_stage) { 0 };
}
