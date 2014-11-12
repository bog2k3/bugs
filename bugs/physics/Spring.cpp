/*
 * Spring.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: bogdan
 */

#include "Spring.h"

Spring::Spring(AttachPoint a1, AttachPoint a2, float k, float initialLength)
	: a1(a1), a2(a2), k(k), initialLength(initialLength)
{
}

Spring::~Spring() {
}

float Spring::getForce() {
	float dx = (a2.getWorldPos() - a1.getWorldPos()).length() - initialLength;
	return k * dx;
}
