#include "customprinter.h"
#include <iostream>

using namespace std;

void CustomPrinter::print(const TestResult *testResult) {
	cout << "Custom Results" << endl;
	cout << "Overall result: ";
	if (testResult->getTotalErrors() > 0 || testResult->getTotalFailures() > 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "SUCCESS" << endl;
	}
}