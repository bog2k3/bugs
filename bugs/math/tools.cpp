#include "tools.h"
#include <stdlib.h>
#include <time.h>
#include <limits.h>

static unsigned int rand_seed = 0;

inline void check_init_rand_seed() {
	if (rand_seed == 0) {
		rand_seed = (unsigned int)time(NULL);
		srand(rand_seed);
	}
}

double randd() {
	check_init_rand_seed();
	return (double)rand() / RAND_MAX;
}

double srandd() {
	return 2.0*randd() - 1.0;
}

int randi(int max) {
	int sign = 1;
	if (max < 0) {
		max = -max;
		sign = -1;
	}
	return sign * (int)(randd() * (max + 0.9999));
}

int randi(int min, int max) {
	if (min > max)
		return 0;
	return min + randi(max-min);
}

unsigned long new_RID() {
	return (unsigned long)(ULONG_MAX * randd());
}
