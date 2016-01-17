#include "functions.h"
#include <math.h>
#include "../math/math2D.h"
#include "../utils/rand.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

std::map<transferFuncNames, transfer_function> mapTransferFunctions;

int initNeuralFunctionMap() {
	mapTransferFunctions[transferFuncNames::FN_ONE] = &transfer_fn_one;
	mapTransferFunctions[transferFuncNames::FN_LN] = &transfer_fn_ln;
	mapTransferFunctions[transferFuncNames::FN_SIGM] = &transfer_fn_sigmoid;
	mapTransferFunctions[transferFuncNames::FN_SIN] = &transfer_fn_sin;
	mapTransferFunctions[transferFuncNames::FN_RAND] = &transfer_fn_rand;
	mapTransferFunctions[transferFuncNames::FN_EXP] = &transfer_fn_exp;
	mapTransferFunctions[transferFuncNames::FN_POW] = &transfer_fn_pow;
	mapTransferFunctions[transferFuncNames::FN_CONSTANT] = &transfer_fn_constant;

	return 0;
}

int dummyToInitMap = initNeuralFunctionMap();

// value
float transfer_fn_one(float value, float constant) {
	return value;
}

// always constant
float transfer_fn_constant(float value, float constant) {
	return constant;
}

// sin(value)
float transfer_fn_sin(float value, float arg) {
	return sin(value);
}

// ln(value)
float transfer_fn_ln(float value, float arg) {
	if (value <= 0)
		return -1.e+10f;
	else
		return log(value);
}

// arg^value
float transfer_fn_exp(float value, float arg) {
	if (arg < 0)
		return pow(arg, (int)value);
	else
		return pow(arg, value);
}

// value^arg
float transfer_fn_pow(float value, float arg) {
	if (value < 0)
		return pow(value, (int)arg);
	else
		return pow(value, arg);
}

// rand(value)
float transfer_fn_rand(float value, float arg) {
	return randf() * value;
}

// tanh(value*(arg+1))
float transfer_fn_sigmoid(float value, float arg) {
	return tanhf(value*(arg+1));
}
