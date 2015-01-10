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
#include "../../updatable.h"

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
	Joint(BodyPart* parent);
	~Joint() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	glm::vec3 getWorldTransformation(bool force_recompute=false) const override;

	void update(float dt);

	float getTotalRange(); // returns the total angular range (in radians) of the joint.
	float getLowerLimit();
	float getUpperLimit();
	float getJointAngle();

	void addTorque(float t, float maxSpeed);

protected:
	std::weak_ptr<JointInitializationData> jointInitialData_;
	b2RevoluteJoint* physJoint_;
	float repauseAngle_;		// this is the angle toward which the joint tends to settle when muscles are idle
	float resetTorque_;			// torque that resets the joint into repause position
	std::vector<std::pair<float, float>> vecTorques;	// holds torque|maxSpeed pairs

	void getNormalizedLimits(float &low, float &high);
};

template<> void update(Joint*& j, float dt);

#endif /* OBJECTS_BODY_PARTS_JOINT_H_ */
