
#include "CustomizedRunner.h"

CustomizedRunner::CustomizedRunner(char* testCaseName)
: testCaseName_(testCaseName)
{
	
}


void CustomizedRunner::run(TestCase *testCase, int size)
{
	for (int i=0; i<size; i++) {
		if (StringFrom(testCase->getName()) == StringFrom(testCaseName_)) {
			testCase->run();
		}
		testCase = testCase->getNext();
	}
}

