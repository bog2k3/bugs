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
#include "../utils/assert.h"
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

BodyPart::BodyPart(BodyPart* parent, PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData)
	: type_(type)
	, parent_(parent)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
	, dontCreateBody_(false)
	, geneValuesCached_(false)
	, attachmentDirectionParent_(0)
	, angleOffset_(0)
	, lateralOffset_(0)
	, size_(0.01f)
	, density_(1.f)
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

glm::vec2 BodyPart::getUpstreamAttachmentPoint() {
	if (!parent_)
		return glm::vec2(0);
	else {
		glm::vec2 point(parent_->getChildAttachmentPoint(attachmentDirectionParent_));
		assert(!std::isnan(point.x) && !std::isnan(point.y));
		return point;
	}
}

void BodyPart::commit_tree(float initialScale) {
	if (!committed_) {
		cacheInitializationData();
		geneValuesCached_ = true;
		purge_initializationData();
		computeBodyPhysProps();
	} else
		reverseUpdateCachedProps();
	size_ *= initialScale;
	lastCommitSize_inv_ = 1.f / size_;
	// perform commit on local node:
	if (type_ != BODY_PART_JOINT) {
		if (!physBody_.b2Body_ && !dontCreateBody_)
			physBody_.create(cachedProps_);
		commit();
	}
	// perform recursive commit on all non-muscle children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ != BODY_PART_MUSCLE)
			children_[i]->commit_tree(initialScale);
	}
	// muscles go after all other children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ == BODY_PART_MUSCLE)
			children_[i]->commit_tree(initialScale);
	}

	if (type_ == BODY_PART_JOINT) {
		commit();
	}
	committed_ = true;
}

void BodyPart::purge_initializationData() {
	initialData_.reset();
}

glm::vec2 BodyPart::getParentSpacePosition() {
	glm::vec2 upstreamAttach = getUpstreamAttachmentPoint();
	glm::vec2 localOffset = getChildAttachmentPoint(PI - angleOffset_);
	assert(!std::isnan(localOffset.x) && !std::isnan(localOffset.y));
	float angle;
	if (!geneValuesCached_)
		angle = limitAngle(initialData_->attachmentDirectionParent + initialData_->angleOffset, 2*PI);
	else
		angle = attachmentDirectionParent_ + angleOffset_;
	glm::vec2 ret(upstreamAttach - glm::rotate(localOffset, angle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
#warning "must take into account lateral offset"
}

void BodyPart::reverseUpdateCachedProps() {
	// reverse the magic here: get values from the physics engine and put them in our cached props
	// in order to facilitate a recommit with changed data.
	glm::vec3 worldTransform = getWorldTransformation();
	cachedProps_.angle = worldTransform.z;
	cachedProps_.position = vec3xy(worldTransform);
	if (physBody_.b2Body_) {
		cachedProps_.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
		cachedProps_.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	} else if (parent_) {
		cachedProps_.velocity = parent_->cachedProps_.velocity;
		cachedProps_.angularVelocity = parent_->cachedProps_.angularVelocity;
	}
}

void BodyPart::computeBodyPhysProps() {
	// do the magic here and update cachedProps from other positioning fields
	// cachedProps must be in world space
	// parent's cachedProps are assumed to be updated and in world space at this time
	PhysicsProperties parentProps = parent_ ? parent_->cachedProps_ : PhysicsProperties();
	cachedProps_.velocity = parentProps.velocity;
	cachedProps_.angularVelocity = parentProps.angularVelocity;
	// compute parent space position:
	glm::vec2 pos = getParentSpacePosition();
	assert(!std::isnan(pos.x) && !std::isnan(pos.y));
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	assert(!std::isnan(pos.x) && !std::isnan(pos.y));
	cachedProps_.position = pos;
	// compute world space angle:
	cachedProps_.angle = parentProps.angle + attachmentDirectionParent_ + angleOffset_;
	assert(!std::isnan(cachedProps_.angle));
}

glm::vec3 BodyPart::getWorldTransformation() {
	if (physBody_.b2Body_) {
		return glm::vec3(b2g(physBody_.b2Body_->GetPosition()), physBody_.b2Body_->GetAngle());
	} else {
		// if not committed yet, must compute these values on the fly
		glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
		glm::vec2 pos = getParentSpacePosition();
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				attachmentDirectionParent_ + angleOffset_);
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
	float mass = size_ * density_;

	for (int i=0; i<nChildren_; i++)
		mass += children_[i]->getMass_tree();
	return mass;

#warning "muscle mass must somehow count toward total body mass - maybe add it to torso?"
}

void BodyPart::applyScale_tree(float scale) {
	applyScale_treeImpl(scale, false);
}

bool BodyPart::applyScale_treeImpl(float scale, bool parentChanged) {
	size_ *= scale;
	bool committed_now = false, should_commit_joint = false;
	if (committed_) {
		if (size_ * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
				|| size_ * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
		{
			lastCommitSize_inv_ = 1.f / size_;
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

void BodyPart::cacheInitializationData() {
	attachmentDirectionParent_ = limitAngle(initialData_->attachmentDirectionParent, 2*PI);
	angleOffset_ = limitAngle(initialData_->angleOffset, 2*PI);
	lateralOffset_ = initialData_->lateralOffset;
	size_ = initialData_->size.clamp(BodyConst::MinBodyPartSize, 1.e10f);
	density_ = initialData_->density.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}
