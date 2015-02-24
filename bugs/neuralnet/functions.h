#ifndef __functions_h__
#define __functions_h__

#include <map>
#include <string>
// standard model for all neural transfer functions:
typedef float (*transfer_function)(float value, float constant);

enum class transferFuncNames {
	FN_ONE,
	FN_LN,
	FN_SIGM,
	FN_THRESHOLD,
	FN_SIN,
	FN_RAND,
	FN_EXP,
	FN_POW,
	FN_CONSTANT,

	FN_MAXCOUNT			// this is the total number of functions
};

extern std::map<transferFuncNames, transfer_function> mapTransferFunctions;

// sin(value)
float transfer_fn_sin(float value, float constant);

// value
float transfer_fn_one(float value, float constant);

// ln(value)
float transfer_fn_ln(float value, float constant);

// constant^value
float transfer_fn_exp(float value, float constant);

// value^constant
float transfer_fn_pow(float value, float constant);

// tanh(value*(constant+1))
float transfer_fn_sigmoid(float value, float constant);

// value > constant ? value : constant
float transfer_fn_threshold(float value, float constant);

// always constant
float transfer_fn_constant(float value, float constant);

// rand(value+constant)
float transfer_fn_rand(float value, float constant);

#endif // #ifndef __functions_h__
