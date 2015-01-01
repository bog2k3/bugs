/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "../../math/box2glm.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../math/math2D.h"
#include "../../log.h"
#include "../../genetics/GeneDefinitions.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <assert.h>

BodyPart::BodyPart(BodyPart* parent, PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData)
	: type_(type)
	, parent_(parent)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
	, keepInitializationData_(false)
	, dontCreateBody_(false)
	, initialData_(initialData)
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

/*
void BodyPart::transform_position_and_angle() {
	if (parent_) {
		glm::vec3 wt = getWorldTransformation();
		initialData_->angle = wt.z;
		initialData_->position = vec3xy(wt);
	}
	coordinates_local_ = false;
}*/

void BodyPart::commit_tree() {
	assert(!committed_);
	//LOGGER("commit_tree");
	// initialData_->position = getFinalPrecommitPosition();
	computeBodyPhysProps();
	// perform commit on local node:
	if (type_ != BODY_PART_JOINT) {
		if (!dontCreateBody_)
			WorldObject::createPhysicsBody(initialData_->cachedProps);
		commit();
	}
	// perform recursive commit on all children:
	for (int i=0; i<nChildren_; i++)
		children_[i]->commit_tree();

	if (type_ == BODY_PART_JOINT) {
		commit();
	}
	if (!keepInitializationData_) {
		initialData_.reset();
	}
	committed_ = true;
}

/*glm::vec2 BodyPart::getFinalPrecommitPosition() {
	if (parent_) {
		// update attachment point (since parent may have changed its size or aspect ratio):
		glm::vec2 pos = getUpstreamAttachmentPoint();
		// move away from the parent by half size:
		pos -= glm::rotate(getChildAttachmentPoint(PI), initialData_->angle);
		return pos;
	} else
		return initialData_->position;
}*/

void BodyPart::computeBodyPhysProps() {
	// do the magic here and update initialData_->cachedProps from other initialData_ fields
	// initialData_->cachedProps must be in world space
	// parent's initialData_->cachedProps are assumed to be in world space at this time
	PhysicsProperties parentProps = parent_ ? parent_->initialData_->cachedProps : PhysicsProperties();
	initialData_->cachedProps.velocity = parentProps.velocity;
	// compute parent space position:
	glm::vec2 pos = getUpstreamAttachmentPoint()
			- glm::rotate(getChildAttachmentPoint(PI), (float)initialData_->attachmentDirectionParent);
#warning "must take into account lateral offset"
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	initialData_->cachedProps.position = pos;
	// compute world space angle:
	initialData_->cachedProps.angle = parentProps.angle + initialData_->attachmentDirectionParent + initialData_->angleOffset;
}

glm::vec3 BodyPart::getWorldTransformation() const {
	if (body_ == nullptr) {
		glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
		glm::vec2 pos = getUpstreamAttachmentPoint()
					- glm::rotate(getChildAttachmentPoint(PI), (float)initialData_->attachmentDirectionParent);
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				initialData_->attachmentDirectionParent + initialData_->angleOffset);
	} else {
		return glm::vec3(b2g(body_->GetPosition()), body_->GetAngle());
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

void BodyPart::registerAttribute(gene_attribute_type type, CummulativeValue& value) {
	mapAttributes_[type] = &value;
}

CummulativeValue* BodyPart::getAttribute(gene_attribute_type attrib) {
	return mapAttributes_[attrib];
}
