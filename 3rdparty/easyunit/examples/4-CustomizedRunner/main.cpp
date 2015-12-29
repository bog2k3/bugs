

#include "easyunit\testharness.h"
#include "CustomizedRunner.h"
using namespace easyunit;

int main()
{
	CustomizedRunner c("QueueTestCase");
	TestRegistry::runAndPrint(&c);
}