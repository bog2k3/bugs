#ifndef __tools_h__
#define __tools_h__

#include <cstdlib>
#include <time.h>
#include <climits>

extern unsigned int rand_seed;

inline void check_init_rand_seed() {
	if (rand_seed == 0) {
		rand_seed = (unsigned int)time(NULL);
		srand(rand_seed);
	}
}

// generates a random number between 0.0 and 1.0 inclusive
inline float randf() {
	check_init_rand_seed();
	return (float)rand() / RAND_MAX;
}

// generates a signed random number between -1.0 and 1.0 inclusive
inline float srandf() {
	return 2.0f*randf() - 1.0f;
}

// generates a random number between 0.0 and 1.0 inclusive
inline double randd() {
	check_init_rand_seed();
	return (double)rand() / RAND_MAX;
}

// generates a signed number between -1.0 and +1.0 inclusive
inline double srandd() {
	return 2.0*randd() - 1.0;
}

// generates a random number between 0 and max inclusive, with equal chances for all numbers, including max
// negative value for max is allowed, in this case the value will be between [max..0]
inline int randi(int max) {
	int sign = 1;
	if (max < 0) {
		max = -max;
		sign = -1;
	}
	return sign * (int)(randd() * (max + 0.9999));
}

// generates a random number between min inclusive and max inclusive, with equal chances for all numbers
// negative values are allowed, but min must always be smaller than max
inline int randi(int min, int max) {
	if (min > max)
		return 0;
	return min + randi(max-min);
}

// generates a new Random IDentifier
inline unsigned long new_RID() {
	return (unsigned long)(ULONG_MAX * randd());
}


#endif // __tools_h__
