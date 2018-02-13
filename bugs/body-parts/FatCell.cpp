/*
 * FatCell.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: bog
 */

#include "FatCell.h"
#include "BodyConst.h"
#include "BodyCell.h"

#include <boglfw/World.h>

#include <Box2D/Box2D.h>

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

void FatCell::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf((size_)/PI);

	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}
