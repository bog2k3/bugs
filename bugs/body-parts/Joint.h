/*
 * Joint.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_BODY_PARTS_JOINT_H_
#define OBJECTS_BODY_PARTS_JOINT_H_

#include "BodyPart.h"
#include <glm/fwd.hpp>
#include <memory>

#define DEBUG_DRAW_JOINT

struct JointInitializationData : public BodyPartInitializationData {
	virtual ~JointInitializationData() noexcept = default;
	JointInitializationData();

	CummulativeValue phiMin;
	CummulativeValue phiMax;
	CummulativeValue resetTorque;
};

class b2RevoluteJoint;

class Joint : public BodyPart {
public:
	Joint();
	~Joint() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	glm::vec3 getWorldTransformation() override;

	void update(float dt);

	inline float getTotalRange() { return phiMax_ - phiMin_; } // returns the total angular range (in radians) of the joint.
	inline float getLowerLimit() { return phiMin_; }
	inline float getUpperLimit() { return phiMax_; }
	float getJointAngle();

	void addTorque(float t, float maxSpeed);

protected:
	b2RevoluteJoint* physJoint_;
	float phiMin_;
	float phiMax_;
	float resetTorque_;			// torque that resets the joint into repause position
	std::vector<std::pair<float, float>> vecTorques;	// holds torque|maxSpeed pairs

	friend class World;
	void getNormalizedLimits(float &low, float &high);
	void commit() override;
	void cacheInitializationData() override;
	void die() override;
};

#endif /* OBJECTS_BODY_PARTS_JOINT_H_ */
