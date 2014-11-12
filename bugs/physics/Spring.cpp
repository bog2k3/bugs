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

glm::vec2 Spring::getForce() {
	glm::vec2 distance = a2.getWorldPos() - a1.getWorldPos();
	float len = distance.length();
	if (len <= initialLength)
		return glm::vec2(0);
	return k * distance * (1 - initialLength / len);
}
