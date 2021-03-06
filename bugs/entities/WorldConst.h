/*
 * WorldConst.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_WORLDCONST_H_
#define OBJECTS_WORLDCONST_H_

#include "../math/constants.h"
#include <glm/vec2.hpp>

class WorldConst {
public:
	static constexpr float FoodChunkDensity						= 7.f;				// [kg/m^2]
	static constexpr float FoodChunkDensityInv					= 1.f/FoodChunkDensity;	// [m^2/kg]
	static constexpr float FoodChunkDecaySpeed					= 1.5e-3f;			// [kg/s]  how much mass it loses in a second
	static constexpr float FoodChunkSensorRatio					= 1.5f;				// [*]
	static constexpr float FoodDispenserPeriod					= 2.f;				// [s]
	static constexpr float FoodDispenserSize					= PI*0.625e-1f;		// [m^2]
	static constexpr float FoodDispenserSpawnVelocity			= 0.3f;				// [m/s]
	static constexpr float FoodDispenserSpawnMass				= 60.e-3f;			// [kg]
	static constexpr float FoodDispenserSpreadAngleHalf			= PI;				// [rad]

	static constexpr float GameteAttractRadius					= 10;				// [m]
	static constexpr float GameteAttractForceFactor				= 30;				// force between 2 1kg gametes [N]
	static constexpr unsigned MaxGenomeLengthDifference			= 10;				// max length difference that is still compatible

	static constexpr float BodyDecaySpeed						= 2.e-3f;			// [kg/s] speed at which mass is lost from dead bodies
};

#endif /* OBJECTS_WORLDCONST_H_ */
