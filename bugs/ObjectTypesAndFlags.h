/*
 * ObjectTypesAndFlags.h
 *
 *  Created on: Jan 22, 2015
 *      Author: bog
 */

#ifndef OBJECTTYPESANDFLAGS_H_
#define OBJECTTYPESANDFLAGS_H_

#include <cstdint>

// all the objects that have a PhysicsBody should have an entry here:
enum class ObjectTypes : uint32_t {
	UNDEFINED		= 0,
	BPART_BONE,
	BPART_GRIPPER,
	BPART_MOUTH,
	BPART_TORSO,
	BPART_ZYGOTE,
	FOOD_CHUNK,
	FOOD_DISPENSER,
};

struct CategoryFlags {
	typedef uint32_t type;

	static constexpr type BODYPART					= 1<< 0;
	static constexpr type FOOD						= 1<< 1;
	static constexpr type STATIC					= 1<< 2;


	static constexpr type ALL						= 0xFFFFFFFF;
};

#endif /* OBJECTTYPESANDFLAGS_H_ */
