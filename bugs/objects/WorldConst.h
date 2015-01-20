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
	static constexpr float BasicFoodDispenserPeriod				= 5.f;				// [s]
	static constexpr float BasicFoodDispenserSize				= PI*0.625e-1f;		// [m^2]
	static constexpr float BasicFoodDispenserSpawnPositionX 	= 0.3f;				// [m]
	static constexpr float BasicFoodDispenserSpawnPositionY 	= 0.0f;				// [m]
	static constexpr float BasicFoodDispenserSpawnVelocity		= 0.5f;				// [m/s]
	static constexpr float BasicFoodDispenserSpawnMass			= 10.e-3f;			// [kg]
};

#endif /* OBJECTS_WORLDCONST_H_ */
