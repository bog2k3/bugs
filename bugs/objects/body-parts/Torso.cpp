/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"

Torso::Torso(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_TORSO, props)
	, size_(1.e-3f) // 10 sq cm
	, density_(1.f)
	, committed_(false)
{
}

Torso::~Torso() {
	// delete fixture
}

void Torso::commit() {
	assert(!committed_);
	committed_ = true;

	// create fixture....
}

void Torso::setSize(float val) {
	assert(!committed_);
	size_ = val;
}
void Torso::setDensity(float val) {
	assert(!committed_);
	density_ = val;
}
