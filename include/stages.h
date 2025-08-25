#ifndef STAGES
#define STAGES

#include "instructions.h"

struct signal {
	unsigned char reg_write : 1;
	unsigned char mem_read  : 1;
	unsigned char mem_write : 1;
	unsigned char alu_op	: 3;
	unsigned char wb_src    : 1;	  // 0 for register, 1 for memory
	unsigned char branch	: 1;
};

struct IF_stage {
	struct instr fetched_instr;
	word_t prop_pc;
	
	char err_code;
};

struct ID_stage {
	unsigned char write_reg;
	word_t dest_data;
	word_t src1_data;
	word_t src2_data;

	struct signal sig;

	char err_code;
};

struct EX_stage {
	unsigned char write_reg;
	word_t dest_data;
	word_t alu_result;

	struct signal sig;

	char err_code;
};

struct MEM_stage {
	unsigned char write_reg;
	word_t dest_data;
	word_t mem_result;
	word_t alu_result;

	struct signal sig;

	char err_code;
};

struct WB_stage {
	char err_code;
};

#endif // STAGES