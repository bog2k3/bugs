/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "../../renderOpenGL/Rectangle.h"

Bone::Bone(glm::vec2 position, float rotation, float density, glm::vec2 size, glm::vec2 initialVelocity, float initialAngularVelocity)
	: RigidBody(size.x*size.y*density, position, rotation, initialVelocity, initialAngularVelocity)
	, density(density)
	, size(size)
{
}

Bone::~Bone() {
}

void Bone::draw(ObjectRenderContext* ctx) {
	ctx->rectangle->draw(getPosition().x, getPosition().y, 0, size.x, size.y, getRotation(), 0, 1, 0);
}
