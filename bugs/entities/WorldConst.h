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
	static constexpr float FoodChunkLifeTime					= 10.f;				// [s]
	static constexpr float FoodDispenserPeriod					= 5.f;				// [s]
	static constexpr float FoodDispenserSize					= PI*0.625e-1f;		// [m^2]
	static constexpr float FoodDispenserSpawnVelocity			= 0.2f;				// [m/s]
	static constexpr float FoodDispenserSpawnMass				= 10.e-3f;			// [kg]
	static constexpr float FoodDispenserSpreadAngleHalf			= PI;				// [rad]
};

#endif /* OBJECTS_WORLDCONST_H_ */
