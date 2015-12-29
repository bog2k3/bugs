#include "easyunit\test.h"

#include "queue.h"

using namespace easyunit;

DECLARE(QueueTestCase)
	Queue q1;
	Queue *q2;
END_DECLARE

SETUP(QueueTestCase)
{
	q2 = new Queue();
}

TEARDOWN(QueueTestCase)
{
	delete q2;
}

TESTF(QueueTestCase,Initialization)
{
	ASSERT_TRUE(q2->size() == 0);
}

TESTF(QueueTestCase,Size)
{
	ASSERT_TRUE(q1.size() == 0);
}