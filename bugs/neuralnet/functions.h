#ifndef __functions_h__
#define __functions_h__

#include <map>
#include <string>
// standard model for all neural transfer functions:
typedef double (*transfer_function)(double value, double arg);

enum transferFuncNames {
	FN_INVAID = 0,
	FN_SIN,
	FN_LN,
	FN_EXP,
	FN_POW,
	FN_RAND,
	FN_SIGM,

	FN_MAXCOUNT			// this is the total number of functions + 1
};

extern std::map<transferFuncNames, transfer_function> mapTransferFunctions;
extern std::map<transferFuncNames, std::string> mapTransferFunctionNames;

// sin(value)
double transfer_fn_sin(double value, double arg);

// ln(value)
double transfer_fn_ln(double value, double arg);

// e^value
double transfer_fn_exp(double value, double arg);

// value^arg
double transfer_fn_pow(double value, double arg);

// rand(value)
double transfer_fn_rand(double value, double arg);

// tanh(value*(arg+1))
double transfer_fn_sigmoid(double value, double arg);

#endif // #ifndef __functions_h__
