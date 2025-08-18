#include "processor.h"
#include "instructions.h"
#include "errors.h"
#include "debugger/debugger.h"
#include <stdio.h>

int main() {
	// R0 = destination address
	struct instr copy_loop[10] = {
		INSTR(MOVR, R1, IGNR, PC),		
		INSTR(MOVI, R2, IGNR, sizeof(copy_loop)),
		INSTR(MOVI, R3, IGNR, 0),					// R3 = counter, starting at 1
		INSTR(MOVI, R6, IGNR, sizeof(word_t)),		// R6 = word size

		INSTR(MOVR, R4, IGNR, PC),				// R4 = address of this instruction
		INSTR(LOAD, R5, R1, R3),				// R5 = word at R1 + R3
		INSTR(STORE, R5, R0, R3),				// Write to destination R0 + R3
		INSTR(ADD, R3, R3, R6),					// Increment counter
		INSTR(SUB, EQ, R2, R3),					// Compare counter to size
		INSTR(BNE, EQ, IGNR, R4),				// Loop if not equal
		// INSTR(BRN, IGNR, IGNR, PC),
	};

	char data[MEM_SIZE];
	struct ememory memory = { .data = data };
	init_ememory(&memory, MEM_SIZE);

	struct processor proc = new_processor(&memory);

	struct eptr initial_ptr = emalloc(&memory, sizeof(copy_loop));
	memcpy(data + initial_ptr.ptr, copy_loop, sizeof(copy_loop));
	proc.regs[PC] = STARTING_OFFSET;

	struct eptr ptr = emalloc(&memory, sizeof(copy_loop));
	if (is_null(ptr)) {
		printf("Could not allocate memory for copy\n");
		return 1;
	}

	proc.regs[R0] = ptr.ptr;

	// We don't loop at the end of the instructions above, so the processor
	// will crash once they're executed. 
	int status = run(&proc);

	if (memcmp(copy_loop, &data[ptr.ptr], sizeof(copy_loop)) == 0) {
		printf("Memory copied successfully!\n");
		return 0;
	} else {
		printf("Memory failed to copy.\n");
		return 1;
	}
}