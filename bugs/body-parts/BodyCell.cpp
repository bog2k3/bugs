/*
 * BodyCell.cpp
 *
 *  Created on: Dec 27, 2017
 *      Author: bog
 */

#include "BodyCell.h"

void BodyCell::initializeGeneValues() {
	mapDivisionParams_[GENE_DIVISION_RATIO].reset(1.f);
	mapDivisionParams_[GENE_DIVISION_SEPARATE].reset(-0.5f);

	mapJointAttribs_[GENE_JOINT_ATTR_HIGH_LIMIT].reset(BodyConst::initialJointMaxPhi);
	mapJointAttribs_[GENE_JOINT_ATTR_LOW_LIMIT].reset(BodyConst::initialJointMinPhi);
	mapJointAttribs_[GENE_JOINT_ATTR_RESET_TORQUE].reset(BodyConst::initialJointResetTorque);
	mapJointAttribs_[GENE_JOINT_ATTR_SIZE].reset(BodyConst::initialJointSize);

	mapMuscleAttribs_[GENE_MUSCLE_ATTR_ASPECT_RATIO].reset(BodyConst::initialMuscleAspectRatio);
	mapMuscleAttribs_[GENE_MUSCLE_ATTR_SIZE].reset(BodyConst::initialMuscleSize);
	mapMuscleAttribs_[GENE_MUSCLE_ATTR_INSERT_OFFSET].reset(BodyConst::initialMuscleInsertOffset);
}
