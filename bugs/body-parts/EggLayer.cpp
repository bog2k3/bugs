/*
 * EggLayer.cpp
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#include "EggLayer.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../entities/Bug.h"
#include "../entities/Gamete.h"
#include "../neuralnet/InputSocket.h"
#include "../ObjectTypesAndFlags.h"

#include <boglfw/math/box2glm.h>
#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/World.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/log.h>
#include <boglfw/perf/marker.h>

#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(0.2f, 0.7f, 1.0f);
static const glm::vec3 debug_color_suppressed(1.0f, 0.7f, 0.2f);
static const glm::vec3 debug_color_ripe(0.2f, 1.0f, 0.1f);

#define DEBUG_DRAW_EGGLAYER

EggLayer::EggLayer(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::EGGLAYER, context, cell)
	, targetEggMass_(BodyConst::initialEggMass)
{
	inputs_[0] = new InputSocket(nullptr, 1);	// [0] - suppress growth
	inputs_[1] = new InputSocket(nullptr, 1); 	// [1] - suppress release
	VMSCoords_[0] = cell.mapAttributes_[GENE_ATTRIB_VMS_COORD1].clamp(0, BodyConst::MaxVMSCoordinateValue);
	VMSCoords_[1] = cell.mapAttributes_[GENE_ATTRIB_VMS_COORD2].clamp(0, BodyConst::MaxVMSCoordinateValue);
	cell.mapAttributes_[GENE_ATTRIB_GENERIC1].changeAbs(BodyConst::initialEggEjectSpeed);
	ejectSpeed_ = cell.mapAttributes_[GENE_ATTRIB_GENERIC1].clamp(0, BodyConst::MaxEggEjectSpeed);
	initialSize_ = size_;

	physBody_.userObjectType_ = ObjectTypes::BPART_EGGLAYER;
	physBody_.userPointer_ = this;

	context.updateList.add(this);

	onDied.add([this](BodyPart*) {
		context_.updateList.remove(this);
	});
}

EggLayer::~EggLayer() {
	for (auto &i : inputs_) {
		delete i;
		i = nullptr;
	}
}

void EggLayer::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_EGGLAYER
	glm::vec3 transform = getWorldTransformation();
	glm::vec3 pos {vec3xy(transform), 0};
	if (isDead()) {
		float sizeLeft = getFoodValue() / density_;
		Shape3D::get()->drawCircleXOY(pos, sqrtf(sizeLeft*PI_INV)*0.6f, 12, glm::vec3(0.5f,0,1));
	} else {
		float r_2 = sqrtf(size_*PI_INV) * 0.5f;
		glm::vec3 color = eggMassBuffer_ >= targetEggMass_ ? debug_color_ripe : (suppressGrowth_ ? debug_color_suppressed : debug_color);
		Shape3D::get()->drawLine(
			pos - glm::vec3(glm::rotate(glm::vec2(r_2, 0), transform.z), 0),
			pos + glm::vec3(glm::rotate(glm::vec2(r_2, 0), transform.z), 0),
			color);
		Shape3D::get()->drawLine(
			pos - glm::vec3(glm::rotate(glm::vec2(r_2, 0), transform.z+PI*0.5f), 0),
			pos + glm::vec3(glm::rotate(glm::vec2(r_2, 0), transform.z+PI*0.5f), 0),
			color);
	}
#endif // DEBUG_DRAW_EGGLAYER
}

static glm::vec2 getEggLayerAttachmentPoint(float size, float angle) {
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

glm::vec2 EggLayer::getAttachmentPoint(float relativeAngle) {
	return getEggLayerAttachmentPoint(size_, relativeAngle);
}

float EggLayer::getDensity(BodyCell const& cell) {
	return BodyConst::ZygoteDensity;
}

float EggLayer::getRadius(BodyCell const& cell, float angle) {
	glm::vec2 p = getEggLayerAttachmentPoint(cell.size(), angle);
	return glm::length(p);
}

void EggLayer::update(float dt) {
	PERF_MARKER_FUNC;
	if (isDead())
		return;
	suppressGrowth_ = inputs_[0]->value > 0;
	suppressRelease_ = inputs_[1]->value > 0;

	if (eggMassBuffer_ >= targetEggMass_ && !suppressRelease_) {
		// lay an egg here
		LOGLN("MAKE EGG! !!!!! ! !");
		Chromosome chr = GeneticOperations::meyosis(getOwner()->getGenome());
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 speed = glm::rotate(glm::vec2(1, 0), transform.z) * ejectSpeed_;
		std::unique_ptr<Gamete> egg(new Gamete(chr, vec3xy(transform), speed, targetEggMass_));
		egg->generation_ = getOwner()->getGeneration() + 1;
		World::getInstance().takeOwnershipOf(std::move(egg));
		eggMassBuffer_ -= targetEggMass_;
	}
}

float EggLayer::getFoodRequired() {
	if (!suppressGrowth_)
		return std::max(targetEggMass_ - eggMassBuffer_, 0.f);
	else
		return 0;
}

void EggLayer::useFood(float food) {
	if (!suppressGrowth_) {
		eggMassBuffer_ += food;
		size_ = initialSize_ + eggMassBuffer_ * BodyConst::ZygoteDensityInv;
		//applyScale_tree(1.f);
		throw std::runtime_error("Implement!");
	}
}

void EggLayer::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (physBody_.b2Body_->GetFixtureList()) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
//		physBody_.b2Body_->GetWorld()->DestroyJoint(pJoint);
//		pJoint = nullptr;
	}

	float fRatio;
	auto fSize = adjustFixtureValues({size_, 0.f}, fRatio);

	b2CircleShape shape;
	shape.m_radius = sqrtf(fSize.first * PI_INV);
	b2FixtureDef fdef;
	fdef.density = density_ / fRatio;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	physBody_.b2Body_->CreateFixture(&fdef);

	/*b2WeldJointDef jdef;
	jdef.bodyA = parent_->getBody().b2Body_;
	jdef.bodyB = physBody_.b2Body_;
	glm::vec2 parentAnchor = parent_->getAttachmentPoint(attachmentDirectionParent_);
	jdef.localAnchorA = g2b(parentAnchor);
	glm::vec2 childAnchor = getAttachmentPoint(PI - localRotation_);
	jdef.localAnchorB = g2b(childAnchor);
	pJoint = (b2WeldJoint*) physBody_.b2Body_->GetWorld()->CreateJoint(&jdef);*/
}

/*void EggLayer::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}*/
