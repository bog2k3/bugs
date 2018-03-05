/*
 * Joint.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_BODY_PARTS_JOINT_H_
#define OBJECTS_BODY_PARTS_JOINT_H_

#include "Joint.h"

class b2RevoluteJoint;

#define DEBUG_DRAW_JOINT

class JointPivot : public Joint {
public:
	JointPivot(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor);	// use the parent cell (that has divided) for the joint
	virtual ~JointPivot() override;

	void draw(RenderContext const& ctx) override;
	glm::vec3 getWorldTransformation() const override;

	void update(float dt);

	inline float getTotalRange() const { return phiMax_ - phiMin_; } // returns the total angular range (in radians) of the joint.
	inline float getLowerLimit() const { return phiMin_; }
	inline float getUpperLimit() const { return phiMax_; }
	float getJointAngle() const;

	void addTorque(float t, float maxSpeed);

	static float getDensity(BodyCell const& cell);

protected:
	float phiMin_;
	float phiMax_;
	float resetTorque_;			// torque that resets the joint into repause position
	std::vector<std::pair<float, float>> vecTorques;	// holds torque|maxSpeed pairs

	void getNormalizedLimits(float &low, float &high);
	void die() override;
	//void onDetachedFromParent() override;

	b2JointDef* createJointDef(b2Vec2 localAnchorA, b2Vec2 localAnchorB, float refAngle) override;
	b2RevoluteJoint* b2PJoint() const;
};

#endif /* OBJECTS_BODY_PARTS_JOINT_H_ */
