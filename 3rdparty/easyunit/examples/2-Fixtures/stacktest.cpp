#include "easyunit\test.h"
#include "stack.h"

using namespace easyunit;


DECLARE(Stack)
	Stack s;
	Stack s2;
	Stack *s3;
	Stack s4;
	Stack s5;
END_DECLARE


SETUP(Stack)
{
	s3 = new Stack();
	s2.push(1);
	s3->push(1);
	s3->push(2);
}

TEARDOWN(Stack)
{
	delete s3;
}

TESTF(Stack,Constructor)
{
	ASSERT_TRUE(s.size() == 0);
	ASSERT_TRUE(s.pull() == 0);
}

TESTF(Stack, PushTop)
{
	s.push(1);
	ASSERT_TRUE(s.top() == 1);
	s.push(2);
	s.push(3);
	ASSERT_TRUE(s.top() == 3);
}

TESTF(Stack, PushPull)
{
	s.push(1);
	ASSERT_TRUE(s.pull() == 1);
	s.push(2);
	s.push(3);
	s.pull();
	// This test will fail
	ASSERT_TRUE(s.pull() == 2);
}

TESTF(Stack, PushSize)
{
	ASSERT_TRUE(s2.size() == 1);
	ASSERT_TRUE(s3->size() == 2);
}

TESTF(Stack, PushPullSize)
{
	s.push(1);
	s.push(2);
	s.pull();
	
	// This is essentially the same tests
	
	ASSERT_TRUE(s.size() == 1);
		
  ASSERT_EQUALS(s.size(),1);
	
	// Display a message if this is a failure
	ASSERT_EQUALS_M(s.size(),1,"size != 1 but should == 1");
	
	// Display the value of each parameter instead of the litteral
	// SimpleString StringFrom(type) method must be declared for 
	// this type.
	ASSERT_EQUALS_V(s.size(),1);
	
	// See also ASSERT_EQUALS_DELTA
	
	s.pull();
	s.pull();
	if (s.size() < 0) {
		// This will add a failure
		FAIL_M("s.size() < 0");
	}
}