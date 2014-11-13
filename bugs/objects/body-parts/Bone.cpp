/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "../../renderOpenGL/Rectangle.h"
#include "BonePhysicsComponent.h"

Bone::Bone(glm::vec2 position, float rotation, float density, glm::vec2 size, glm::vec2 initialVelocity, float initialAngularVelocity)
	: WorldObject(new BonePhysicsComponent(size.x*size.y*density, position, rotation, size, initialVelocity, initialAngularVelocity))
	, density(density)
	, size(size)
{
}

Bone::~Bone() {
}
