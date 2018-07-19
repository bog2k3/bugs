/*
 * Eye.cpp
 *
 *  Created on: Jul 19, 2018
 *      Author: bog
 */

#include "Eye.h"
#include "../BodyConst.h"
#include "../BodyCell.h"
#include "../../ObjectTypesAndFlags.h"

#include <boglfw/utils/UpdateList.h>

#include <glm/gtx/rotate_vector.hpp>

Eye::Eye(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::SENSOR_SIGHT, context, cell)
{
	physBody_.userObjectType_ = ObjectTypes::BPART_EYE;
	physBody_.userPointer_ = this;

	outputVMSCoord_ = cell.vmsOffset() + cell.mapAttributes_[GENE_ATTRIB_VMS_COORD1].clamp(-BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);

	context_.updateList.add(this);

	onDied.add([this](BodyPart*) {
		context_.updateList.remove(this);
	});
}

Eye::~Eye() {
	// TODO Auto-generated destructor stub
}

static glm::vec2 getEyeAttachmentPoint(float size, float angle) {
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

glm::vec2 Eye::getAttachmentPoint(float relativeAngle) {
	return getEyeAttachmentPoint(size_, relativeAngle);
}

float Eye::getDensity(BodyCell const& cell) {
	return BodyConst::EyeDensity;
}

float Eye::getRadius(BodyCell const& cell, float angle) {
	glm::vec2 p = getEyeAttachmentPoint(cell.size(), angle);
	return glm::length(p);
}

void Eye::draw(Viewport* vp) {
	// TODO
}

void Eye::update(float dt) {
	// TODO
}

void Eye::updateFixtures() {
	// TODO
}
