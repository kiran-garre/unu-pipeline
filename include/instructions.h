#ifndef INSTRUCTIONS
#define INSTRUCTIONS

#include <stdint.h>
#include <stdlib.h>
#include "macro_utils.h"

// Constants:
#define word_t 			uint32_t
#define NUM_REGS		10
#define NUM_OPCODES		12

// Opcodes:
#define OPCODES(X)		\
	X(MOV, 		0)	 	\
	X(LOAD, 	1)		\
	X(STORE, 	2)	 	\
	X(ADD, 		3)	 	\
	X(SUB, 		4)	 	\
	X(AND, 		5)	 	\
	X(OR, 		6)	 	\
	X(XOR, 		7)	 	\
	X(CMP, 		8)		\
	X(BEQ, 		9)	 	\
	X(BNE, 		10)	 	\
	X(BRN, 		11)	 

MACRO_TRACK(OPCODES)
MACRO_DISPLAY(OPCODES, opcode_to_str)

// Registers:
#define REGISTERS(X) 	\
	X(R0,	0) 			\
	X(R1,	1) 			\
	X(R2,	2) 			\
	X(R3,	3) 			\
	X(R4,	4) 			\
	X(R5,	5) 			\
	X(R6,	6) 			\
	X(R7,	7) 			\
	X(PC,	8) 			\
	X(FLAG, 9)

MACRO_TRACK(REGISTERS)
MACRO_DISPLAY(REGISTERS, reg_to_str)

struct instr {
	unsigned char opcode : 7;
	unsigned char imm_flag : 1;
	unsigned char dest : 4;
	unsigned char src1 : 4;
	int16_t src2;
};

#define READ_REG(PROC, REG) ((REG == FLAG) ? (PROC)->flag : (PROC)->regs[REG])
#define WRITE_REG(PROC, REG, VALUE) \
    ((REG) == FLAG ? ((PROC)->flag = (int64_t)(VALUE)) : ((PROC)->regs[(REG)] = (word_t)(VALUE)))


#endif // INSTRUCTIONS

