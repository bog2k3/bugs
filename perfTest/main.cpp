/*
 * main.cpp
 *
 *  Created on: Sep 2, 2016
 *      Author: bog
 */

#include <thread>
#include "../bugs/perf/marker.h"

void foo() {
	perf::Marker(__FUNCTION__);
}

void callee(int rec) {
	perf::Marker(__FUNCTION__);
	if (rec)
		callee(rec-1);
	foo();
}

void caller(int rec) {
	perf::Marker(__FUNCTION__);
	callee(rec/2);
	if (rec)
		caller(rec-1);
}

int main() {
	return 0;
}


