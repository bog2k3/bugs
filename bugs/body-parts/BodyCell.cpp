/*
 * BodyCell.cpp
 *
 *  Created on: Dec 27, 2017
 *      Author: bog
 */

#include "BodyCell.h"
#include "BodyConst.h"
#include "../genetics/constants.h"

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
	mapJointAttribs_[GENE_JOINT_ATTR_TYPE] = CumulativeValue(constants::FBOOL_false);		// default to weld joint

	mapMuscleAttribs_[GENE_MUSCLE_ATTR_ASPECT_RATIO] = CumulativeValue(BodyConst::initialMuscleAspectRatio);
	mapMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO] = CumulativeValue(BodyConst::initialMuscleMassRatio);
	mapMuscleAttribs_[GENE_MUSCLE_ATTR_INSERT_OFFSET] = CumulativeValue(BodyConst::initialMuscleInsertOffset);
	mapMuscleAttribs_[GENE_MUSCLE_ATTR_INPUT_COORD] = CumulativeValue();

	for (auto i=GENE_ATTRIB_INVALID+1; i<GENE_ATTRIB_END; i++)
		mapAttributes_[i] = CumulativeValue();
}

float BodyCell::radius(float angle) const {
	// TODO cell radius - use map to call static methods to get attachment points ?
	throw std::runtime_error("Implement this!");
}

Cell* BodyCell::createChild(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide) const {
	throw std::runtime_error("implement!"); // also compute child's branch to pass on ctor
}
