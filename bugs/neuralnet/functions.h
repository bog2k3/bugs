#ifndef __functions_h__
#define __functions_h__

#include <map>
#include <string>
// standard model for all neural transfer functions:
typedef float (*transfer_function)(float value, float param);

enum class transferFuncNames {
	FN_ONE,
	FN_LN,
	FN_SIGM,
	FN_THRESHOLD,
	FN_SIN,
	FN_RAND,
	FN_EXP,
	FN_POW,
	FN_GATE,
	FN_MODULATE,

	FN_MAXCOUNT			// this is the total number of functions
};

extern std::map<transferFuncNames, transfer_function> mapTransferFunctions;

// sin(value)
float transfer_fn_sin(float value, float param);

// value
float transfer_fn_one(float value, float param);

// ln(value)
float transfer_fn_ln(float value, float param);

// param^value
float transfer_fn_exp(float value, float param);

// value^param
float transfer_fn_pow(float value, float param);

// tanh(value*param) -- a good param value is 10 -> allows smooth variance between [-0.1, +0.1] and snaps to +-1 outside [-0.2, +0.2]
float transfer_fn_sigmoid(float value, float param);

// value > 0 ? value : 0
// continuous: value * sigmoid(sum, 10);
float transfer_fn_threshold(float value, float param);

// rand(value)
float transfer_fn_rand(float value, float param);

#endif // #ifndef __functions_h__
