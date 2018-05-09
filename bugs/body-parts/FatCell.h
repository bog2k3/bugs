/*
 * FatCell.h
 *
 *  Created on: Jan 3, 2018
 *      Author: bog
 */

#ifndef BODY_PARTS_FATCELL_H_
#define BODY_PARTS_FATCELL_H_

#include "BodyPart.h"

#include <glm/vec2.hpp>

class FatCell: public BodyPart {
public:
	FatCell(BodyPartContext const& context, BodyCell& cell);
	virtual ~FatCell();

	void draw(RenderContext const& ctx) override;

	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	void updateFixtures() override;

	// returns the actual energy consumed (less than requested if running low)
	float consumeEnergy(float amount);
	void replenishFromMass(float mass);

	static float getDensity(BodyCell const& cell);
	static float getRadius(BodyCell const& cell, float angle);

private:
	float frameUsedEnergy_;
	float energyBuffer_;
	float maxEnergyBuffer_;
	float fixtureSizeInv_;
};

#endif /* BODY_PARTS_FATCELL_H_ */
