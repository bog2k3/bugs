/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../entities/Bug.h"
#include "../genetics/GeneDefinitions.h"
#include "../ObjectTypesAndFlags.h"
#include "Joint.h"

#include <boglfw/math/box2glm.h>
#include <boglfw/math/aabb.h>
#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>
#include <boglfw/World.h>

#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>

#include <cassert>
#include <sstream>
#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

/*BodyPartInitializationData::BodyPartInitializationData()
	: localRotation(0)
	, size(BodyConst::initialBodyPartSize)
	, density(BodyConst::initialBodyPartDensity)
{
}*/

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

//	for (int i=0; i<nChildren_; i++)
//		children_[i]->destroy();
	delete this;
}

//bool BodyPart::applyRecursive(std::function<bool(BodyPart* pCurrent)> pred) {
//	if (pred(this))
//		return true;
//	for (int i=0; i<nChildren_; i++)
//		if (children_[i]->applyRecursive(pred))
//			return true;
//	return false;
//}

void BodyPart::addMotorLine(int lineId) {
	motorLines_.push_back(lineId);
//	if (parent_)
//		parent_->addMotorLine(lineId);
}

//float BodyPart::add(BodyPart* part, float angle) {
//	angle = limitAngle(angle, 2*PI);
//	assertDbg(nChildren_ < (int)MAX_CHILDREN && initialData_);
//	// determine the position in buffer:
//	int bufferPos = 0;
//	while (bufferPos < nChildren_ && angle >= children_[bufferPos]->attachmentDirectionParent_)
//		bufferPos++;
//	for (int i=nChildren_; i>bufferPos; i--) {
//		// initialData_->circularBuffer[i] = initialData_->circularBuffer[i-1];
//		children_[i] = children_[i-1];
//	}
//	nChildren_++;
//	children_[bufferPos] = part;
//	part->parent_ = this;
//	part->setAttachmentDirection(angle);
//	part->onAddedToParent();
//	return children_[bufferPos]->attachmentDirectionParent_;
//}

/*void BodyPart::detach(bool die) {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	/*if (parent_) {
		// first must detach all neural connections
		detachMotorLines(motorLines_);
		parent_->remove(this);
		onDetachedFromParent();
		parent_->hierarchyMassChanged();
	}
	parent_ = nullptr;* /
	if (die && !dead_)
		this->die();
}*/

/*void BodyPart::detachMotorLines(std::vector<unsigned> const& lines) {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
//	if (parent_)
//		parent_->detachMotorLines(lines);
}*/

/*void BodyPart::remove(BodyPart* part) {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	for (int i=0; i<nChildren_; i++)
		if (children_[i] == part) {
			children_[i] = children_[--nChildren_];
			break;
		}
}*/

/*glm::vec2 BodyPart::getUpstreamAttachmentPoint() {
	if (!parent_)
		return glm::vec2(0);
	else {
		glm::vec2 point(parent_->getAttachmentPoint(attachmentDirectionParent_));
		assertDbg(!std::isnan(point.x) && !std::isnan(point.y));
		return point;
	}
}*/

/*void BodyPart::commit_tree(float initialScale) {
	if (!committed_) {
		initialData_->size.changeRel(initialScale);
		cacheInitializationData();
		geneValuesCached_ = true;
		purge_initializationData();
		computeBodyPhysProps();
	} else
		reverseUpdateCachedProps();
	lastCommitSize_inv_ = 1.f / size_;

	World::getInstance().queueDeferredAction([this, initialScale] () {
		// perform commit on local node:
		if (type_ != BodyPartType::JOINT) {
			if (!physBody_.b2Body_ && !dontCreateBody_)
				physBody_.create(cachedProps_);
			commit();
		}
		// perform recursive commit on all non-muscle children:
		for (int i=0; i<nChildren_; i++) {
			if (children_[i]->type_ != BodyPartType::MUSCLE)
				children_[i]->commit_tree(initialScale);
		}
		// muscles go after all other children:
		for (int i=0; i<nChildren_; i++) {
			if (children_[i]->type_ == BodyPartType::MUSCLE)
				children_[i]->commit_tree(initialScale);
		}

		if (type_ == BodyPartType::JOINT) {
			commit();
		}
		committed_ = true;
	});
}*/

/*void BodyPart::purge_initializationData() {
	initialData_.reset();
}*/

/*glm::vec2 BodyPart::getParentSpacePosition() {
	if (!geneValuesCached_) {
#ifdef DEBUG
		World::assertOnMainThread();
#endif
		cacheInitializationData();
	}
	glm::vec2 upstreamAttach = getUpstreamAttachmentPoint();
	glm::vec2 localOffset = getAttachmentPoint(PI - localRotation_);
	assertDbg(!std::isnan(localOffset.x) && !std::isnan(localOffset.y));
	float angle;
	angle = attachmentDirectionParent_ + localRotation_;
	glm::vec2 ret(upstreamAttach - glm::rotate(localOffset, angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
#warning "must take into account lateral offset"
}*/

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

void BodyPart::draw(RenderContext const& ctx) {
	glm::vec3 trans = getWorldTransformation();
	glm::vec3 pos(trans.x, trans.y, 0);
	glm::vec3 u = {glm::rotate(glm::vec2(1.f, 0.f), trans.z), 0};
	glm::vec3 v = {glm::rotate(glm::vec2(0.f, 1.f), trans.z), 0};
	Shape3D::get()->drawLine(pos - 0.01f*u, pos + 0.05f*u, glm::vec3(0.2f, 0.2f, 1.f));
	Shape3D::get()->drawLine(pos - 0.01f*v, pos + 0.01f*v, glm::vec3(1.f, 0.2f, 0.2f));
}

/*void BodyPart::registerAttribute(gene_part_attribute_type type, CumulativeValue& value) {
	registerAttribute(type, 0, value);
}

void BodyPart::registerAttribute(gene_part_attribute_type type, unsigned index, CumulativeValue& value) {
	auto &attrVec = mapAttributes_[type];
	while (attrVec.size() < index + 1)
		attrVec.push_back(nullptr);
	attrVec[index] = &value;
}*/

/*UpdateList* BodyPart::getUpdateList() {
	if (updateList_)
		return updateList_;
	else if (parent_) {
		updateList_ = parent_->getUpdateList();
		return updateList_;
	} else
		return nullptr;
}*/

/*float BodyPart::getMass_tree() {
	float mass = size_ * density_;

	for (int i=0; i<nChildren_; i++)
		mass += children_[i]->getMass_tree();
	return mass;
}*/

/*void BodyPart::applyScale_tree(float scale) {
	applyScale_treeImpl(scale, false);
}*/

/*bool BodyPart::applyScale_treeImpl(float scale, bool parentChanged) {
	size_ *= scale;
	bool committed_now = false, should_commit_joint = false;
	if (committed_) {
		if (size_ * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
				|| size_ * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
		{
			lastCommitSize_inv_ = 1.f / size_;
			if (type_ != BodyPartType::JOINT) {
				World::getInstance().queueDeferredAction([this] {
					commit();
				});
				committed_now = true;
			} else
				should_commit_joint = true;
		}
	}
	bool child_changed = false;
	for (int i=0; i<nChildren_; i++) {
		child_changed |= children_[i]->applyScale_treeImpl(scale, committed_now);
	}
	if (type_ == BodyPartType::JOINT && committed_ && (should_commit_joint || parentChanged || child_changed)) {
		// must commit a joint whenever the threshold is reached, or parent or child has committed
		World::getInstance().queueDeferredAction([this] {
			commit();
		});
		committed_now = true;
	}

	return committed_now;
}*/

void BodyPart::die() {
	if (!isDead()) {
		dead_.store(true, std::memory_order_release);
		physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
		foodValueLeft_ = size_ * density_;
		onDied.trigger(this);
	}
}

void BodyPart::consumeFoodValue(float amount) {
	if (isDead()) {
		foodValueLeft_ -= amount;
	}
}

/*void BodyPart::removeAllLinks() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	throw std::runtime_error("implement!");
	//parent_ = nullptr;
	//nChildren_ = 0;
}*/

/*void BodyPart::reattachChildren() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (committed_) {
		for (int i=0; i<nChildren_; i++) {
			children_[i]->commit();
		}
	}
}*/

/*void BodyPart::draw_tree(RenderContext const& ctx) {
	draw(ctx);
	for (int i=0; i<nChildren_; i++)
		children_[i]->draw_tree(ctx);
}*/

/*void BodyPart::cacheInitializationData() {
	localRotation_ = limitAngle(initialData_->localRotation, 2*PI);
	//lateralOffset_ = initialData_->lateralOffset;
	size_ = initialData_->size.clamp(BodyConst::MinBodyPartSize, 1.e10f);
	density_ = initialData_->density.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}*/

/*void BodyPart::hierarchyMassChanged() {
	if (parent_)
		parent_->hierarchyMassChanged();
}*/

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

bool BodyPart::applyPredicateGraph(std::function<bool(BodyPart* pCurrent)> pred, uint32_t UOID) {
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
	neighbours_.erase(std::remove(neighbours_.begin(), neighbours_.end(), n), neighbours_.end());
}

void BodyPart::disconnectAllNeighbors() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	std::vector<Joint*> jointsToBreak;
	for (auto n : neighbours_) {
		if (n->type_ == BodyPartType::JOINT_PIVOT || n->type_ == BodyPartType::JOINT_WELD)
			jointsToBreak.push_back(static_cast<Joint*>(n));
		else
			n->removeNeighbor(this);
	}
	neighbours_.clear();
	// break joints after the neighbor loop to avoid screwing iterators from recursively calling removeNeighbor
	for (auto j : jointsToBreak)
		j->breakJoint();
}

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
