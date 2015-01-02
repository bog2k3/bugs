/*
 * Muscle.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 *
 *
 *  Muscle width determines its max effective torque
 *  Muscle length determines the max speed of the joint
 *  max muscle contraction (%) is constant -> longer muscle's length during a full contraction varies more.
 *  muscle contraction speed (m/s) is constant => a longer muscle (which needs to vary its length more) is slower.
 *  muscle length spectrum correlates 1:1 to the joint's rotation limits.
 *  	- minimum muscle length (max contraction) corresponds to the joint being at its closest limit
 *  	- max muscle length (min contraction) corresponds to the joint being wide open
 *  the greater the ratio between muscle length variation and joint angle variation the greater the insertion point
 *  	(theoretical) of the muscle. This should have an effect on muscle's max effective torque (greater torque, but slower)
 *  muscle should know in which direction it actions the joint motor.
 *  muscle command signal is clamped to [0.0 : 1.0]
 *  joint motor speed is muscle's max angular speed
 *  joint max torque is signal.value * muscle's max torque
 *
 *  formulas for muscle:
 *
 *  l0 = muscle relaxed length												[m]
 *  l = muscle max contracted length < l									[m]
 *  contractionRatio = l/l0 < 1												[1]
 *  dx (length difference) = l0 - l = l0 * (1 - contractionRatio)			[m]
 *  h (distance from muscle's end to joint - length of tendon) 				[m]
 *  r (insertion distance from joint center) = (dx^2 - 2*dx*h)/(2*(dx+h))	[m]
 *  F (max muscle force) = constant * muscle.width							[N]
 *  tau (max torque) = F * h*r/sqrt(h^2+r^2)								[Nm]
 */

#include "Muscle.h"
#include "Joint.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f,0.2f, 0.8f);

const float Muscle::contractionRatio = 0.7f;
const float Muscle::forcePerWidthRatio = 100; // this is the theoretical force of a muscle 1 meter wide.
const float Muscle::maxLinearContractionSpeed = 0.8f; // max meters/second linear contraction speed

Muscle::Muscle(BodyPart* parent, Joint* joint, int motorDirSign)
	: BodyPart(parent, BODY_PART_MUSCLE, std::make_shared<MuscleInitializationData>())
	, muscleInitialData_(std::static_pointer_cast<MuscleInitializationData>(getInitializationData()))
	, joint_(joint)
	, rotationSign_(motorDirSign)
	, maxTorque_(0)
	, maxJointAngularSpeed_(0)
{
	// we need this for debug draw, since muscle doesn't create fixture, nor body
	keepInitializationData_ = true;
	dontCreateBody_ = true;

	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();
	registerAttribute(GENE_ATTRIB_ASPECT_RATIO, initData->aspectRatio);
}

Muscle::~Muscle() {
}

void Muscle::commit() {
	assert(joint_ != nullptr);

	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();

	// here we compute the characteristics of the muscle
	float w0 = sqrtf(initData->size / initData->aspectRatio); // relaxed width
	float l0 = initData->aspectRatio * w0; // relaxed length
	float dx = l0 * (1 - contractionRatio);

	// h is computed as the distance alongside the axis from the parent's center to the joint's attachment angle,
	// from the muscle's attachment point height to	the joint's attachment point height minus l0/2
	// JA (joint attachment vector) - distance from parent to joint
	// MA (muscle attachment vector) - distance from parent to muscle's attachment point
	glm::vec3 parentTransform = parent_->getWorldTransformation();
	glm::vec2 parentXY = vec3xy(parentTransform);
	glm::vec2 JA = vec3xy(joint_->getWorldTransformation()) - parentXY;
	glm::vec2 MA = glm::rotate(getUpstreamAttachmentPoint(), parentTransform.z);
	float h = (JA - glm::dot(glm::normalize(JA), MA)).length() - l0*0.5f;

	// r is the theoretical insertion distance (from joint)
	float r = (dx*dx - 2*dx*h)/(2*(dx+h));

	// and finally:
	maxTorque_ = forcePerWidthRatio * w0 * h*r/sqrt(h*h+r*r);

	// must also compute max speed:
	maxJointAngularSpeed_ = joint_->getTotalRange() / dx * maxLinearContractionSpeed;
}

glm::vec2 Muscle::getChildAttachmentPoint(float relativeAngle) const {
	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();
	// this also takes aspect ratio into account as if the angle is expressed
	// for an aspect ratio of 1:1, and then the resulting point is stretched along the edge.

	float hw = sqrtf(initData->size / initData->aspectRatio) * 0.5f; // half width
	float hl = initData->aspectRatio * hw; // half length
	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, 7*PI/4);
	if (relativeAngle < PI/4) {
		// front edge
		return glm::vec2(hl, sinf(relativeAngle) / sinf(PI/4) * hw);
	} else if (relativeAngle < 3*PI/4 || relativeAngle > 5*PI/4) {
		// left or right edge
		return glm::vec2(cosf(relativeAngle) / cosf(PI/4) * hl, relativeAngle < PI ? hw : -hw);
	} else {
		// back edge
		return glm::vec2(-hl, sinf(relativeAngle) / sinf(PI/4) * hw);
	}
}

void Muscle::draw(RenderContext& ctx) {
	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();
	float w = sqrtf(initData->size / initData->aspectRatio);
	float l = initData->aspectRatio * w;
	glm::vec3 worldTransform = getWorldTransformation();
	ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
			glm::vec2(l, w), worldTransform.z, debug_color);
	ctx.shape->drawLine(
			vec3xy(worldTransform),
			vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
			0,
			debug_color);
}
