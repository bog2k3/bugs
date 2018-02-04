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

	mapJointAttribs_[GENE_JOINT_ATTR_HIGH_LIMIT] = CumulativeValue(BodyConst::initialJointMaxPhi);
	mapJointAttribs_[GENE_JOINT_ATTR_LOW_LIMIT] = CumulativeValue(BodyConst::initialJointMinPhi);
	mapJointAttribs_[GENE_JOINT_ATTR_RESET_TORQUE] = CumulativeValue(BodyConst::initialJointResetTorque);
	mapJointAttribs_[GENE_JOINT_ATTR_MASS_RATIO] = CumulativeValue(BodyConst::initialJointMassRatio);
	mapJointAttribs_[GENE_JOINT_ATTR_DENSITY] = CumulativeValue(BodyConst::initialJointDensity);
	mapJointAttribs_[GENE_JOINT_ATTR_TYPE] = CumulativeValue();		// default to weld joint (0)

	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_ASPECT_RATIO] = CumulativeValue(BodyConst::initialMuscleAspectRatio);
	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO] = CumulativeValue(BodyConst::initialMuscleMassRatio);
	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_INSERT_OFFSET] = CumulativeValue(BodyConst::initialMuscleInsertOffset);
	mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_INPUT_COORD] = CumulativeValue();

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
	float angle = limitAngle(mapDivisionParams_[GENE_DIVISION_ANGLE].get(), PI);
	float ratio = mapDivisionParams_[GENE_DIVISION_RATIO].clamp(
			BodyConst::minDivisionRatio,
			1.f / BodyConst::minDivisionRatio);
	bool reorient = mapDivisionParams_[GENE_DIVISION_REORIENT] > 0.f;
	bool mirror = mapDivisionParams_[GENE_DIVISION_MIRROR] > 0.f;
	auto p = Cell::divide(angle, ratio, reorient, mirror);
	return {static_cast<BodyCell*>(p.first), static_cast<BodyCell*>(p.second)};
}
