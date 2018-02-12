/*
 * FatCell.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: bog
 */

#include "FatCell.h"
#include "BodyConst.h"
#include "BodyCell.h"

#include <glm/gtx/rotate_vector.hpp>

FatCell::~FatCell() {
	// TODO Auto-generated destructor stub
}

FatCell::FatCell(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::FAT, context, cell)
{
	// TODO Auto-generated constructor stub

}

float FatCell::getDensity(BodyCell const& cell) {
	return BodyConst::FatDensity;
}

static glm::vec2 getFatCellAttachmentPoint(float size, float angle) {
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

glm::vec2 FatCell::getAttachmentPoint(float relativeAngle) {
	return getFatCellAttachmentPoint(size_, relativeAngle);
}

float FatCell::getRadius(BodyCell const& cell, float angle) {
	glm::vec2 p = getFatCellAttachmentPoint(cell.size(), angle);
	return glm::length(p);
}
