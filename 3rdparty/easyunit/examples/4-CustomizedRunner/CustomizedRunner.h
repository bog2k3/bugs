#ifndef CUSTOM_RUNNER_H
#define CUSTOM_RUNNER_H

#include "easyunit/testrunner.h"
#include "easyunit/testcase.h"

using namespace easyunit;

class CustomizedRunner :public TestRunner {

	public:
		CustomizedRunner(char* testCaseName);
		void run(TestCase *testCase, int size);

	private:
		char* testCaseName_;

};


#endif