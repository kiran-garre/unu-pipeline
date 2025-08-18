#ifndef ERRORS
#define ERRORS

#include "macro_utils.h"

#define ERROR_CODES(X) 				\
    X(INVALID_OPCODE_ERR, 	-1000) 	\
    X(INVALID_REG_ERR,   	-1001) 	\
    X(SEGFAULT,  			-1002)

MACRO_TRACK(ERROR_CODES)
MACRO_DISPLAY(ERROR_CODES, error_to_str)

#endif // ERRORS