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
	ASSERT_TRUE(s.size() == 1);
	s.pull();
	s.pull();
	if (s.size() < 0) {
		// This will add a failure
		FAIL_M("s.size() < 0");
	}
}