/*
 * Nose.cpp
 *
 *  Created on: Jan 1, 2016
 *      Author: bog
 */

#include "Nose.h"
#include "../BodyConst.h"
#include "../../World.h"
#include "../../neuralnet/OutputSocket.h"
#include "../../math/math2D.h"
#include "../../utils/UpdateList.h"
#include "../../utils/rand.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Shape2D.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>
#include <algorithm>

const glm::vec3 debug_color(1.f, 0.8f, 0.f);

#define DEBUG_DRAW_NOSE

Nose::Nose()
	: BodyPart(BodyPartType::SENSOR_PROXIMITY, std::make_shared<NoseInitializationData>())
{
	for (uint i=0; i<getOutputCount(); i++)
		outputSocket_[i] = new OutputSocket();

	physBody_.userObjectType_ = ObjectTypes::BPART_NOSE;
	physBody_.userPointer_ = this;

	auto data = std::dynamic_pointer_cast<NoseInitializationData>(getInitializationData());
	for (int i=0; i<NoseDetectableFlavoursCount; i++)
		registerAttribute(GENE_ATTRIB_SENSOR_OUTPUT_COORD, i, data->outputVMSCoord[i]);
}

Nose::~Nose() {
	for (uint i=0; i<getOutputCount(); i++)
		delete outputSocket_[i];
}

void Nose::draw(RenderContext const& ctx) {
	if (committed_ && !noFixtures_) {
		// nothing to draw, physics will draw for us
	} else {
#ifdef DEBUG_DRAW_NOSE
		glm::vec3 worldTransform = getWorldTransformation();
		glm::vec2 zero = vec3xy(worldTransform);
		float sqA3 = sqrt(size_/3);
		float base = 2 * sqA3;
		float height = 3 * sqA3;
		glm::vec2 vert[] {
			glm::vec2(-height/2, base/2),
			glm::vec2(height/2, 0),
			glm::vec2(-height/2, -base/2),
			glm::vec2(-height/2, base/2)
		};
		for (int i=0; i<4; i++)
			vert[i] = zero + glm::rotate(vert[i], worldTransform.z);
		ctx.shape->drawLineStrip(vert, 4, 0, debug_color);
#endif
	}
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
	 * sizeScaling = factor that scales the max amplitude of the signal with the size such that:
	 * 					> for size = infinite, scaling = 1
	 * 					> for size = 0, scaling = 0
	 * 					> for size = 2 * 10^4, scaling = 0.5
	 * 			   = 1 - 1 / (ks * size + 1)
	 * 			   where ks is the size scaling constant
	 * s1 = sizeScaling * s0; > scaled signal (bigger nose, more powerful signal)
	 * noise:
	 * ntk = 4.5 * 10^4		> noise threshold constant
	 * nt = 1 / (1 + ntk * size)	> noise threshold (0.1 at size=2*10^-4) -> bigger nose, lower noise threshold, improved sensing
	 * ia = +/-15% * s1		> inaccuracy proportional to the strength of the signal, in such a way as to always prevent perfect accuracy
	 * n0 = ntk * rand		> this is noise
	 * s = n0 + s1 + ia;	> the output signal with noise threshold, size-modulated amplitude and proportional inaccuracy
	 */

	// compute maxDist as the distance at which the scaled signal becomes as low as the noise threshold
	float ks = BodyConst::SensorSizeScalingConstant;
	float kt = BodyConst::SensorNoiseThreshConstant;
	float maxDist = sqrtf((ks*kt*size_*size_ - 1) / (ks*size_ + 1));

	for (int i=0; i<NoseDetectableFlavoursCount; i++) {
		glm::vec3 posRot = getWorldTransformation();
		glm::vec2 pos = vec3xy(posRot);
		auto ents = World::getInstance()->getEntitiesInBox(NoseDetectableFlavours[i], Entity::FunctionalityFlags::ALL, pos, maxDist * 1.1f, true);

		// sort by closest first
		std::sort(ents.begin(), ents.end(), [&pos](Entity* e1, Entity* e2) {
			float d1sq = vec2lenSq(vec3xy(e1->getWorldTransform()) - pos);
			float d2sq = vec2lenSq(vec3xy(e2->getWorldTransform()) - pos);
			return d1sq < d2sq;
		});

		glm::vec3 otherPosRot = ents[0]->getWorldTransform();

		float minDistSq = vec2lenSq(vec3xy(otherPosRot) - pos);
		float cosphi = cosf(angleDiff(posRot.z, pointDirection(glm::vec2(otherPosRot))));	// direction factor
		float s0 = 1.f / (minDistSq + 1) * max(0.f, cosphi);	// raw signal
		float sizeScaling = 1 - 1.f / (1 + size_ * ks);
		float s1 = s0 * sizeScaling;		// modulated signal
		float nt = 1.f / (1 + kt * size_);	// noise threshold
		float ia = srandf() * 0.15f * s1;	// +/-15% inaccuracy
		float noise = randf() * nt;

		float signal = noise + ia + s1;
		outputSocket_[i]->push_value(signal);
	}
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
	if (committed_ && !noFixtures_) {
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

	if (base*height < b2_linearSlop) {
		noFixtures_ = true;
		return;
	}

	b2Vec2 points[] {
			{-height/2, -base/2},
			{height/2, 0},
			{-height/2, +base/2}
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
}


void Nose::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

