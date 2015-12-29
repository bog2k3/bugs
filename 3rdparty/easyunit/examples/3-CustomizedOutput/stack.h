#ifndef STACK_H
#define STACK_H


class Stack
{
	public:
		Stack();
		
		void push(int val);
		int pull();
		int top();
		
		int size() const;
		
	private:
		int value;
		int size_;
};

#endif