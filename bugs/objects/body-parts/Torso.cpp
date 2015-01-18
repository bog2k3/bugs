/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"
#include "BodyConst.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../UpdateList.h"
#include "../../log.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f, 1.f, 0.f);

Torso::Torso(BodyPart* parent)
	: BodyPart(parent, BODY_PART_TORSO, std::make_shared<BodyPartInitializationData>())
	, size_(0.f)
	, fatMass_(0.1)
	, lastCommittedTotalSizeInv_(0)
	, frameUsedEnergy_(0)
{
	getUpdateList()->add(this);
}

Torso::~Torso() {
#warning("delete fixture")
	getUpdateList()->remove(this);
}

void Torso::commit() {
	if (committed_) {
		body_->DestroyFixture(&body_->GetFixtureList()[0]);
	}

	size_ = getInitializationData()->size;
	float fatSize = fatMass_ * BodyConst::FatDensityInv;
	lastCommittedTotalSizeInv_ = 1.f / (size_ + fatSize);

	float density = (size_ * BodyConst::TorsoDensity + fatMass_) / (size_ + fatMass_ * BodyConst::FatDensityInv);

	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf((size_+fatSize)/PI);

	b2FixtureDef fixDef;
	fixDef.density = density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}

void Torso::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing, physics draws
#ifdef DEBUG_DRAW_TORSO
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
#endif
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(getInitializationData()->size/PI), 0, 12, debug_color);
		ctx.shape->drawCircle(pos, sqrtf((getInitializationData()->size+fatMass_*BodyConst::FatDensityInv)/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size/PI), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Torso::getChildAttachmentPoint(float relativeAngle) const
{
	float size = getInitializationData()->size + fatMass_*BodyConst::FatDensityInv;
	return glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), relativeAngle);
}

void Torso::setInitialFatMass(float fat) {
	assert(!committed_);
	fatMass_ = fat;
}

void Torso::consumeEnergy(float amount) {
	frameUsedEnergy_ += amount;

#warning "optimization: have a ready-to-use energy buffer instead of using fat all the time. use fat to replenish buffer. food replenishes buffer too."
}

template<> void update(Torso* &t, float dt) {
	t->update(dt);
}

void Torso::update(float dt) {
	fatMass_ -= frameUsedEnergy_ * BodyConst::FatEnergyDensityInv;
	float crtSize = fatMass_ * BodyConst::FatDensityInv + size_;
	if (crtSize * lastCommittedTotalSizeInv_ > BodyConst::SizeThresholdToCommit
			|| crtSize * lastCommittedTotalSizeInv_ < BodyConst::SizeThresholdToCommit_inv) {
		commit();
#warning "this is flawed, must also reconfigure attachments' positions - like apply_scale()"
	}
	LOGLN("fatMass:"<<fatMass_<<"\tinstantEnergyUse:"<<frameUsedEnergy_/dt);
	frameUsedEnergy_ = 0;
}

void Torso::die() {
	commit();
}
