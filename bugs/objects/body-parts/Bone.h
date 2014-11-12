/*
 * Bone.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BONE_H_
#define OBJECTS_BODY_PARTS_BONE_H_

#include "../../physics/RigidBody.h"

class Bone: public RigidBody {
public:
	Bone();
	virtual ~Bone();
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
