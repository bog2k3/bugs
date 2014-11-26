/*
 * Bone.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BONE_H_
#define OBJECTS_BODY_PARTS_BONE_H_

#include "../WorldObject.h"
#include <glm/vec2.hpp>

class b2World;

class Bone: public WorldObject {
public:
	Bone(b2World* world, glm::vec2 position, float rotation, float density, glm::vec2 size, glm::vec2 initialVelocity, float initialAngularVelocity);
	virtual ~Bone();

protected:
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
