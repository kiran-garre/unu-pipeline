#ifndef DISASSEMBLER
#define DISASSEMBLER

#include "instructions.h"
#include <stdio.h>
#include <string.h>

/**
 * Returns the offset (or word) at offset
 */
struct instr get_instr(char* memory, word_t offset);

/**
 * Converts an instruction to a displayable string
 */
void instr_to_str(struct instr* in, char* buf);


#endif // DISASSEMBLER