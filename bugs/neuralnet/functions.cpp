#include "functions.h"
#include <math.h>
#include "../math/math2D.h"
#include "../math/tools.h"

std::map<transferFuncNames, transfer_function> mapTransferFunctions;
std::map<transferFuncNames, std::string> mapTransferFunctionNames;

int initNeuralFunctionMap() {
	mapTransferFunctions[FN_SIN] = &transfer_fn_sin;
	mapTransferFunctions[FN_LN] = &transfer_fn_ln;
	mapTransferFunctions[FN_EXP] = &transfer_fn_exp;
	mapTransferFunctions[FN_POW] = &transfer_fn_pow;
	mapTransferFunctions[FN_RAND] = &transfer_fn_rand;
	mapTransferFunctions[FN_SIGM] = &transfer_fn_sigmoid;

	mapTransferFunctionNames[FN_SIN] = "SIN";
	mapTransferFunctionNames[FN_LN] = "LOG";
	mapTransferFunctionNames[FN_EXP] = "EXP";
	mapTransferFunctionNames[FN_POW] = "POW";
	mapTransferFunctionNames[FN_RAND] = "RAND";
	mapTransferFunctionNames[FN_SIGM] = "SIGM";

	return 0;
}

int dummyToInitMap = initNeuralFunctionMap();

// sin(value)
double transfer_fn_sin(double value, double arg) {
	return sin(value);
}

// ln(value)
double transfer_fn_ln(double value, double arg) {
	return log(value);
}

// e^value
double transfer_fn_exp(double value, double arg) {
	return pow(E, value);
}

// value^arg
double transfer_fn_pow(double value, double arg) {
	return pow(value, arg);
}

// rand(value)
double transfer_fn_rand(double value, double arg) {
	return randd() * value;
}

// tanh(value*(arg+1))
double transfer_fn_sigmoid(double value, double arg) {
	return tanh(value*(arg+1));
}
