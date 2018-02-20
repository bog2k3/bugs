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

//	glm::vec3 getWorldTransformation() const override;

protected:
	BodyPart* leftAnchor_;
	BodyPart* rightAnchor_;
	b2Joint* physJoint_;

	void updateFixtures() override;
//	void die() override;

	virtual b2JointDef* createJointDef(b2Body* left, b2Body* right) = 0;

private:
	unsigned jointListenerHandle_ = 0;

	void onPhysJointDestroyed(b2Joint* joint);
	void destroyPhysJoint();
};

#endif /* BODY_PARTS_Joint_H_ */
