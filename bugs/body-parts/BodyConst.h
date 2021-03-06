/*
 * BodyConst.h
 *
 *  Created on: Jan 7, 2015
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BODYCONST_H_
#define OBJECTS_BODY_PARTS_BODYCONST_H_

#include "../math/constants.h"

class BodyConst {
public:
	// optimization values:
	static constexpr float SizeThresholdToCommit			= 1.1f;			// [*]
	static constexpr float SizeThresholdToCommit_inv = 1.f / SizeThresholdToCommit;

	// fixed values:
	static constexpr float MinBodyPartSize					= 1.e-4f;		// [m^2]
	static constexpr float MinBodyPartDensity				= 0.5f;			// [kg/m^2]
	static constexpr float MaxBodyPartDensity				= 100.f;		// [kg/m^2]
	static constexpr float MaxBodyPartAspectRatio			= 100.f;		// [*]
	static constexpr float MaxBodyPartAspectRatioInv		= 1.f / MaxBodyPartAspectRatio;	// [*]
	static constexpr float MinEggMass						= 1.e-3;		// [kg]
	static constexpr float MaxEggMass						= 10;			// [kg]
	static constexpr float MaxEggEjectSpeed					= 100;			// [m/s]
	static constexpr float ZygoteDensity					= 10.f;			// [kg/m^2]
	static constexpr float ZygoteDensityInv					= 1.f / ZygoteDensity; // [m^2/kg]
	static constexpr float GripperDensity					= 7.f;			// [kg/m^2]
	static constexpr float MuscleDensity					= 13.f;			// [kg/m^2]
	static constexpr float TorsoDensity						= 7.f;			// [kg/m^2]
	static constexpr float TorsoEnergyDensity				= 100.f;		// [J/m^2] how much ready-to-use energy the torso can store
	static constexpr float MuscleContractionRatio 			= 0.5f;			// [*]
	static constexpr float MuscleForcePerWidthRatio 		= 100;			// [N/m] the theoretical force of a muscle 1 meter wide.
	static constexpr float MuscleMaxLinearContractionSpeed 	= 0.6f;			// [m/s] max meters/second linear contraction speed
	static constexpr float FatDensity						= 8.f;			// [kg/m^2]
	static constexpr float FatDensityInv					= 1.f/FatDensity;	// [m^2/kg]
	static constexpr float FatEnergyDensity					= 0.5e+4f;		// [J/kg]
	static constexpr float FatEnergyDensityInv				= 1.f / FatEnergyDensity;	// [kg/J]
	static constexpr float NeuronSize						= 1.e-6f;		// [m^2]
	static constexpr float MuscleEnergyConstant				= 1.0f;			// [J/(N*s)] how many Joules uses a muscle with F=1N for 1 sec?
	static constexpr float MouthDensity						= 8.f;			// [kg/m^2]
	static constexpr float NoseDensity						= 3.f;			// [kg/m^2]
	static constexpr float MouthBufferDensity				= 10.f;			// [kg/m^2] how much food (kg) can fit into a unit size mouth?
	static constexpr float FoodProcessingSpeedDensity		= 50.e-2f;		// [kg/(m^2*s)] how much food can a unit size torso process in a second?
	static constexpr float JointForceToleranceFactor		= 4.e+5f;		// [N/m^2] how much force a joint can take, relative to its size
	static constexpr float JointTorqueToleranceFactor		= 3.5e+2f;		// [Nm/m^2] how much torque a joint can take, relative to its size
	static constexpr float JointDensity						= 9.f;			// [kg/m^2]
	static constexpr float SensorSizeScalingConstant		= 0.5e+4f;		// [*]
	static constexpr float SensorNoiseThreshConstant		= 6.5e+4f;		// [*]	bigger makes lower noise threshold

	// default initial values for cummulative properties:
	static constexpr float initialBodyPartSize				= 1.e-4f;		// [m^2]
	static constexpr float initialBodyPartDensity			= 10.f;			// [kg/m^2]
	static constexpr float initialBoneDensity				= 11.f;			// [kg/m^2]
	static constexpr float initialBoneAspectRatio			= 0.7f;			// [*]  length/width
	static constexpr float initialJointSize					= 1.9e-4f;		// [m^2]
	static constexpr float initialJointMinPhi				= -PI/8;		// [rad]
	static constexpr float initialJointMaxPhi				= PI*0.6f;		// [rad]
	static constexpr float initialJointResetTorque			= 0.6e-3f;		// [Nm]
	static constexpr float initialMuscleAspectRatio			= 2.0f;			// [*]  length/width
	static constexpr float initialMouthSize					= 0.5e-3f;		// [m^2]
	static constexpr float initialMouthAspectRatio			= 0.15f;		// [*] length/width
	static constexpr float initialNoseSize					= 0.5e-4f;		// [m^2]

	// default values for whole-body attributes:
	static constexpr float initialFatMassRatio				= 0.3f;			// [*] fraction of total mass that is fat
	static constexpr float initialMinFatMassRatio			= 0.15f;		// [*] fat percentage of total body weight. below this fraction, growth is halted
	static constexpr float initialAdultLeanMass				= 6.f;			// [kg]
	static constexpr float initialGrowthSpeed				= 20.e-3f;		// [kg/s] how fast lean mass can be added to the body
	static constexpr float initialEggMass					= 0.5f;			// [kg] mass of zygote
	static constexpr float initialEggEjectSpeed				= 0.6f;			// [m/s]
	static constexpr float initialReproductiveMassRatio		= 0.35f;		// [*] fraction of growth mass that is invested in creating eggs

	static constexpr float MaxVMSCoordinateValue			= 1000.f;		// Virtual Matching Space span
};



#endif /* OBJECTS_BODY_PARTS_BODYCONST_H_ */
