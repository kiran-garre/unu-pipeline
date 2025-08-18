#ifndef MACRO_UTILS
#define MACRO_UTILS

/**
 * Used for displaying error codes, opcodes, and registers as strings.
 * 
 * To define a list of displayable codes:
 * 
 * #define LIST(X)		\
 * 		X(ENTRY1, 0)	\
 * 		X(ENTRY2, 1)
 * 
 * MACRO_TRACK(LIST) 			// for turning the LIST into a usable enum
 * MACRO_DISPLAY(LIST, to_str)	// for creating a function called to_str() to 
 * 								// convert the enum values to a string
 */

#define MACRO_TRACK(LIST) enum { LIST(MACRO_INST) };

#define MACRO_DISPLAY(LIST, FUNCNAME) 				\
	static inline char* FUNCNAME(int code) { 		\
		switch (code) { 							\
			LIST(STRING_CASE) 						\
			default: return "__";					\
		}											\
	}

#define MACRO_INST(NAME, VALUE) NAME = VALUE, 
#define STRING_CASE(NAME, VALUE) case VALUE: return #NAME;



/**
 * This allows us to count function arguments (up to 32) without the caller
 * specifying the number
 */

#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( 										\
	_1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, 	\
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, 	\
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30, 	\
	_31, _32, N, ...) N
#define PP_RSEQ_N() 										\
	32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 	\
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#endif // MACRO_UTILS