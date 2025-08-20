#include "pipeline.h"


struct IF_stage fetch(struct processor* proc) {
	struct instr in;
	memcpy(&in, proc->memory->data, sizeof(struct instr));
	return (struct IF_stage) { in };
}

struct ID_stage decode(struct IF_stage fetched) {

}

struct EX_stage execute(struct ID_stage decoded) {

}

struct MEM_stage memory_access(struct EX_stage executed) {

}

void write_back(struct MEM_stage accessed) {

}

struct signal instr_to_signal(struct instr in) {
	struct signal sig;
	
}

char opcode_to_op(char opcode) {
	
}
/**
 * LOAD: 
 * - reg_write = true
 * - mem_read = true
 * - mem_write = false
 * - alu_op = ADD
 * - alu_src = immediate flag from instruction
 * - wb_src = memory
 * 
 * STORE:
 * - reg_write = false
 * - mem_read = false
 * - mem_write = true
 * - alu_op = ADD
 * - alu_src = immediate flag from instruction
 * - wb_src = memory
 * 		
 * ADD | SUB | AND | OR | XOR
 * - reg_write = true
 * - mem_read = false
 * - mem_write = false
 * - alu_op = OP
 * - alu_src = immediate flag from instruction
 * - wb_src = register
 * 
 * MOV
 * - reg
 */