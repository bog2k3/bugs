/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "../../math/box2glm.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../math/math2D.h"
#include "../../log.h"
#include "../../genetics/GeneDefinitions.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <assert.h>

BodyPartInitializationData::BodyPartInitializationData()
	: size(BodyConst::initialBodyPartSize)
	, density(BodyConst::initialBodyPartDensity) {
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
		children_[i]->changeParent(nullptr);
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
	else
		return parent_->getChildAttachmentPoint(initialData_->attachmentDirectionParent);
}

void BodyPart::commit_tree() {
	assert(initialData_);
	if (!committed_)
		computeBodyPhysProps();
	lastCommitSize_inv_ = 1.f / initialData_->size;
	// perform commit on local node:
	if (type_ != BODY_PART_JOINT) {
		if (!body_ && !dontCreateBody_)
			WorldObject::createPhysicsBody(initialData_->cachedProps);
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

glm::vec2 BodyPart::computeParentSpacePosition() const {
	return getUpstreamAttachmentPoint() - glm::rotate(
			getChildAttachmentPoint(PI - initialData_->angleOffset),
			(float)initialData_->attachmentDirectionParent + initialData_->angleOffset);
#warning "must take into account lateral offset"
}

void BodyPart::computeBodyPhysProps() {
	// do the magic here and update initialData_->cachedProps from other initialData_ fields
	// initialData_->cachedProps must be in world space
	// parent's initialData_->cachedProps are assumed to be in world space at this time
	PhysicsProperties parentProps = parent_ ? parent_->initialData_->cachedProps : PhysicsProperties();
	initialData_->cachedProps.velocity = parentProps.velocity;
	initialData_->cachedProps.angularVelocity = parentProps.angularVelocity;
	// compute parent space position:
	glm::vec2 pos = computeParentSpacePosition();
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	initialData_->cachedProps.position = pos;
	// compute world space angle:
	initialData_->cachedProps.angle = parentProps.angle + initialData_->attachmentDirectionParent + initialData_->angleOffset;
}

glm::vec3 BodyPart::getWorldTransformation(bool force_recompute /*=false*/) const {
	if (body_ && !force_recompute) {
		return glm::vec3(b2g(body_->GetPosition()), body_->GetAngle());
	} else if (initialData_) {
		glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
		glm::vec2 pos = computeParentSpacePosition();
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				initialData_->attachmentDirectionParent + initialData_->angleOffset);
	} else {
		assert(false && "no known method to compute world transformation!!!");
		return glm::vec3(0);
	}
}

void BodyPart::draw(RenderContext& ctx) {
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

CummulativeValue* BodyPart::getAttribute(gene_part_attribute_type attrib) {
	return mapAttributes_[attrib];
}

UpdateList* BodyPart::getUpdateList() const {
	if (updateList_)
		return updateList_;
	else if (parent_)
		return parent_->getUpdateList();
	else
		return nullptr;
}

float BodyPart::getMass_tree() {
	float mass;
	if (initialData_)
		mass = initialData_->size * initialData_->density;
	else if (body_)
		mass = body_->GetMass();
	else
		assert(false && "no known method to compute the mass of the body part!!!");

	for (int i=0; i<nChildren_; i++)
		mass += children_[i]->getMass_tree();
	return mass;
}

void BodyPart::applyScale_tree(float scale) {
	assert(initialData_ && "applyScale_tree cannot be called after purging the initialization data!");
	initialData_->size.reset(initialData_->size * scale);
	if (initialData_->size * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
			|| initialData_->size * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
	{
		lastCommitSize_inv_ = 1.f / initialData_->size;
		commit();
		/*if (body_)
			body_->SetTransform(g2b(vec3xy(getWorldTransformation(true))), body_->GetAngle());*/
#error "must also move bodies when changing fixture size"
		// this is not easy since the bodies may be in a different position from the default, the joints may
		// be bent away from their default angle.
		// must figure out a way, and preserve the original properties of the joints (min and max angles, distance between bodies,
		// anchor points).
	}
	for (int i=0; i<nChildren_; i++)
		children_[i]->applyScale_tree(scale);
}
