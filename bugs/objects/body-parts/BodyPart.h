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
	BodyPart(BodyPart* parent, PART_TYPE type, PhysicsProperties props);
	virtual ~BodyPart();

	PART_TYPE getType() { return type_; }

	void changeParent(BodyPart* newParent);

protected:
	PART_TYPE type_;
	BodyPart* parent_;
	static const unsigned MAX_CHILDREN = 4;
	BodyPart* children_[MAX_CHILDREN];
	int nChildren_;

	void add(BodyPart* part);
	void remove(BodyPart* part);
};



#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
