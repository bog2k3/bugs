/*
 * MouseObject.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "MouseObject.h"
#include "../math/math.h"
#include <glm/vec2.hpp>
using namespace glm;

MouseObject::MouseObject()
	: RigidBody(1.f, vec2(0), 0, vec2(0), 0)
{
	isFixed = true;
}

MouseObject::~MouseObject() {
	// TODO Auto-generated destructor stub
}

AlignedBox MouseObject::getAlignedBoundingBox() const {
	return AlignedBox(getPosition(), getPosition());
}
ArbitraryBox MouseObject::getOrientedBoundingBox() const {
	return ArbitraryBox::empty(getPosition());
}
AlignedBox MouseObject::getLocalBoundingBox() const {
	return AlignedBox();
}
