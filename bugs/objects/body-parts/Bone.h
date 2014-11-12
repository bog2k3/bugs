/*
 * Bone.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BONE_H_
#define OBJECTS_BODY_PARTS_BONE_H_

#include "../../physics/RigidBody.h"
#include"../IWorldObject.h"

class Bone: public RigidBody, public IWorldObject {
public:
	Bone(glm::vec2 position, float rotation, float density, glm::vec2 size, glm::vec2 initialVelocity, float initialAngularVelocity);
	virtual ~Bone();

	virtual AlignedBox getAlignedBoundingBox() const;
	virtual ArbitraryBox getOrientedBoundingBox() const;
	virtual float getMomentOfInertia() const;

	virtual void draw(ObjectRenderContext*);

protected:
	float density;
	glm::vec2 size;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
