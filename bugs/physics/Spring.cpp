/*
 * Spring.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: bogdan
 */

#include "Spring.h"
#include "../math/math2D.h"
#include "../log.h"
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

	glm::vec2 instantForce;
	if (len == 0) {
		instantForce = glm::vec2(0);
	} else {
		instantForce = k * distance * (1 - initialLength / len);
	}

	float v = (len-initialLength - l0) / dt;
	a0 = (v - v0) / dt;
	v0 = v;
	l0 = len-initialLength;

	force = 0.5f * (f0 + instantForce) * (1.f - k*a0/12.f * sqr(dt));
	LOG(""<<v0<<"\t"<<a0<<"\t"<<l0<<"\t"<<glm::length(force));

	f0 = instantForce;

	/*if (len <= initialLength) {
		force = prevForce * 0.5f;
	}
	else {
		force = (k * distance * (1 - initialLength / len) + prevForce) * 0.5f;
	}
	prevForce = force;*/
}

float Spring::getDelta() {
	return l0;
}
