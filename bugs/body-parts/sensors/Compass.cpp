/*
 * Compass.cpp
 *
 *  Created on: Jul 19, 2018
 *      Author: bog
 */

#include "Compass.h"
#include "../BodyConst.h"
#include "../BodyCell.h"
#include "../../ObjectTypesAndFlags.h"

#include <boglfw/utils/UpdateList.h>

#include <glm/gtx/rotate_vector.hpp>

Compass::Compass(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::SENSOR_COMPASS, context, cell)
{
	physBody_.userObjectType_ = ObjectTypes::BPART_COMPASS;
	physBody_.userPointer_ = this;

	outputVMSCoord_ = cell.vmsOffset() + cell.mapAttributes_[GENE_ATTRIB_VMS_COORD1].clamp(-BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);

	context_.updateList.add(this);

	onDied.add([this](BodyPart*) {
		context_.updateList.remove(this);
	});
}

Compass::~Compass() {
	// TODO Auto-generated destructor stub
}

static glm::vec2 getCompassAttachmentPoint(float size, float angle) {
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

glm::vec2 Compass::getAttachmentPoint(float relativeAngle) {
	return getCompassAttachmentPoint(size_, relativeAngle);
}

float Compass::getDensity(BodyCell const& cell) {
	return BodyConst::CompassDensity;
}

float Compass::getRadius(BodyCell const& cell, float angle) {
	glm::vec2 p = getCompassAttachmentPoint(cell.size(), angle);
	return glm::length(p);
}

void Compass::draw(Viewport* vp) {
	// TODO
}

void Compass::update(float dt) {
	// TODO
}

void Compass::updateFixtures() {
	// TODO
}
