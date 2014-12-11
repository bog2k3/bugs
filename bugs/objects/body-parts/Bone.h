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
	Bone(BodyPart* parent, PhysicsProperties props);
	virtual ~Bone() override;
	virtual void commit() override;

	float getDensity() { return density_; }
	glm::vec2 getSize() { return size_; }

	void setDensity(float value);
	void setSize(glm::vec2 value);

protected:
	float density_;
	glm::vec2 size_;
	bool committed_;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
