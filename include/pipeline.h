#ifndef PIPELINE
#define PIPELINE

#include "instructions.h"
#include "macro_utils.h"

/**
 * DETAILS:
 * 
 * This is a 5 stage instruction pipeline:
 * - Fetch
 * - Decode
 * - Execute
 * - Memory access
 * - Write back
 * 
 * Each struct contains the results of its stage. For example, the IF stage 
 * produces a fetched instruction as a result. Since the WB stage is the final
 * stage, it produces no results, and there's no struct dedicated to it.
 */

#define PIPELINE_ERRS(X) 			\
    X(INVALID_OPCODE, 	-100) 		\
    X(INVALID_REG,   	-101) 		\	
    X(SEGFAULT,  		-102)


#define ALU_OP(X)		\
	X(ALU_ADD, 	0)		\
	X(ALU_SUB, 	1)		\
	X(ALU_AND, 	2)		\
	X(ALU_OR, 	3)		\
	X(ALU_XOR, 	4)		\
	X(ALU_PASS, 5)

MACRO_TRACK(ALU_OP)
MACRO_DISPLAY(ALU_OP, op_to_str)

struct signal {
	unsigned char reg_write : 1;
	unsigned char mem_read  : 1;
	unsigned char mem_write : 1;
	unsigned char alu_op	: 3;
	unsigned char wb_src    : 1;	  // 0 for register, 1 for memory
};

struct IF_stage {
	struct instr fetched_instr;
	word_t prop_pc;

	char err_code;
};

struct ID_stage {
	char write_reg;
	word_t dest_data;
	word_t src1_data;
	word_t src2_data;

	struct signal sig;

	char err_code;
};

struct EX_stage {
	char write_reg;
	word_t dest_data;
	word_t alu_result;

	struct signal sig;
};

struct MEM_stage {
	char write_reg;
	word_t dest_data;
	word_t mem_result;
	word_t alu_result;

	struct signal sig;

	char err_code;
};

struct WB_stage {
	char err_code;
};

#endif // PIPELINE