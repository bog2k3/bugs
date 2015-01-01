/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include "../../World.h"
#include "../../math/box2glm.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../log.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);

JointInitializationData::JointInitializationData()
	: phiMin(-PI/8), phiMax(PI * 0.9f) {
	size = 0.2e-4f;
}

Joint::Joint(BodyPart* parent)
	: BodyPart(parent, BODY_PART_JOINT, std::make_shared<JointInitializationData>())
	, jointInitialData_(std::static_pointer_cast<JointInitializationData>(getInitializationData()))
	, physJoint_(nullptr)
	, repauseAngle_(0)
{
	std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
	registerAttribute(GENE_ATTRIB_JOINT_LOW_LIMIT, initData->phiMin);
	registerAttribute(GENE_ATTRIB_JOINT_HIGH_LIMIT, initData->phiMax);
}

Joint::~Joint() {
	// delete joint
}

/**
 * if the angles are screwed, limit them to 0
 */
void Joint::fixAngles() {
	std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
	if (initData->phiMin > 0)
		initData->phiMin = 0;
	if (initData->phiMax < 0)
		initData->phiMax = 0;
}

void Joint::commit() {
	assert(!committed_);
	assert(nChildren_ == 1);

	std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();

	fixAngles();

	b2RevoluteJointDef def;
	def.Initialize(parent_->getBody(), children_[0]->getBody(), g2b(initData->cachedProps.position));
	def.enableLimit = true;
	def.lowerAngle = initData->phiMin;
	def.upperAngle = initData->phiMax;
	def.userData = (void*)this;
	// def.collideConnected = true;

	physJoint_ = (b2RevoluteJoint*)World::getInstance()->getPhysics()->CreateJoint(&def);

	repauseAngle_ = initData->angleOffset;
}

glm::vec3 Joint::getWorldTransformation() const {
	if (!committed_)
		return BodyPart::getWorldTransformation();
	else {
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f,
			physJoint_->GetBodyA()->GetAngle() + physJoint_->GetJointAngle());
	}
}

void Joint::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		/*initialData_->position = getFinalPrecommitPosition();
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos, pos+glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
		*/
	}
}

glm::vec2 Joint::getChildAttachmentPoint(float relativeAngle) const
{
	assert(!committed_);
	return glm::rotate(glm::vec2(sqrtf(getInitializationData()->size * PI_INV), 0), relativeAngle);
}

float Joint::getTotalRange() {
	if (committed_) {
		//return phiMax_ - phiMin_;
		// get from the actual physic Joint
	} else {
		std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
		// save original values:
		float fm = initData->phiMin, fM = initData->phiMax;
		// fix the angles:
		fixAngles();
		// compute range:
		float ret = initData->phiMax - initData->phiMin;
		// restore original values:
		initData->phiMax = fM;
		initData->phiMin = fm;

		return ret;
	}
}
