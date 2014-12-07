/*
 * BodyPart.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BODYPART_H_
#define OBJECTS_BODY_PARTS_BODYPART_H_

#include "../WorldObject.h"

enum PART_TYPE {
	BODY_PART_INVALID,

	BODY_PART_TORSO,
	BODY_PART_BONE,
	BODY_PART_MUSCLE,
	BODY_PART_JOINT,
	BODY_PART_GRIPPER,
	BODY_PART_ZYGOTE_SHELL,
	BODY_PART_SENSOR,
};

class BodyPart : public WorldObject {
public:
	BodyPart(World* world, glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, PART_TYPE type);
	BodyPart(BodyPart* parent, PART_TYPE type);
	virtual ~BodyPart();

	void setParent(WorldObject* parent);
	PART_TYPE getType() { return type; }

protected:
	PART_TYPE type;
	BodyPart* parent;
};



#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
