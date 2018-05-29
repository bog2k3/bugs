/*
 * JointWeld.h
 *
 *  Created on: Dec 31, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_JOINTWELD_H_
#define BODY_PARTS_JOINTWELD_H_

#include "Joint.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class b2WeldJoint;

class JointWeld: public Joint {
public:
	JointWeld(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor);	// use the parent cell (that has divided) for the joint
	virtual ~JointWeld() override;

	void draw(Viewport* vp) override;
	glm::vec3 getWorldTransformation() const override;

protected:
//	void die() override;
	float breakForce() const override;
	float breakTorque() const override;

	b2JointDef* createJointDef(b2Vec2 localAnchorA, b2Vec2 localAnchorB, float refAngle) override;

private:
	unsigned jointListenerHandle_ = 0;
};

#endif /* BODY_PARTS_JOINTWELD_H_ */
