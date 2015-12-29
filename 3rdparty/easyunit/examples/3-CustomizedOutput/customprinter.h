#ifndef CUSTOMPRINTER_H
#define CUSTOMPRINTER_H

#include "easyunit\testprinter.h"

using namespace easyunit;

class CustomPrinter : public TestPrinter
{
	virtual void print(const TestResult *testResult);
};

#endif