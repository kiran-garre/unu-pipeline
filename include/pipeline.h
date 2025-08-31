#ifndef PIPELINE
#define PIPELINE

#include "instructions.h"
#include "macro_utils.h"
#include "stages.h"

// Forward declare processor
struct processor;

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
 * stage, it produces no results except for an error code.
 */

#define PIPELINE_ERRS(X) 			\
    X(INVALID_OPCODE, 	-100) 		\
    X(INVALID_REG,   	-101) 		\
	X(INVALID_OP, 		-102)		\
    X(SEGFAULT,  		-103)

MACRO_TRACK(PIPELINE_ERRS)
MACRO_DISPLAY(PIPELINE_ERRS, pipeline_err_to_string)

struct IF_stage fetch(struct processor* proc);

struct ID_stage decode(struct processor* proc, struct IF_stage fetched);

struct EX_stage execute(struct processor* proc, struct ID_stage decoded);

struct MEM_stage memory_access(struct processor* proc, struct EX_stage executed);

struct WB_stage write_back(struct processor* proc, struct MEM_stage accessed);

#endif // PIPELINE