/*
 * Bone.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BONE_H_
#define OBJECTS_BODY_PARTS_BONE_H_

#include "BodyPart.h"
#include <glm/vec2.hpp>

class Bone: public BodyPart {
public:
	Bone(BodyPart* parent, float density, glm::vec2 size, PhysicsProperties props);
	virtual ~Bone();

protected:
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
