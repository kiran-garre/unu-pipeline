#ifndef INSTRUCTIONS
#define INSTRUCTIONS

#include <stdint.h>
#include <stdlib.h>

#include "macro_utils.h"

// Constants:
#define word_t 			uint32_t
#define NUM_REGS		10
#define IGNR			0

// Opcodes:
#define OPCODES(X)		\
	X(LOAD, 	0)	 	\
	X(STORE, 	1)	 	\
	X(ADD, 		2)	 	\
	X(SUB, 		3)	 	\
	X(AND, 		4)	 	\
	X(OR, 		5)	 	\
	X(XOR, 		6)	 	\
	X(MOVR, 	7)	 	\
	X(MOVI, 	8)	 	\
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
	X(EQ,	9) 

MACRO_TRACK(REGISTERS)
MACRO_DISPLAY(REGISTERS, reg_to_str)

struct instr {
	unsigned char opcode : 7;
	unsigned char imm_flag : 1;
	unsigned char dest : 4;
	unsigned char src1 : 4;
	uint16_t imm;
};

#define INSTR(OPCODE, DEST, SRC1, SRC2) (struct instr){OPCODE, DEST, SRC1, SRC2}

#endif // INSTRUCTIONS

