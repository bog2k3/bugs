#include "easyunit\test.h"
#include "stack.h"

using namespace easyunit;

TEST(Stack,Constructor)
{
	Stack s;
	ASSERT_TRUE(s.size() == 0);
	ASSERT_TRUE(s.pull() == 0);
}

TEST(Stack, PushTop)
{
	Stack s;
	s.push(1);
	ASSERT_TRUE(s.top() == 1);
	s.push(2);
	s.push(3);
	ASSERT_TRUE(s.top() == 3);
}

TEST(Stack, PushPull)
{
	Stack s;
	s.push(1);
	ASSERT_TRUE(s.pull() == 1);
	s.push(2);
	s.push(3);
	s.pull();
	// This test will fail
	ASSERT_TRUE(s.pull() == 2);
}

TEST(Stack, PushSize)
{
	Stack s;
	s.push(1);
	ASSERT_TRUE(s.size() == 1);
	s.push(2);
	ASSERT_TRUE(s.size() == 2);
}

TEST(Stack, PushPullSize)
{
  Stack s;
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
	// This would output: Expected: 1 but Actual: 2
	ASSERT_EQUALS_V(s.size(),1);
	
	// See also ASSERT_EQUALS_DELTA
	
	s.pull();
	s.pull();
	if (s.size() < 0) {
		// This will add a failure
		FAIL_M("s.size() < 0");
	}
}