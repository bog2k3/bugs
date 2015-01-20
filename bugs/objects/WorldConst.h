/*
 * WorldConst.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_WORLDCONST_H_
#define OBJECTS_WORLDCONST_H_

class WorldConst {
public:
	static constexpr float FoodChunkDensity						= 7.f;		// [kg/m^2]
	static constexpr float FoodChunkDensityInv					= 1.f/FoodChunkDensity;		// [m^2/kg]
	static constexpr float BasicFoodDispenserPeriod				= 5.f;		// [s]
};

#endif /* OBJECTS_WORLDCONST_H_ */
