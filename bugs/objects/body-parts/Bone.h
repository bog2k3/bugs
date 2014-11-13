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

class Bone: public WorldObject {
public:
	Bone(glm::vec2 position, float rotation, float density, glm::vec2 size, glm::vec2 initialVelocity, float initialAngularVelocity);
	virtual ~Bone();

protected:
	float density;
	glm::vec2 size;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
