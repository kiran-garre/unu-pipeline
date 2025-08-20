#include "processor.h"
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

#define ALU_OP(X)	\
	X(ADD, 	0)		\		
	X(SUB, 	1)		\
	X(AND, 	2)		\
	X(OR, 	3)		\
	X(XOR, 	4)	

MACRO_TRACK(ALU_OP)
MACRO_DISPLAY(ALU_OP, op_to_str)

struct signal {
	char reg_write : 1;
	char mem_read  : 1;
	char mem_write : 1;
	char alu_op	   : 3;
	char alu_src   : 1;	  // 0 for register, 1 for immediate
	char wb_src    : 1;	  // 0 for register, 1 for memory
};

struct IF_stage {
	struct instr fetched_instr;
};

struct ID_stage {
	word_t dest;
	word_t src1;
	word_t imm;
	struct signal sig;
};

struct EX_stage {
	word_t dest;
	word_t result;
	struct signal sig;
};

struct MEM_stage {
	word_t dest;
	word_t mem_result;
	word_t alu_result;
	struct signal sig;
};

