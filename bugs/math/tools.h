#ifndef __tools_h__
#define __tools_h__

float randf(); // generates a random number between 0.0 and 1.0 inclusive
double randd(); // generates a random number between 0.0 and 1.0 inclusive
double srandd(); // generates a signed number between -1.0 and +1.0 inclusive
int randi(int max); // generates a random number between 0 and max inclusive, with equal chances for all numbers, including max
					// negative value for max is allowed, in this case the value will be between [max..0]
int randi(int min, int max); // generates a random number between min inclusive and max inclusive, with equal chances for all numbers
					// negative values are allowed, but min must always be smaller than max
unsigned long new_RID(); // generates a new Random IDentifier

#endif // __tools_h__
