#ifndef DEBUGGER
#define DEBUGGER

#include "instructions.h"
#include "processor.h"
#include "disassembler.h"
#include <stdarg.h>

/**
 * Prints the instructions surrounding and including the instrction at PC
 */
void raw_context(struct processor* proc);

/**
 * raw_context(), but prettier
 */
void context(struct processor* proc);

/**
 * Prints only the contents of the requested registers
 */
void raw_regs_(struct processor* proc, int num_args, ...);

/**
 * raw_regs(), but prettier
 */
void regs_(struct processor* proc, int num_args, ...);

// Convenience macros
#define raw_regs(proc, ...) \
	raw_regs_(proc, PP_NARG(__VA_ARGS__), __VA_ARGS__)

#define regs(proc, ...) \
	regs_(proc, PP_NARG(__VA_ARGS__), __VA_ARGS__)

#define ALL_REGS 	R0, R1, R2, R3, R4, R5, R6, R7, EQ, PC

#endif // DEBUGGER