/*
 * Spring.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: bogdan
 */

#include "Spring.h"
#include "../math/math2D.h"
#include <glm/glm.hpp>

Spring::Spring(AttachPoint a1, AttachPoint a2, float k, float initialLength)
	: a1(a1), a2(a2), k(k), initialLength(initialLength)
	, force(0)
	, f0(0)
	, v0(0), a0(0)
	, l0(0)
{
}

Spring::~Spring() {
}

void Spring::update(float dt) {
	glm::vec2 distance = a2.getWorldPos() - a1.getWorldPos();
	float len = glm::length(distance);

	/*if (len <= initialLength) {
		force = glm::vec2(0);
	} else {
		force = distance / len * (F0*dt + k*v0*sqr(dt) + k*a0*0.5f*sqr(dt)*dt);
	}*/

	if (len == 0)
		len = EPS;

	glm::vec2 instantForce = k * distance * (1 - initialLength / len);

	force = 0.5f * (f0 + instantForce) - distance * (1.f/len * k*a0/12.f * sqr(dt));

	f0 = instantForce;
	float v = (len-initialLength - l0) / dt;
	a0 = (v - v0) / dt;
	v0 = v;
	l0 = len-initialLength;

	/*if (len <= initialLength) {
		force = prevForce * 0.5f;
	}
	else {
		force = (k * distance * (1 - initialLength / len) + prevForce) * 0.5f;
	}
	prevForce = force;*/
}

float Spring::getDelta() {
	glm::vec2 distance = a2.getWorldPos() - a1.getWorldPos();
	float len = glm::length(distance);
	return len - initialLength;
}
