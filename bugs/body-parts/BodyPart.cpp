/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "../math/box2glm.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"
#include "../utils/log.h"
#include "../genetics/GeneDefinitions.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <cassert>

BodyPartInitializationData::BodyPartInitializationData()
	: attachmentDirectionParent(0)
	, angleOffset(0)
	, lateralOffset(0)
	, size(BodyConst::initialBodyPartSize)
	, density(BodyConst::initialBodyPartDensity)
{
}

void BodyPartInitializationData::sanitizeData() {
	if (size <= BodyConst::MinBodyPartSize)
		size.reset(BodyConst::MinBodyPartSize);
	if (density <= BodyConst::MinBodyPartDensity)
		density.reset(BodyConst::MinBodyPartDensity);
}

BodyPart::BodyPart(BodyPart* parent, PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData)
	: type_(type)
	, parent_(parent)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
	, keepInitializationData_(false)
	, dontCreateBody_(false)
	, initialData_(initialData)
	, updateList_(nullptr)
	, lastCommitSize_inv_(0)
{
	assert (initialData != nullptr);
	if (parent) {
		parent->add(this);
	}

	registerAttribute(GENE_ATTRIB_ATTACHMENT_ANGLE, initialData_->attachmentDirectionParent);
	registerAttribute(GENE_ATTRIB_LOCAL_ROTATION, initialData_->angleOffset);
	registerAttribute(GENE_ATTRIB_ATTACHMENT_OFFSET, initialData_->lateralOffset);
	registerAttribute(GENE_ATTRIB_SIZE, initialData_->size);
}

BodyPart::~BodyPart() {
	changeParent(nullptr);
	for (int i=0; i<nChildren_; i++)
		delete children_[i];
}

void BodyPart::matchLocation(const Atom<LocationLevelType>* location, int nLevel, std::vector<BodyPart*> *out) {
	assert(nLevel >= 0);
	if (*location & (1<<15)) {
		out->push_back(this);
	}
	for (int i=0; i<nChildren_; i++) {
		if (*location & (1 << i))
			children_[i]->matchLocation(location+1, nLevel-1, out);
	}
}

void BodyPart::applyRecursive(std::function<void(BodyPart* pCurrent)> pred) {
	pred(this);
	for (int i=0; i<nChildren_; i++)
		children_[i]->applyRecursive(pred);
}

void BodyPart::addMotorLine(int line) {
	motorLines_.push_back(line);
	if (parent_)
		parent_->addMotorLine(line);
}

void BodyPart::addSensorLine(int line) {
	sensorLines_.push_back(line);
	if (parent_)
		parent_->addSensorLine(line);
}

void BodyPart::add(BodyPart* part) {
	assert(nChildren_ < MAX_CHILDREN);
	children_[nChildren_++] = part;
}

void BodyPart::changeParent(BodyPart* newParent) {
	if (parent_)
		parent_->remove(this);
	parent_ = newParent;
	if (parent_)
		parent_->add(this);
}

void BodyPart::remove(BodyPart* part) {
	for (int i=0; i<nChildren_; i++)
		if (children_[i] == part) {
			children_[i] = children_[--nChildren_];
			break;
		}
}

glm::vec2 BodyPart::getUpstreamAttachmentPoint() const {
	if (!parent_)
		return glm::vec2(0);
	else {
		glm::vec2 point(parent_->getChildAttachmentPoint(initialData_->attachmentDirectionParent));
		return point;
	}
}

void BodyPart::commit_tree() {
	assert(initialData_);
	if (!committed_) {
		initialData_->sanitizeData();
		computeBodyPhysProps();
	} else
		reverseUpdateCachedProps();
	lastCommitSize_inv_ = 1.f / initialData_->size;
	// perform commit on local node:
	if (type_ != BODY_PART_JOINT) {
		if (!physBody_.b2Body_ && !dontCreateBody_)
			physBody_.create(initialData_->cachedProps);
		commit();
	}
	// perform recursive commit on all non-muscle children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ != BODY_PART_MUSCLE)
			children_[i]->commit_tree();
	}
	// muscles go after all other children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ == BODY_PART_MUSCLE)
			children_[i]->commit_tree();
	}

	if (type_ == BODY_PART_JOINT) {
		commit();
	}
	committed_ = true;
}

void BodyPart::purge_initializationData_tree() {
	assert(committed_);
	if (!keepInitializationData_) {
		initialData_.reset();
	}
	for (int i=0; i<nChildren_; i++)
		children_[i]->purge_initializationData_tree();
}

glm::vec2 BodyPart::getParentSpacePosition() const {
	glm::vec2 upstreamAttach = getUpstreamAttachmentPoint();
	assert(!std::isnan(upstreamAttach.x) && !std::isnan(upstreamAttach.y));
	glm::vec2 localOffset = getChildAttachmentPoint(PI - initialData_->angleOffset);
	assert(!std::isnan(localOffset.x) && !std::isnan(localOffset.y));
	float angle = initialData_->attachmentDirectionParent + initialData_->angleOffset;
	glm::vec2 ret(upstreamAttach - glm::rotate(localOffset, angle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
#warning "must take into account lateral offset"
}

void BodyPart::reverseUpdateCachedProps() {
	// reverse the magic here: get values from the physics engine and put them in our cached props
	// in order to facilitate a recommit with changed data.
	glm::vec3 worldTransform = getWorldTransformation();
	initialData_->cachedProps.angle = worldTransform.z;
	initialData_->cachedProps.position = vec3xy(worldTransform);
	if (physBody_.b2Body_) {
		initialData_->cachedProps.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
		initialData_->cachedProps.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	} else if (parent_) {
		initialData_->cachedProps.velocity = parent_->initialData_->cachedProps.velocity;
		initialData_->cachedProps.angularVelocity = parent_->initialData_->cachedProps.angularVelocity;
	}
}

void BodyPart::computeBodyPhysProps() {
	// do the magic here and update initialData_->cachedProps from other initialData_ fields
	// initialData_->cachedProps must be in world space
	// parent's initialData_->cachedProps are assumed to be in world space at this time
	PhysicsProperties parentProps = parent_ ? parent_->initialData_->cachedProps : PhysicsProperties();
	initialData_->cachedProps.velocity = parentProps.velocity;
	initialData_->cachedProps.angularVelocity = parentProps.angularVelocity;
	// compute parent space position:
	glm::vec2 pos = getParentSpacePosition();
	assert(!std::isnan(pos.x) && !std::isnan(pos.y));
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	assert(!std::isnan(pos.x) && !std::isnan(pos.y));
	initialData_->cachedProps.position = pos;
	// compute world space angle:
	initialData_->cachedProps.angle = parentProps.angle + initialData_->attachmentDirectionParent + initialData_->angleOffset;
	assert(!std::isnan(initialData_->cachedProps.angle));
}

glm::vec3 BodyPart::getWorldTransformation() const {
	if (physBody_.b2Body_) {
		return glm::vec3(b2g(physBody_.b2Body_->GetPosition()), physBody_.b2Body_->GetAngle());
	} else if (initialData_) {
		glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
		glm::vec2 pos = getParentSpacePosition();
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				initialData_->attachmentDirectionParent + initialData_->angleOffset);
	} else {
		assert(false && "no known method to compute world transformation!!!");
		return glm::vec3(0);
	}
}

void BodyPart::draw(RenderContext const& ctx) {
	if (committed_)
		return;
	glm::vec3 trans = getWorldTransformation();
	glm::vec2 pos(trans.x, trans.y);
	ctx.shape->drawLine(pos + glm::vec2(-0.01f, 0), pos + glm::vec2(0.01f, 0), 0, glm::vec3(0.2f, 0.2f, 1.f));
	ctx.shape->drawLine(pos + glm::vec2(0, -0.01f), pos + glm::vec2(0, 0.01f), 0, glm::vec3(1.f, 0.2f, 0.2f));
}

void BodyPart::registerAttribute(gene_part_attribute_type type, CummulativeValue& value) {
	mapAttributes_[type] = &value;
}

UpdateList* BodyPart::getUpdateList() {
	if (updateList_)
		return updateList_;
	else if (parent_) {
		updateList_ = parent_->getUpdateList();
		return updateList_;
	} else
		return nullptr;
}

float BodyPart::getMass_tree() {
	float mass = 0;
	if (initialData_)
		mass = initialData_->size * initialData_->density;
	else if (physBody_.b2Body_)
		mass = physBody_.b2Body_->GetMass();
	else
		assert(false && "no known method to compute the mass of the body part!!!");

	for (int i=0; i<nChildren_; i++)
		mass += children_[i]->getMass_tree();
	return mass;
}

void BodyPart::applyScale_tree(float scale) {
	applyScale_treeImpl(scale, false);
}

bool BodyPart::applyScale_treeImpl(float scale, bool parentChanged) {
	assert(initialData_ && "applyScale_tree cannot be called after purging the initialization data!");
	initialData_->size.reset(initialData_->size * scale);
	bool committed_now = false, should_commit_joint = false;
	if (committed_) {
		if (initialData_->size * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
				|| initialData_->size * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
		{
			lastCommitSize_inv_ = 1.f / initialData_->size;
			if (type_ != BODY_PART_JOINT) {
				commit();
				committed_now = true;
			} else
				should_commit_joint = true;
		}
	}
	bool child_changed = false;
	for (int i=0; i<nChildren_; i++) {
		child_changed |= children_[i]->applyScale_treeImpl(scale, committed_now);
	}
	if (type_ == BODY_PART_JOINT && committed_ && (should_commit_joint || parentChanged || child_changed)) {
		// must commit a joint whenever the threshold is reached, or parent or child has committed
		commit();
		committed_now = true;
	}

	return committed_now;
}

float BodyPart::getDefaultAngle() {
	assert(initialData_ && "getDefaultAngle cannot be called after purging initialization data!");
	return initialData_->attachmentDirectionParent + initialData_->angleOffset;
}

void BodyPart::consumeEnergy(float amount) {
	if (parent_)
		parent_->consumeEnergy(amount);
}

void BodyPart::die_tree() {
	die();
	for (int i=0; i<nChildren_; i++)
		children_[i]->die_tree();
}

void BodyPart::reattachChildren() {
	if (committed_) {
		for (int i=0; i<nChildren_; i++) {
			children_[i]->commit();
		}
	}
}

void BodyPart::draw_tree(RenderContext const& ctx) {
	draw(ctx);
	for (int i=0; i<nChildren_; i++)
		children_[i]->draw_tree(ctx);
}
