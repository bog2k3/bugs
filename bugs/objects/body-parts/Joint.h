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
	Joint(BodyPart* parent);
	~Joint() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	glm::vec3 getWorldTransformation() const override;

	float getTotalRange(); // returns the total angular range (in radians) of the joint.

protected:
	float size_;
	float phiMin_;
	float phiMax_;
	b2RevoluteJoint* physJoint_;

	void fixAngles();
};

#endif /* OBJECTS_BODY_PARTS_JOINT_H_ */
