#include "pipeline.h"
#include "processor.h"

// ================================
//		 FORWARD DECLARATIONS
// ================================

struct instr read_be_instr(char* buf);
struct signal instr_to_signal(struct instr* in);

// Declare macros to silence errors
#define CHECK_STAGE_ERR(...)
#define ASSIGN_PROP_PC(...)
#define ALU_SIGNAL(...)


// =============================
//		 PIPELINE HANDLERS
// =============================

struct IF_stage fetch(struct processor* proc) {
	CHECK_STAGE_ERR(struct IF_stage, verify_in_bounds(proc->regs[PC]));
	struct instr in = read_be_instr(proc->memory->data + proc->regs[PC]);
	return (struct IF_stage) { .fetched_instr = in, .prop_pc = proc->regs[PC] };
}

struct ID_stage decode(struct processor* proc, struct IF_stage fetched) {
	struct instr* in = &fetched.fetched_instr;

	CHECK_STAGE_ERR(struct ID_stage, verify_reg(in->dest));
	CHECK_STAGE_ERR(struct ID_stage, verify_reg(in->src1));

	struct signal sig = instr_to_signal(in);
	
	word_t dest_data, src1_data, src2_data;
	ASSIGN_PROP_PC(dest_data, in->dest);
	ASSIGN_PROP_PC(src1_data, in->src1);
	if (in->imm_flag) {
		src2_data = in->src2;
	} else {
		CHECK_STAGE_ERR(struct ID_stage, verify_reg(in->src2));
		ASSIGN_PROP_PC(src2_data, in->src2);
	}

	return (struct ID_stage) {
		.write_reg = in->dest, 
		.dest_data = dest_data,
		.src1_data = src1_data,
		.src2_data = src2_data
	};
}

struct signal instr_to_signal(struct instr* in) {
	switch (in->opcode) {
		case LOAD:
			return (struct signal) { 
				.reg_write = 1, 
				.mem_read = 1, 
				.mem_write = 0, 
				.alu_op = ALU_ADD, 
				.wb_src = 1 
			};
		case STORE:
			return (struct signal) { 
				.reg_write = 0, 
				.mem_read = 0, 
				.mem_write = 1, 
				.alu_op = ALU_ADD,  
				.wb_src = 0 
			};
		case ADD:
			ALU_SIGNAL(ALU_ADD);
		case SUB:
			ALU_SIGNAL(ALU_SUB);
		case AND:
			ALU_SIGNAL(ALU_AND);
		case OR:
			ALU_SIGNAL(ALU_OR);
		case XOR:
			ALU_SIGNAL(ALU_XOR);
		case MOV:
			ALU_SIGNAL(ALU_PASS);
		case CMP:
			ALU_SIGNAL(ALU_SUB);
		case BRN:
		case BNE:
		case BEQ:
			return (struct signal) { 
				.reg_write = 0, 
				.mem_read = 0, 
				.mem_write = 0, 
				.alu_op = ALU_ADD, 
				.wb_src = 0 
			};
		
	}
} 

struct EX_stage execute(struct ID_stage decoded) {
	word_t alu_result;
	switch (decoded.sig.alu_op) {
		case ALU_ADD:
			alu_result = decoded.src1_data + decoded.src2_data;
		case ALU_SUB:
			alu_result = decoded.src1_data - decoded.src2_data;
		case ALU_AND:
			alu_result = decoded.src1_data & decoded.src2_data;
		case ALU_OR:
			alu_result = decoded.src1_data | decoded.src2_data;
		case ALU_XOR:
			alu_result = decoded.src1_data ^ decoded.src2_data;
		case ALU_PASS:
			alu_result = decoded.src2_data;
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
		mem_result = proc->memory->data[executed.alu_result];
	} else if (executed.sig.mem_write) {
		CHECK_STAGE_ERR(struct MEM_stage, verify_in_bounds(executed.alu_result));
		proc->memory->data[executed.alu_result] = executed.dest_data;
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
			proc->regs[accessed.write_reg] = accessed.mem_result;
		} else {
			proc->regs[accessed.write_reg] = accessed.alu_result;
		}
	}
}


// ============================
//		 HELPER FUNCTIONS
// ============================

#define ALU_SIGNAL(OP)				\
	return (struct signal) { 		\
		.reg_write = 1, 			\
		.mem_read = 0, 				\
		.mem_write = 0, 			\
		.alu_op = OP, 				\
		.wb_src = 0 				\
	}

#define ASSIGN_PROP_PC(VAR, REG)	\
	if (REG == PC) {				\
		VAR = fetched.prop_pc;		\
	} else {						\
		VAR = proc->regs[REG];		\
	}

#define CHECK_STAGE_ERR(STAGE, ERR_CODE)			\
	if (ERR_CODE) {									\
		return (STAGE) { .err_code = ERR_CODE };	\
	}						
	
static inline char verify_reg(word_t value) {
	if ((value < 0 || value >= NUM_REGS)) {
		return INVALID_OPCODE_ERR; 			
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

char min(char x, char y) {
	return (x < y) ? x : y;
}