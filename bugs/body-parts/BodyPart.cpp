/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "../entities/Bug.h"
#include "../genetics/GeneDefinitions.h"
#include "../ObjectTypesAndFlags.h"

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

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

BodyPartInitializationData::BodyPartInitializationData()
	: localRotation(0)
	, size(BodyConst::initialBodyPartSize)
	, density(BodyConst::initialBodyPartDensity)
{
}

BodyPart::BodyPart(BodyPartType type, std::shared_ptr<BodyPartInitializationData> initialData)
	: type_(type)
	, committed_(false)
	, dontCreateBody_(false)
	, geneValuesCached_(false)
	, localRotation_(0)
	, size_(0.01f)
	, density_(1.f)
	, initialData_(initialData)
	, updateList_(nullptr)
	, lastCommitSize_inv_(0)
	, destroyCalled_(false)
	, dead_(false)
{
	assertDbg (initialData != nullptr);

	registerAttribute(GENE_ATTRIB_LOCAL_ROTATION, initialData_->localRotation);
	registerAttribute(GENE_ATTRIB_SIZE, initialData_->size);

	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
	physBody_.getEntityFunc_ = &getEntityFromBodyPartPhysBody;
}

BodyPart::~BodyPart() {
	assertDbg(destroyCalled_);
}

void BodyPart::destroy() {
	destroyCalled_ = true;
	detach(true);

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

void BodyPart::detach(bool die) {
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
	parent_ = nullptr;
	if (die && !dead_)
		die_tree();*/
}

void BodyPart::detachMotorLines(std::vector<unsigned> const& lines) {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
//	if (parent_)
//		parent_->detachMotorLines(lines);
}

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

	World::getInstance()->queueDeferredAction([this, initialScale] () {
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

void BodyPart::purge_initializationData() {
	initialData_.reset();
}

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

glm::vec3 BodyPart::getWorldTransformation() {
	if (physBody_.b2Body_ && !noFixtures_) {
		return glm::vec3(b2g(physBody_.b2Body_->GetPosition()), physBody_.b2Body_->GetAngle());
	} else {
		// if not committed yet, must compute these values on the fly
		glm::vec3 parentTransform(/*parent_ ? parent_->getWorldTransformation() :*/ glm::vec3(0));
		glm::vec2 pos {0}; //= getParentSpacePosition(); // this will temporarily cache gene values as well - that's ok because
													// we're not committed yet, so we're invisible to other threads
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				/*attachmentDirectionParent_ +*/ localRotation_);
	}
}

void BodyPart::draw(RenderContext const& ctx) {
	if (committed_)
		return;
	glm::vec3 trans = getWorldTransformation();
	glm::vec3 pos(trans.x, trans.y, 0);
	Shape3D::get()->drawLine(pos + glm::vec3(-0.01f, 0, 0), pos + glm::vec3(0.01f, 0, 0), glm::vec3(0.2f, 0.2f, 1.f));
	Shape3D::get()->drawLine(pos + glm::vec3(0, -0.01f, 0), pos + glm::vec3(0, 0.01f, 0), glm::vec3(1.f, 0.2f, 0.2f));
}

void BodyPart::registerAttribute(gene_part_attribute_type type, CummulativeValue& value) {
	registerAttribute(type, 0, value);
}

void BodyPart::registerAttribute(gene_part_attribute_type type, unsigned index, CummulativeValue& value) {
	auto &attrVec = mapAttributes_[type];
	while (attrVec.size() < index + 1)
		attrVec.push_back(nullptr);
	attrVec[index] = &value;
}

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
				World::getInstance()->queueDeferredAction([this] {
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
		World::getInstance()->queueDeferredAction([this] {
			commit();
		});
		committed_now = true;
	}

	return committed_now;
}*/

void BodyPart::consumeEnergy(float amount) {
	/*if (parent_)
		parent_->consumeEnergy(amount);*/
}

/*void BodyPart::die_tree() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (!dead_) {
		die();
		dead_ = true;
		physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
		foodValueLeft_ = size_ * density_;
	}
	for (int i=0; i<nChildren_; i++)
		children_[i]->die_tree();
	onDied.trigger(this);
}*/

void BodyPart::consumeFoodValue(float amount) {
	if (dead_) {
		foodValueLeft_ -= amount;
	}
}

void BodyPart::removeAllLinks() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	//parent_ = nullptr;
	//nChildren_ = 0;
}

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

void BodyPart::cacheInitializationData() {
	localRotation_ = limitAngle(initialData_->localRotation, 2*PI);
	//lateralOffset_ = initialData_->lateralOffset;
	size_ = initialData_->size.clamp(BodyConst::MinBodyPartSize, 1.e10f);
	density_ = initialData_->density.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}

/*void BodyPart::hierarchyMassChanged() {
	if (parent_)
		parent_->hierarchyMassChanged();
}*/

void BodyPart::buildDebugName(std::stringstream &out_stream) const {
#ifndef DEBUG
	throw "BodyPart::buildDebugName(): Don't call this on release builds because it's slow";
#endif
	/*if (parent_) {
		parent_->buildDebugName(out_stream);
		out_stream << "::";
	}
	// compute attachment slice:
	int slice = -1;
	if (parent_)
		slice = (int)(getAttachmentAngle() / (2*PI) * MAX_CHILDREN);
	// compute own name from type & slice:
	switch (type_) {
	case BodyPartType::BONE:
		out_stream << "Bone";
		break;
	case BodyPartType::EGGLAYER:
		out_stream << "EggLayer";
		break;
	case BodyPartType::GRIPPER:
		out_stream << "Gripper";
		break;
	case BodyPartType::JOINT:
		out_stream << "Joint";
		break;
	case BodyPartType::MOUTH:
		out_stream << "Mouth";
		break;
	case BodyPartType::MUSCLE:
		out_stream << "Muscle";
		break;
	case BodyPartType::SENSOR_COMPASS:
		out_stream << "SensorCompass";
		break;
//	case BodyPartType::SENSOR_DIRECTION:
//		out_stream << "SensorDirection";
//		break;
	case BodyPartType::SENSOR_PROXIMITY:
		out_stream << "SensorProximity";
		break;
	case BodyPartType::SENSOR_SIGHT:
		out_stream << "SensorSight";
		break;
	case BodyPartType::TORSO:
		out_stream << "Torso";
		break;
	case BodyPartType::ZYGOTE_SHELL:
		out_stream << "ZygoteShell";
		break;

	default:
		out_stream << "UNKNOWN";
		break;
	}
	if (slice >= 0) {
		out_stream << "(" << slice << ")";
	}*/
}

/*std::string BodyPart::getDebugName() const {
	std::stringstream ss;
	buildDebugName(ss);
	return ss.str();
}*/

Entity* BodyPart::getEntityFromBodyPartPhysBody(PhysicsBody const& body) {
	BodyPart* pPart = static_cast<BodyPart*>(body.userPointer_);
	assertDbg(pPart);
	if (!pPart)
		return nullptr;
	return nullptr; //pPart->getOwner();
}

/*aabb BodyPart::getAABBRecursive() {
	aabb X = physBody_.getAABB();
	for (int i=0; i<nChildren_; i++)
		X = X.reunion(children_[i]->getAABBRecursive());
	return X;
}*/
