/*
 * RigidBody.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "RigidBody.h"

RigidBody::RigidBody(float mass, glm::vec2 position, float rotation, glm::vec2 initialVelocity, float initialAngularVelocity)
	: isFixed(false), mass(mass), position(position), velocity(initialVelocity), rotation(rotation), angularVelocity(initialAngularVelocity)
	, resultantForce(0), resultantTorque(0)
	, matLocalToWorld(1)
{
	updateMatrix();
}

glm::vec2 RigidBody::localToWorld(glm::vec2 local) const {
	return matLocalToWorld * glm::vec3(local, 1);
}

void RigidBody::updateMatrix() {
	float cosT = cos(rotation);
	float sinT = sin(rotation);
	matLocalToWorld[0][0] = cosT;
	matLocalToWorld[0][1] = cosT;
	matLocalToWorld[1][0] = -sinT;
	matLocalToWorld[1][1] = sinT;
	matLocalToWorld[2][0] = position.x;
	matLocalToWorld[2][1] = position.y;
}

void RigidBody::teleport(glm::vec2 where) {
	position = where;
	updateMatrix();
}
