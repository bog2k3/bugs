/*
 * Nose.cpp
 *
 *  Created on: Jan 1, 2016
 *      Author: bog
 */

#include "Nose.h"
#include "../BodyConst.h"
#include "../../neuralnet/OutputSocket.h"
#include "../../math/math2D.h"
#include "../../utils/UpdateList.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

const glm::vec3 debug_color(1.f, 0.8f, 0.f);

#define DEBUG_DRAW_NOSE

Nose::Nose()
	: BodyPart(BodyPartType::SENSOR_PROXIMITY, std::make_shared<NoseInitializationData>())
{
	for (uint i=0; i<getOutputCount(); i++)
		outputSocket_[i] = new OutputSocket();
}

Nose::~Nose() {
	for (uint i=0; i<getOutputCount(); i++)
		delete outputSocket_[i];
}

void Nose::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_NOSE
#endif
}


glm::vec2 Nose::getChildAttachmentPoint(float relativeAngle) {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}


void Nose::update(float dt) {
	/*
	 * max radius & accuracy are proportional to the size of the nose
	 * detect the nearest object of the right flavour and compute the output signal like this:
	 * s0 = 1/(d^2+1) * max(0, cos(phi))
	 * 		where:  > [d] is the distance from sensor to object
	 * 				> [phi] is the angle of the object relative to the sensor's orientation in space
	 * n = 25% * s0 	this is noise which is proportional to the strength of the signal, in such a way as to always prevent perfect accuracy
	 * s = n + s0;	// the output signal which is 80% useful data and 20% noise (precision is 80%)
	 */
}


float Nose::getOutputVMSCoord(unsigned index) const {
	if (index >= getOutputCount())
		return 0;
	auto initData = std::dynamic_pointer_cast<NoseInitializationData>(getInitializationData());
	if (initData)
		return initData->outputVMSCoord[index].clamp(0, BodyConst::MaxVMSCoordinateValue);
	else
		return 0;
}


void Nose::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(
				&physBody_.b2Body_->GetFixtureList()[0]);
		physBody_.b2Body_->GetWorld()->DestroyJoint(pJoint);
		pJoint = nullptr;
	}

	// create fixture:
	b2PolygonShape shape;
	// nose is oriented towards OX axis
	float sqA3 = sqrt(size_/3);
	float base = 2 * sqA3;
	float height = 3 * sqA3;
	b2Vec2 points[] {
			{0, -base/2},
			{height, 0},
			{0, +base/2}
	};
	shape.Set(points, sizeof(points)/sizeof(points[0]));
	b2FixtureDef fixDef;
	fixDef.density = BodyConst::NoseDensity;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);

	b2WeldJointDef jdef;
	jdef.bodyA = parent_->getBody().b2Body_;
	jdef.bodyB = physBody_.b2Body_;
	glm::vec2 parentAnchor = parent_->getChildAttachmentPoint(attachmentDirectionParent_);
	jdef.localAnchorA = g2b(parentAnchor);
	glm::vec2 childAnchor = getChildAttachmentPoint(angleOffset_);
	jdef.localAnchorB = g2b(childAnchor);
	pJoint = (b2WeldJoint*) physBody_.b2Body_->GetWorld()->CreateJoint(&jdef);
}


void Nose::die() {
	if (getUpdateList())
		getUpdateList()->remove(this);
	physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
}


void Nose::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

