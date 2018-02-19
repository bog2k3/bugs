/*
 * JointWeld.h
 *
 *  Created on: Dec 31, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_JOINTWELD_H_
#define BODY_PARTS_JOINTWELD_H_

#include "BodyPart.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class b2WeldJoint;

class JointWeld: public BodyPart {
public:
	JointWeld(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor);	// use the parent cell (that has divided) for the joint
	virtual ~JointWeld() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	glm::vec3 getWorldTransformation() const override;

protected:
	BodyPart* leftAnchor_;
	BodyPart* rightAnchor_;
	b2WeldJoint* physJoint_;

	void updateFixtures() override;
//	void die() override;

	void onPhysJointDestroyed(b2Joint* joint);
	void destroyPhysJoint();

private:
	unsigned jointListenerHandle_ = 0;
};

#endif /* BODY_PARTS_JOINTWELD_H_ */
