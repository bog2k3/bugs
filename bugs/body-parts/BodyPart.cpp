/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../entities/Bug/Bug.h"
#include "../genetics/GeneDefinitions.h"
#include "../ObjectTypesAndFlags.h"
#include "Joint.h"

#include <boglfw/math/box2glm.h>
#include <boglfw/math/aabb.h>
#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>
#include <boglfw/World.h>

#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>

#include <cassert>
#include <sstream>
#include <algorithm>

BodyPart::BodyPart(BodyPartType type, BodyPartContext const& context, BodyCell const& cell, bool suppressPhysicalBody)
	: context_(context)
	, type_(type)
	, size_(cell.size())
	, density_(cell.density())
	, lastCommitSize_inv_(0)
	, destroyCalled_(false)
#ifdef DEBUG
	, divisionPath_(cell.getBranchString())
#endif
{
	if (!suppressPhysicalBody) {
		auto pos = cell.position();
		auto angle = cell.angle();
		World::getInstance().queueDeferredAction([this, pos, angle] {
			PhysicsProperties props(pos, angle, true, {0, 0}, 0.f); //TODO velocity?, angularVelocity?);
			physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
			physBody_.getEntityFunc_ = &getEntityFromBodyPartPhysBody;
			physBody_.create(props);
			lastCommitSize_inv_ = 1.f / size_;
			updateFixtures();
		});
	}
}

BodyPart::BodyPart(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass, BodyPartContext const& context)
	: context_(context)
	, type_(BodyPartType::ZYGOTE_SHELL)
	, density_(BodyConst::ZygoteDensity)
	, lastCommitSize_inv_(0)
	, destroyCalled_(false)
{
	size_ = mass / density_;
	World::getInstance().queueDeferredAction([this, position, velocity, angle, angularVelocity] {
			PhysicsProperties props(position, angle, true, velocity, angularVelocity); //TODO velocity?, angularVelocity?);
			physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
			physBody_.getEntityFunc_ = &getEntityFromBodyPartPhysBody;
			physBody_.create(props);
			updateFixtures();
		});
}

BodyPart::~BodyPart() {
	assertDbg(destroyCalled_);
}

void BodyPart::destroy() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	destroyCalled_ = true;
	die();
	disconnectAllNeighbors();
	destroyFixtures();

#ifdef DEBUG
	// check that no link to this body part remains anywhere
	for (auto bp : context_.owner.getBodyParts())
		if (bp)
			assertDbg(std::find(bp->neighbours_.begin(), bp->neighbours_.end(), this) == bp->neighbours_.end());
	for (auto bp : context_.owner.getDeadBodyParts())
		if (bp)
			assertDbg(std::find(bp->neighbours_.begin(), bp->neighbours_.end(), this) == bp->neighbours_.end());
#endif

	delete this;
}

/*void BodyPart::reverseUpdateCachedProps() {
	// reverse the magic here: get values from the physics engine and put them in our cached props
	// in order to facilitate a recommit with changed data.
	glm::vec3 worldTransform = getWorldTransformation();
	cachedProps_.angle = worldTransform.z;
	cachedProps_.position = vec3xy(worldTransform);
	if (physBody_.b2Body_) {
		cachedProps_.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
		cachedProps_.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	} /*else if (parent_) {
		cachedProps_.velocity = parent_->cachedProps_.velocity;
		cachedProps_.angularVelocity = parent_->cachedProps_.angularVelocity;
	}* /
}*/

/*void BodyPart::computeBodyPhysProps() {
	// do the magic here and update cachedProps from other positioning fields
	// cachedProps must be in world space
	// parent's cachedProps are assumed to be updated and in world space at this time
	PhysicsProperties parentProps = /*parent_ ? parent_->cachedProps_ :* / PhysicsProperties();
	cachedProps_.velocity = parentProps.velocity;
	cachedProps_.angularVelocity = parentProps.angularVelocity;
	// compute parent space position:
	glm::vec2 pos {0};// = getParentSpacePosition();
	assertDbg(!std::isnan(pos.x) && !std::isnan(pos.y));
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	assertDbg(!std::isnan(pos.x) && !std::isnan(pos.y));
	cachedProps_.position = pos;
	// compute world space angle:
	cachedProps_.angle = parentProps.angle /*+ attachmentDirectionParent_* / + localRotation_;
	assertDbg(!std::isnan(cachedProps_.angle));
}*/

glm::vec3 BodyPart::getWorldTransformation() const {
	if (physBody_.b2Body_ /*&& !noFixtures_*/) {
		return glm::vec3(b2g(physBody_.b2Body_->GetPosition()), physBody_.b2Body_->GetAngle());
	} else {
		assertDbg(!!!"This should never happen");
		return glm::vec3(0);
		// if not committed yet, must compute these values on the fly
//		glm::vec3 parentTransform(/*parent_ ? parent_->getWorldTransformation() :*/ glm::vec3(0));
//		glm::vec2 pos = getParentSpacePosition(); // this will temporarily cache gene values as well - that's ok because
//													// we're not committed yet, so we're invisible to other threads
//		return parentTransform + glm::vec3(
//				glm::rotate(pos, parentTransform.z),
//				/*attachmentDirectionParent_ +*/ localRotation_);
	}
}

void BodyPart::draw(Viewport* vp) {
	glm::vec3 trans = getWorldTransformation();
	glm::vec3 pos(trans.x, trans.y, 0);
	glm::vec3 u = {glm::rotate(glm::vec2(1.f, 0.f), trans.z), 0};
	glm::vec3 v = {glm::rotate(glm::vec2(0.f, 1.f), trans.z), 0};
	Shape3D::get()->drawLine(pos - 0.01f*u, pos + 0.05f*u, glm::vec3(0.2f, 0.2f, 1.f));
	Shape3D::get()->drawLine(pos - 0.01f*v, pos + 0.01f*v, glm::vec3(1.f, 0.2f, 0.2f));
}

void BodyPart::destroyFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (!physBody_.b2Body_)
		return;
	auto f = physBody_.b2Body_->GetFixtureList();
	while (f) {
		auto fn = f->GetNext();
		physBody_.b2Body_->DestroyFixture(f);
		f = fn;
	}
}

void BodyPart::applyScale(float scale) {
	size_ *= scale;
	if (type_ == BodyPartType::JOINT_PIVOT || type_ == BodyPartType::JOINT_WELD)
		return;
	if (size_ * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
			|| size_ * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
	{
		lastCommitSize_inv_ = 1.f / size_;
		World::getInstance().queueDeferredAction([this] {
			destroyFixtures();
			updateFixtures();
		});
		// must update all neighbouring joints since the fixture has changed
		for (auto n : neighbours_) {
			if (n->type_ == BodyPartType::JOINT_PIVOT || n->type_ == BodyPartType::JOINT_WELD) {
				context_.owner.recreateJoint(static_cast<Joint*>(n));
			}
		}
	}
}

void BodyPart::die() {
	bool expect = false;
	if (!dead_.compare_exchange_strong(expect, true, std::memory_order_acq_rel, std::memory_order_relaxed)) {
		return;
	}
#ifdef DEBUG
	DEBUGLOGLN("BodyPart::die() : " << getDebugName());
#endif
	physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
	foodValueLeft_ = size_ * density_;
	if (!destroyCalled()) {
		World::getInstance().queueDeferredAction([this] {
			onDied.trigger(this);
		});
	}
}

void BodyPart::consumeFoodValue(float amount) {
	if (isDead()) {
		foodValueLeft_ -= amount;
	}
}

Entity* BodyPart::getEntityFromBodyPartPhysBody(PhysicsBody const& body) {
	BodyPart* pPart = static_cast<BodyPart*>(body.userPointer_);
	assertDbg(pPart);
	if (!pPart)
		return nullptr;
	return &pPart->context_.owner;
}

aabb BodyPart::getAABB() const {
	return physBody_.getAABB();
}

std::pair<float, float> BodyPart::adjustFixtureValues(std::pair<float, float> const& val, float &outTotalRatio) {
	float v1 = val.first, v2 = val.second;
	bool combined = val.second == 0;
	if (combined)
		v1 = sqrt(v1);
	outTotalRatio = 1.f;
	if (v1 < b2_linearSlop) {
		outTotalRatio *= b2_linearSlop / v1;
		v1 = b2_linearSlop;
	}
	if (!combined && v2 < b2_linearSlop) {
		outTotalRatio *= b2_linearSlop / v2;
		v2 = b2_linearSlop;
	}
	return {combined ? sqr(v1) : v1, v2};
}

glm::vec3 BodyPart::worldToLocal(glm::vec3 in, bool isDirection) const {
	auto tr = getWorldTransformation();
	if (!isDirection)
		in.x -= tr.x, in.y -= tr.y;
	return { glm::rotate(vec3xy(in), -tr.z), in.z - tr.z };
}

glm::vec2 BodyPart::worldToLocal(glm::vec2 tr, bool isDirection) const {
	return vec3xy(worldToLocal(glm::vec3{tr, 0}, isDirection));
}

glm::vec3 BodyPart::localToWorld(glm::vec3 const& in, bool isDirection) const {
	auto tr = getWorldTransformation();
	glm::vec2 w = glm::rotate(vec3xy(in), tr.z);
	if (!isDirection)
		w.x += tr.x, w.y += tr.y;
	return { w, in.z + tr.z };
}

glm::vec2 BodyPart::localToWorld(glm::vec2 const& tr, bool isDirection) const {
	return vec3xy(localToWorld(glm::vec3{tr, 0}, isDirection));
}

void BodyPart::overrideSizeAndDensity(float newSize, float newDensity) {
	assert(physBody_.b2Body_ == nullptr); // don't call this if you have a physical body already!
	size_ = newSize;
	density_ = newDensity;
}

bool BodyPart::applyPredicateGraph(std::function<bool(const BodyPart* pCurrent)> const& pred, uint32_t UOID) const {
	while (UOID==0)
		UOID = new_RID();
	UOID_ = UOID;

	if (pred(this))
		return true;
	for (auto n : neighbours_) {
		if (n->UOID_ == UOID)
			continue;
		if (n->applyPredicateGraph(pred, UOID))
			return true;
	}
	return false;
}

bool BodyPart::applyPredicateGraphMutable(std::function<bool(BodyPart* pCurrent)> const& pred, uint32_t UOID) {
	while (UOID==0)
		UOID = new_RID();
	UOID_ = UOID;

	if (pred(this))
		return true;
	auto copyNeighbours = neighbours_;	// must iterate over a copy because the predicate may affect the vector and invalidate iterators
	for (auto n : copyNeighbours) {
		if (n->UOID_ == UOID)
			continue;
		if (n->applyPredicateGraphMutable(pred, UOID))
			return true;
	}
	return false;
}

void BodyPart::addNeighbor(BodyPart* n) {
#ifdef DEBUG
	auto it = std::find(neighbours_.begin(), neighbours_.end(), n);
	assert(it == neighbours_.end() && "Attempting to add the same neighbor twice");
#endif
	neighbours_.push_back(n);
}

void BodyPart::removeNeighbor(BodyPart* n) {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	auto it = std::find(neighbours_.begin(), neighbours_.end(), n);
	if (it != neighbours_.end())
		neighbours_.erase(it);
}

void BodyPart::disconnectAllNeighbors() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	std::vector<Joint*> jointsToBreak;
	for (auto n : neighbours_) {
		if (n->type_ == BodyPartType::JOINT_PIVOT || n->type_ == BodyPartType::JOINT_WELD)
			jointsToBreak.push_back(static_cast<Joint*>(n));
		n->removeNeighbor(this);
	}
	neighbours_.clear();
	// break joints after the neighbor loop to avoid screwing iterators from recursively calling removeNeighbor
	for (auto j : jointsToBreak)
		j->breakJoint();
}

#ifdef DEBUG
std::string BodyPart::getDebugName() const {
	// compute own name from type & division branch:
	std::string type;
	switch (type_) {
	case BodyPartType::BONE:
		type = "Bone";
		break;
	case BodyPartType::EGGLAYER:
		type = "EggLayer";
		break;
	case BodyPartType::GRIPPER:
		type = "Gripper";
		break;
	case BodyPartType::JOINT_PIVOT:
		type = "JointPivot";
		break;
	case BodyPartType::JOINT_WELD:
		type = "JointWeld";
		break;
	case BodyPartType::MOUTH:
		type = "Mouth";
		break;
	case BodyPartType::MUSCLE:
		type = "Muscle";
		break;
	case BodyPartType::SENSOR_COMPASS:
		type = "SensorCompass";
		break;
//	case BodyPartType::SENSOR_DIRECTION:
//		type = "SensorDirection";
//		break;
	case BodyPartType::SENSOR_PROXIMITY:
		type = "SensorProximity";
		break;
	case BodyPartType::SENSOR_SIGHT:
		type = "SensorSight";
		break;
	case BodyPartType::ZYGOTE_SHELL:
		type = "ZygoteShell";
		break;
	case BodyPartType::FAT:
		type = "Fat";
		break;

	default:
		type = "UNKNOWN";
		break;
	}

	return type + " [" + divisionPath_ + "]";
}
#endif
