/*
 * BodyCell.cpp
 *
 *  Created on: Dec 27, 2017
 *      Author: bog
 */

#include "BodyCell.h"
#include "BodyConst.h"
#include "../genetics/constants.h"
#include "../genetics/Gene.h"

#include <boglfw/math/math3D.h>

void BodyCell::initializeGeneValues() {
	mapDivisionParams_[GENE_DIVISION_RATIO] = CumulativeValue(1.f);			// default to 50%-50%
	mapDivisionParams_[GENE_DIVISION_SEPARATE] = CumulativeValue(constants::FBOOL_false);	// default to sticky
	mapDivisionParams_[GENE_DIVISION_AFFINITY] = CumulativeValue();
	mapDivisionParams_[GENE_DIVISION_ANGLE] = CumulativeValue();
	mapDivisionParams_[GENE_DIVISION_MIRROR] = CumulativeValue();
	mapDivisionParams_[GENE_DIVISION_REORIENT] = CumulativeValue();
	mapDivisionParams_[GENE_DIVISION_BOND_BIAS] = CumulativeValue();

	mapJointAttribs_[GENE_JOINT_ATTR_HIGH_LIMIT] = CumulativeValue(BodyConst::initialJointMaxPhi);
	mapJointAttribs_[GENE_JOINT_ATTR_LOW_LIMIT] = CumulativeValue(BodyConst::initialJointMinPhi);
	mapJointAttribs_[GENE_JOINT_ATTR_RESET_TORQUE] = CumulativeValue(BodyConst::initialJointResetTorque);
	mapJointAttribs_[GENE_JOINT_ATTR_MASS_RATIO] = CumulativeValue(BodyConst::initialJointMassRatio);
	mapJointAttribs_[GENE_JOINT_ATTR_DENSITY] = CumulativeValue(BodyConst::initialJointDensity);
	mapJointAttribs_[GENE_JOINT_ATTR_TYPE] = CumulativeValue();		// default to weld joint (0)

	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO] = CumulativeValue(BodyConst::initialMuscleMassRatio);
	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_INSERT_OFFSET1] = CumulativeValue(BodyConst::initialMuscleInsertOffset);
	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_INSERT_OFFSET2] = CumulativeValue(BodyConst::initialMuscleInsertOffset);

	mapRightMuscleAttribs_ = mapLeftMuscleAttribs_;

	for (auto i=GENE_ATTRIB_INVALID+1; i<GENE_ATTRIB_END; i++)
		mapAttributes_[i] = CumulativeValue();
}

float BodyCell::radius(float angle) const {
	if (radiusFn)
		return radiusFn(*this, angle);
	else
		return Cell::radius(angle);
}

Cell* BodyCell::createChild(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide) const {
	auto branch = branch_;
	branch.push_back(rightSide ? 'R' : 'L');
	return new BodyCell(size, position, rotation, mirror, rightSide, branch);
}

std::pair<BodyCell*, BodyCell*> BodyCell::divide() {
	float angle = mapDivisionParams_[GENE_DIVISION_ANGLE];
	float ratio = mapDivisionParams_[GENE_DIVISION_RATIO].clamp(
			BodyConst::minDivisionRatio,
			1.f / BodyConst::minDivisionRatio);
	bool reorient = mapDivisionParams_[GENE_DIVISION_REORIENT] > 0.f;
	bool mirror = mapDivisionParams_[GENE_DIVISION_MIRROR] > 0.f;
	float bondBias = tanh(mapDivisionParams_[GENE_DIVISION_BOND_BIAS].get() / 2.f);
	bool noBond = mapDivisionParams_[GENE_DIVISION_SEPARATE] > 0.f;
	auto p = Cell::divide(angle, ratio, bondBias, reorient, mirror, noBond);
	auto left = static_cast<BodyCell*>(p.first);
	auto right = static_cast<BodyCell*>(p.second);
	return {left, right};
}

void BodyCell::updateRotation() {
	angle_ = initialAngle_ + mapAttributes_[GENE_ATTRIB_LOCAL_ROTATION] * (isMirrored() ? -1 : 1);
	updateBonds();
}

void BodyCell::transform(glm::mat4 const& m, float rot) {
	glm::vec4 pos {position_, 0, 1};
	pos = m * pos;
	position_ = {pos.x, pos.y};
	angle_ += rot;
}
