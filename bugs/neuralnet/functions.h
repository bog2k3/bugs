#ifndef __functions_h__
#define __functions_h__

#include <map>
#include <string>
// standard model for all neural transfer functions:
typedef double (*transfer_function)(double value, double constant);

enum transferFuncNames {
	FN_ONE,
	FN_LN,
	FN_SIGM,
	FN_THRESHOLD,
	FN_SIN,
	FN_RAND,
	FN_EXP,
	FN_POW,
	FN_CONSTANT,

	FN_MAXCOUNT			// this is the total number of functions + 1
};

extern std::map<transferFuncNames, transfer_function> mapTransferFunctions;
extern std::map<transferFuncNames, std::string> mapTransferFunctionNames;

// sin(value)
double transfer_fn_sin(double value, double constant);

// value
double transfer_fn_one(double value, double constant);

// ln(value)
double transfer_fn_ln(double value, double constant);

// constant^value
double transfer_fn_exp(double value, double constant);

// value^constant
double transfer_fn_pow(double value, double constant);

// tanh(value*(constant+1))
double transfer_fn_sigmoid(double value, double constant);

// value > constant ? value : constant
double transfer_fn_threshold(double value, double constant);

// always constant
double transfer_fn_constant(double value, double constant);

// rand(value+constant)
double transfer_fn_rand(double value, double constant);

#endif // #ifndef __functions_h__
