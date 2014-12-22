/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "../../math/box2glm.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../math/math2D.h"
#include "../../log.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <assert.h>

BodyPart::BodyPart(BodyPart* parent, PART_TYPE type, PhysicsProperties props)
	: WorldObject(props)
	, type_(type)
	, parent_(parent)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
	, coordinates_local_(true)
{
	if (parent) {
		parent->add(this);
	}
}

BodyPart::~BodyPart() {
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
	else
		return parent_->getChildAttachmentPoint(initialData_->angle);
}

void BodyPart::transform_position_and_angle() {
	if (parent_) {
		glm::vec3 wt = getWorldTransformation();
		initialData_->angle = wt.z;
		initialData_->position = vec3xy(wt);
	}
	coordinates_local_ = false;
}

void BodyPart::commit_tree() {
	assert(!committed_);
	LOGGER("commit_tree");
	std::vector<BodyPart*> joints;
	commit_tree(joints);
	// joints must be committed last, after all other parts are committed
	// because they must use the body of both parent and child when creating the actual joint
	for (auto j : joints) {
		j->commit();
		j->WorldObject::purgeInitializationData();
		j->committed_ = true;
	}
}

void BodyPart::commit_tree(std::vector<BodyPart*> &out_joints) {
	if (parent_) {
		// update attachment point (since parent may have changed its size or aspect ratio):
		initialData_->position = getUpstreamAttachmentPoint();
		// move away from the parent by half size:
		initialData_->position -= glm::rotate(getChildAttachmentPoint(PI), initialData_->angle);
	}
	// transform position and angle into world space:
	transform_position_and_angle();
	// perform commit on local node:
	if (type_ == BODY_PART_JOINT) {
		out_joints.push_back(this);
	} else {
		WorldObject::createPhysicsBody();
		commit();
		WorldObject::purgeInitializationData();
		committed_ = true;
	}
	// perform recursive commit on all children:
	for (int i=0; i<nChildren_; i++)
		children_[i]->commit_tree(out_joints);
}

glm::vec3 BodyPart::getWorldTransformation() const {
	if (!committed_) {
		if (coordinates_local_) {
			glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
			return parentTransform + glm::vec3(glm::rotate(initialData_->position, parentTransform.z), initialData_->angle);
		} else {
			return glm::vec3(initialData_->position, initialData_->angle);
		}
	} else {
		return glm::vec3(b2g(body_->GetPosition()), body_->GetAngle());
	}
}

void BodyPart::draw(ObjectRenderContext* ctx) {
	if (committed_)
		return;
	glm::vec3 trans = getWorldTransformation();
	glm::vec2 pos(trans.x, trans.y);
	ctx->shape->drawLine(pos + glm::vec2(-0.01f, 0), pos + glm::vec2(0.01f, 0), 0, glm::vec3(0.2f, 0.2f, 1.f));
	ctx->shape->drawLine(pos + glm::vec2(0, -0.01f), pos + glm::vec2(0, 0.01f), 0, glm::vec3(1.f, 0.2f, 0.2f));
}

void BodyPart::registerAttribute(gene_attribute_type type, CummulativeValue& value) {
	mapAttributes_[type] = &value;
}

CummulativeValue* BodyPart::getAttribute(gene_attribute_type attrib) {
	return mapAttributes_[attrib];
}
