/*
 * BodyConst.h
 *
 *  Created on: Jan 7, 2015
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BODYCONST_H_
#define OBJECTS_BODY_PARTS_BODYCONST_H_

#include "../../math/constants.h"

class BodyConst {
public:
	// optimization values:
	static constexpr float SizeThresholdToCommit			= 1.1f;			// [*]
	static constexpr float SizeThresholdToCommit_inv = 1.f / SizeThresholdToCommit;

	// fixed values:
	static constexpr float ZygoteDensity					= 1.e+3f;		// [kg/m^2]
	static constexpr float GripperDensity					= 0.7e+3f;		// [kg/m^2]
	static constexpr float MuscleDensity					= 1.3e+3f;		// [kg/m^2]
	static constexpr float MuscleContractionRatio 			= 0.5f;			// [*]
	static constexpr float MuscleForcePerWidthRatio 		= 100;			// [N/m] the theoretical force of a muscle 1 meter wide.
	static constexpr float MuscleMaxLinearContractionSpeed 	= 0.8f;			// [m/s] max meters/second linear contraction speed
	static constexpr float FatDensity						= 0.8e+3f;		// [kg/m^2]
	static constexpr float FatEnergyDensity					= 1.e+3f;		// [J/kg]
	static constexpr float NeuronSize						= 1.e-6f;		// [m^2]

	// default initial values for cummulative properties:
	static constexpr float initialBodyPartSize				= 1.e-4f;		// [m^2]
	static constexpr float initialBodyPartDensity			= 1.e+3f;		// [kg/m^2]
	static constexpr float initialBoneDensity				= 1.1e+3f;		// [kg/m^2]
	static constexpr float initialBoneAspectRatio			= 0.7f;			// [*]
	static constexpr float initialJointMinPhi				= -PI/8;		// [rad]
	static constexpr float initialJointMaxPhi				= PI*0.9f;		// [rad]
	static constexpr float initialJointResetTorque			= 0.5e-4f;		// [Nm]
	static constexpr float initialMuscleAspectRatio			= 2.0f;			// [*]
	static constexpr float initialTorsoDensity				= 1.e+3f;		// [kg/m^2]
};



#endif /* OBJECTS_BODY_PARTS_BODYCONST_H_ */
