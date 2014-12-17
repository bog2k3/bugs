/*
 * Joint.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_BODY_PARTS_JOINT_H_
#define OBJECTS_BODY_PARTS_JOINT_H_

#include <glm/fwd.hpp>
#include "BodyPart.h"

class b2RevoluteJoint;

class Joint : public BodyPart {
public:
	Joint(BodyPart* parent, PhysicsProperties props);
	virtual ~Joint() override;

	void commit() override;

protected:
	float size_;
	float phiMin_;
	float phiMax_;
	b2RevoluteJoint* physJoint_;
};

#endif /* OBJECTS_BODY_PARTS_JOINT_H_ */
