

#include "easyunit\testharness.h"
#include "customprinter.h"
using namespace easyunit;

int main()
{
	CustomPrinter *cp = new CustomPrinter();
	TestRegistry::runAndPrint(cp);
}