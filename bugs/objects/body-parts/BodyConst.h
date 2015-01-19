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
	static constexpr float ZygoteDensity					= 10.f;			// [kg/m^2]
	static constexpr float GripperDensity					= 7.f;			// [kg/m^2]
	static constexpr float MuscleDensity					= 13.f;			// [kg/m^2]
	static constexpr float TorsoDensity						= 7.f;			// [kg/m^2]
	static constexpr float MuscleContractionRatio 			= 0.5f;			// [*]
	static constexpr float MuscleForcePerWidthRatio 		= 100;			// [N/m] the theoretical force of a muscle 1 meter wide.
	static constexpr float MuscleMaxLinearContractionSpeed 	= 0.8f;			// [m/s] max meters/second linear contraction speed
	static constexpr float FatDensity						= 8.f;			// [kg/m^2]
	static constexpr float FatDensityInv					= 1.f/FatDensity;	// [m^2/kg]
	static constexpr float FatEnergyDensity					= 0.5e+4f;		// [J/kg]
	static constexpr float FatEnergyDensityInv				= 1.f / FatEnergyDensity;	// [kg/J]
	static constexpr float NeuronSize						= 1.e-6f;		// [m^2]
	static constexpr float MuscleEnergyConstant				= 10.0f;			// [J/(N*s)] how many Joules uses a muscle with F=1N for 1 sec?
	static constexpr float MouthAspectRatio					= 0.15f;		// [*] length/width

	// default initial values for cummulative properties:
	static constexpr float initialBodyPartSize				= 1.e-4f;		// [m^2]
	static constexpr float initialBodyPartDensity			= 10.f;			// [kg/m^2]
	static constexpr float initialBoneDensity				= 11.f;			// [kg/m^2]
	static constexpr float initialBoneAspectRatio			= 0.7f;			// [*]  length/width
	static constexpr float initialJointMinPhi				= -PI/8;		// [rad]
	static constexpr float initialJointMaxPhi				= PI*0.9f;		// [rad]
	static constexpr float initialJointResetTorque			= 0.6e-3f;		// [Nm]
	static constexpr float initialMuscleAspectRatio			= 2.0f;			// [*]  length/width
	static constexpr float initialJointSize					= 0.8e-4f;		// [m^2]

	// default values for whole-body attributes:
	static constexpr float initialFatMassRatio				= 0.3f;			// [*]
	static constexpr float initialMinFatMassRatio			= 0.1f;			// [*] below this fraction, growth is halted
	static constexpr float initialAdultLeanMass				= 6.f;			// [kg]
};



#endif /* OBJECTS_BODY_PARTS_BODYCONST_H_ */
