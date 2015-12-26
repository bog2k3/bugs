/*
 * EggLayer.cpp
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#include "EggLayer.h"
#include "BodyConst.h"
#include "../entities/Bug.h"
#include "../math/box2glm.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include "../utils/UpdateList.h"
#include "../utils/log.h"
#include "../entities/Gamete.h"
#include "../World.h"
#include "../neuralnet/InputSocket.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(0.2f, 0.7f, 1.0f);
static const glm::vec3 debug_color_suppressed(1.0f, 0.7f, 0.2f);
static const glm::vec3 debug_color_ripe(0.2f, 1.0f, 0.1f);

#define DEBUG_DRAW_EGGLAYER

EggLayerInitializationData::EggLayerInitializationData()
	: ejectSpeed(BodyConst::initialEggEjectSpeed) {
}

void EggLayer::cacheInitializationData() {
	BodyPart::cacheInitializationData();
	auto data = std::dynamic_pointer_cast<EggLayerInitializationData>(getInitializationData());
	ejectSpeed_ = data->ejectSpeed.clamp(0, BodyConst::MaxEggEjectSpeed);
}

float EggLayer::getInputVMSCoord(unsigned index) const {
	if (index >= 2)
		return 0;
	auto initData = std::dynamic_pointer_cast<EggLayerInitializationData>(getInitializationData());
	if (initData)
		return initData->inputVMSCoord[index].clamp(0, BodyConst::MaxVMSCoordinateValue);
	else
		return 0;
}

EggLayer::EggLayer()
	: BodyPart(BodyPartType::EGGLAYER, std::make_shared<EggLayerInitializationData>())
	, targetEggMass_(BodyConst::initialEggMass)
	, ejectSpeed_(0)
{
	inputs_.push_back(new InputSocket(nullptr, 1));	// [0] - suppress growth
	inputs_.push_back(new InputSocket(nullptr, 1)); // [1] - suppress release

	auto data = std::dynamic_pointer_cast<EggLayerInitializationData>(getInitializationData());
	registerAttribute(GENE_ATTRIB_EGG_EJECT_SPEED, data->ejectSpeed);
	registerAttribute(GENE_ATTRIB_MOTOR_INPUT_COORD, 0, data->inputVMSCoord[0]);
	registerAttribute(GENE_ATTRIB_MOTOR_INPUT_COORD, 1, data->inputVMSCoord[1]);

	physBody_.userObjectType_ = ObjectTypes::BPART_EGGLAYER;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
}

EggLayer::~EggLayer() {
	for (auto &i : inputs_)
		delete i;
	inputs_.clear();
}

void EggLayer::die() {
	if (getUpdateList())
		getUpdateList()->remove(this);
	physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
}

float EggLayer::getMass_tree() {
	return initialSize_ * density_;
}

void EggLayer::draw(RenderContext const& ctx) {
	if (committed_) {
#ifdef DEBUG_DRAW_EGGLAYER
		if (isDead()) {
			glm::vec3 transform = getWorldTransformation();
			glm::vec2 pos = vec3xy(transform);
			float sizeLeft = getFoodValue() / density_;
			ctx.shape->drawCircle(pos, sqrtf(sizeLeft*PI_INV)*0.6f, 0, 12, glm::vec3(0.5f,0,1));
		} else {
			glm::vec3 transform = getWorldTransformation();
			glm::vec2 pos = vec3xy(transform);
			float r_2 = sqrtf(size_*PI_INV) * 0.5f;
			glm::vec3 color = eggMassBuffer_ >= targetEggMass_ ? debug_color_ripe : (suppressGrowth_ ? debug_color_suppressed : debug_color);
			ctx.shape->drawLine(
				pos - glm::rotate(glm::vec2(r_2, 0), transform.z),
				pos + glm::rotate(glm::vec2(r_2, 0), transform.z),
				0, color);
			ctx.shape->drawLine(
				pos - glm::rotate(glm::vec2(r_2, 0), transform.z+PI*0.5f),
				pos + glm::rotate(glm::vec2(r_2, 0), transform.z+PI*0.5f),
				0, color);
		}
#endif // DEBUG_DRAW_EGGLAYER
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_*PI_INV), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 EggLayer::getChildAttachmentPoint(float relativeAngle) {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void EggLayer::update(float dt) {
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
		World::getInstance()->takeOwnershipOf(std::move(egg));
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
		applyScale_tree(1.f);
	}
}

void EggLayer::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
		physBody_.b2Body_->GetWorld()->DestroyJoint(pJoint);
		pJoint = nullptr;
	} else {
		initialSize_ = size_;
	}

	// override density with zygote density
	density_ = BodyConst::ZygoteDensity;

	b2CircleShape shape;
	shape.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdef;
	fdef.density = density_;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	physBody_.b2Body_->CreateFixture(&fdef);

	b2WeldJointDef jdef;
	jdef.bodyA = parent_->getBody().b2Body_;
	jdef.bodyB = physBody_.b2Body_;
	glm::vec2 parentAnchor = parent_->getChildAttachmentPoint(attachmentDirectionParent_);
	jdef.localAnchorA = g2b(parentAnchor);
	glm::vec2 childAnchor = getChildAttachmentPoint(angleOffset_);
	jdef.localAnchorB = g2b(childAnchor);
	pJoint = (b2WeldJoint*) physBody_.b2Body_->GetWorld()->CreateJoint(&jdef);
}

void EggLayer::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}
