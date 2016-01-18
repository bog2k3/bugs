#include "functions.h"
#include "../math/math2D.h"
#include "../utils/rand.h"

#include <math.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

std::map<transferFuncNames, transfer_function> mapTransferFunctions;

int initNeuralFunctionMap() {
	mapTransferFunctions[transferFuncNames::FN_ONE] = &transfer_fn_one;
	mapTransferFunctions[transferFuncNames::FN_LN] = &transfer_fn_ln;
	mapTransferFunctions[transferFuncNames::FN_SIGMOID] = &transfer_fn_sigmoid;
	mapTransferFunctions[transferFuncNames::FN_THRESHOLD] = &transfer_fn_threshold;
	mapTransferFunctions[transferFuncNames::FN_SIN] = &transfer_fn_sin;
	mapTransferFunctions[transferFuncNames::FN_ABS] = &transfer_fn_abs;
	mapTransferFunctions[transferFuncNames::FN_RAND] = &transfer_fn_rand;
	mapTransferFunctions[transferFuncNames::FN_EXP] = &transfer_fn_exp;
	mapTransferFunctions[transferFuncNames::FN_POW] = &transfer_fn_pow;
	mapTransferFunctions[transferFuncNames::FN_GATE] = &transfer_fn_gate;
	mapTransferFunctions[transferFuncNames::FN_MODULATE] = &transfer_fn_modulate;

	return 0;
}

int dummyToInitMap = initNeuralFunctionMap();

// biasedValue
float transfer_fn_one(float biasedValue, float neuralParam, float input0, float bias) {
	return biasedValue;
}

// biasedValue > 0 ? biasedValue : 0
float transfer_fn_threshold(float biasedValue, float neuralParam, float input0, float bias) {
	return tanh(biasedValue*neuralParam);
}

// sin(biasedValue)
float transfer_fn_sin(float biasedValue, float neuralParam, float input0, float bias) {
	return sin(biasedValue);
}

// ln(biasedValue)
float transfer_fn_ln(float biasedValue, float neuralParam, float input0, float bias) {
	if (biasedValue <= 0)
		return -1.e+10f;
	else
		return logf(biasedValue);
}

// extra param1: float - the base
// param1^biasedValue
float transfer_fn_exp(float biasedValue, float neuralParam, float input0, float bias) {
	if (neuralParam < 0)
		return pow(neuralParam, (int)biasedValue);
	else
		return pow(neuralParam, biasedValue);
}

// extra param1: float - power
// biasedValue^param1
float transfer_fn_pow(float biasedValue, float neuralParam, float input0, float bias) {
	if (biasedValue < 0)
		return pow(biasedValue, (int)neuralParam);
	else
		return pow(biasedValue, neuralParam);
}

// rand(biasedValue)
float transfer_fn_rand(float biasedValue, float neuralParam, float input0, float bias) {
	return randf() * biasedValue;
}

// extra param1: float - controls the steepness
// tanh(biasedValue*param1) -- a good param1 value is 10 -> allows smooth variance between [-0.1, +0.1] and snaps to +-1 outside [-0.2, +0.2]
float transfer_fn_sigmoid(float biasedValue, float neuralParam, float input0, float bias) {
	return tanhf(biasedValue*neuralParam);
}

// param: sigmoid steepness
// input0:  command signal
// bias: command signal threshold
// lets biasedValue pass through if command signal is greater than command signal bias.
float transfer_fn_gate(float biasedValue, float neuralParam, float input0, float bias) {
	float biasedCmdSignal = input0 - bias;
	float unbiasedValue = biasedValue - bias;
	return tanh(biasedCmdSignal*neuralParam) * unbiasedValue;
}

// input0: command signal (input #0)
// multiplies the biasedValue with the command signal
float transfer_fn_modulate(float biasedValue, float neuralParam, float input0, float bias) {
	return biasedValue * input0;
}
