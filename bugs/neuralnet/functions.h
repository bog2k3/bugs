#ifndef __functions_h__
#define __functions_h__

#include <map>
#include <string>

// standard model for all neural transfer functions:
// param value is the biased input value (sum of inputs*weights + bias)
typedef float (*transfer_function)(float biasedValue, float neuralParam, float input0, float bias);

enum class transferFuncNames {
	FN_ONE,
	FN_LN,
	FN_SIGMOID,
	FN_THRESHOLD,
	FN_SIN,
	FN_ABS,
	FN_RAND,
	FN_EXP,
	FN_POW,
	FN_GATE,
	FN_MODULATE,

	FN_MAXCOUNT			// this is the total number of functions
};

extern std::map<transferFuncNames, transfer_function> mapTransferFunctions;

// sin(value)
float transfer_fn_sin(float biasedValue, float neuralParam, float input0, float bias);

// abs - absolute (positive)
float transfer_fn_abs(float biasedValue, float neuralParam, float input0, float bias);

// value
float transfer_fn_one(float biasedValue, float neuralParam, float input0, float bias);

// ln(value)
float transfer_fn_ln(float biasedValue, float neuralParam, float input0, float bias);

// param^value
float transfer_fn_exp(float biasedValue, float neuralParam, float input0, float bias);

// value^param
float transfer_fn_pow(float biasedValue, float neuralParam, float input0, float bias);

// param: controls the steepness (param)
// tanh(value*param) -- a good param value is 10 -> allows smooth variance between [-0.1, +0.1] and snaps to +-1 outside [-0.2, +0.2]
float transfer_fn_sigmoid(float biasedValue, float neuralParam, float input0, float bias);

// param: controls the steepness of the sigmoid
// value > 0 ? value : 0
// continuous: value * sigmoid(value, param);
// threshold value is controlled indirectly by input bias
float transfer_fn_threshold(float biasedValue, float neuralParam, float input0, float bias);

// rand(value)
float transfer_fn_rand(float biasedValue, float neuralParam, float input0, float bias);

// param: sigmoid steepness
// input0:  command signal
// bias: command signal threshold
// lets value pass through if command signal is greater than command signal threshold.
float transfer_fn_gate(float biasedValue, float neuralParam, float input0, float bias);

// input0: command signal (input #0)
// multiplies the value with the command signal
float transfer_fn_modulate(float biasedValue, float neuralParam, float input0, float bias);

#endif // #ifndef __functions_h__
