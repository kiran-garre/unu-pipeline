#include "disassembler.h"

struct instr get_instr(char* memory, word_t offset) {
	struct instr res;
	memcpy(&res, memory + offset, sizeof(struct instr));
	return res;
}

void instr_to_str(struct instr* in, char* buf) {
	switch (in->opcode) {
		case LOAD:
		case STORE:
		case ADD:
		case SUB:
		case AND:
		case OR:
		case XOR:
			sprintf(buf, "%s, %s, %s, %s", 
				opcode_to_str(in->opcode), 
				reg_to_str(in->dest),
				reg_to_str(in->src1),
				reg_to_str(in->imm)
			);
			return;
		
		case MOVR:
		case BEQ:
		case BNE:
			sprintf(buf, "%s, %s, %s",
				opcode_to_str(in->opcode),
				reg_to_str(in->dest),
				reg_to_str(in->imm)
			);
			return;

		case MOVI:
			sprintf(buf, "%s, %s, %d",
				opcode_to_str(in->opcode),
				reg_to_str(in->dest),
				in->imm
			);
			return;

		case BRN:
			sprintf(buf, "%s, %s",
				opcode_to_str(in->opcode),
				reg_to_str(in->imm)
			);
			return;
		
		default: sprintf(buf, "INVALID OPCODE (%d)", in->opcode);
	}
}