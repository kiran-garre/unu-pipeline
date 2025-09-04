#ifndef STAGES
#define STAGES

#include "instructions.h"

#define ALU_OP(X)		\
	X(ALU_PASS, 0)		\
	X(ALU_ADD, 	1)		\
	X(ALU_SUB, 	2)		\
	X(ALU_AND, 	3)		\
	X(ALU_OR, 	4)		\
	X(ALU_XOR, 	5)		
	
MACRO_TRACK(ALU_OP)
MACRO_DISPLAY(ALU_OP, op_to_str)

struct signal {
	unsigned char reg_write : 1;
	unsigned char mem_read  : 1;
	unsigned char mem_write : 1;
	unsigned char alu_op	: 3;
	unsigned char wb_src    : 1;	  // 0 for register, 1 for memory
	unsigned char branch	: 1;
};

#define PIPELINE_ERR(CODE, FUNC, DUMP) \
	(struct err_base) {.err_code = CODE, .function_name = (char*) FUNC, .dump = DUMP}

struct err_base {
	int err_code;
	char* function_name;
	char* dump;
};

struct IF_stage {
	struct instr fetched_instr;
	word_t prop_pc;
	
	struct err_base err;
};

struct ID_stage {
	unsigned char write_reg;
	unsigned char branch_type;
	word_t dest_data;
	word_t src1_data;
	word_t src2_data;

	struct signal sig;

	struct err_base err;
};

struct EX_stage {
	unsigned char write_reg;
	word_t dest_data;
	int64_t alu_result;

	struct signal sig;

	struct err_base err;
};

struct MEM_stage {
	unsigned char write_reg;
	word_t dest_data;
	word_t mem_result;
	word_t alu_result;

	struct signal sig;

	struct err_base err;
};

struct WB_stage {
	struct err_base err;
};

struct pipeline_ctrl {
	unsigned char flush: 1;
	unsigned char stall: 1;
	unsigned char bubble_next: 1;
};

#endif // STAGES