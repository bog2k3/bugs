/*
 * Joint.h
 *
 *  Created on: Feb 20, 2018
 *      Author: bog
 */

#ifndef BODY_PARTS_JOINT_H_
#define BODY_PARTS_JOINT_H_

#include "BodyPart.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class b2Joint;
class b2JointDef;

class Joint: public BodyPart {
public:
	Joint(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor, BodyPartType type);	// use the parent cell (that has divided) for the joint
	virtual ~Joint() override;

	BodyPart* getLeftAnchor() const { return leftAnchor_; }
	BodyPart* getRightAnchor() const { return rightAnchor_; }

protected:
	BodyPart* leftAnchor_;
	BodyPart* rightAnchor_;
	b2Joint* physJoint_;

	void updateFixtures() override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	virtual b2JointDef* createJointDef(b2Vec2 localAnchorA, b2Vec2 localAnchorB, float refAngle) = 0;
	void destroyPhysJoint();

private:
	unsigned jointListenerHandle_ = 0;

	void onPhysJointDestroyed(b2Joint* joint);
};

#endif /* BODY_PARTS_Joint_H_ */
