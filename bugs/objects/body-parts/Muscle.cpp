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
 *  gamma (contraction ratio) = l/l0 < 1									[1]
 *  dx (length difference) = l0 - l = l0 * (1 - gamma) 						[m]
 *  h (distance from muscle's end to joint - length of tendon) 				[m]
 *  r (insertion distance from joint center) = (dx^2 - 2*dx*h)/(2*(dx+h))	[m]
 *  F (max muscle force) = constant * muscle.width							[N]
 *  tau (max torque) = F * h*r/sqrt(h^2+r^2)								[Nm]
 */

#include "Muscle.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f,0.2f, 0.8f);

Muscle::Muscle(BodyPart* parent, PhysicsProperties props)
: BodyPart(parent, BODY_PART_MUSCLE, props)
, size_(0.5e-4f)
, aspectRatio_(0.7f)
, insertionOffset_(0.5f)
{
	// we need this for debug draw, since muscle doesn't create fixture, nor body
	keepInitializationData_ = true;
	dontCommit_ = true;
}

Muscle::~Muscle() {
}

glm::vec2 Muscle::getChildAttachmentPoint(float relativeAngle)
{
	// this also takes aspect ratio into account as if the angle is expressed
	// for an aspect ratio of 1:1, and then the resulting point is stretched along the edge.

	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, 7*PI/4);
	float hw = sqrtf(size_/aspectRatio_) * 0.5f; // half width
	float hl = aspectRatio_ * hw; // half length
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
	initialData_->position = getFinalPrecommitPosition();
	glm::vec3 worldTransform = getWorldTransformation();
	float w = sqrtf(size_/aspectRatio_);
	float l = aspectRatio_ * w;
	ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
			glm::vec2(l, w), worldTransform.z, debug_color);
	ctx.shape->drawLine(
			vec3xy(worldTransform),
			vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
			0,
			debug_color);
}
