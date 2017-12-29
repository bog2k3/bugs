/*
 * ObjectTypesAndFlags.h
 *
 *  Created on: Jan 22, 2015
 *      Author: bog
 */

#ifndef OBJECTTYPESANDFLAGS_H_
#define OBJECTTYPESANDFLAGS_H_

#include <Box2D/Dynamics/b2Fixture.h>
#include <cstdint>

// all the objects that have a PhysicsBody should have an entry here:
namespace ObjectTypes {
enum values : uint32_t {
	UNDEFINED		= 0,
	BPART_BONE,
	BPART_GRIPPER,
	BPART_MOUTH,
	BPART_FATCELL,
	BPART_ZYGOTE,
	BPART_EGGLAYER,
	BPART_NOSE,
	FOOD_CHUNK,
	FOOD_DISPENSER,
	WALL,
	GAMETE,
	PROTOTYPE,
};}

/*
 * Use these values to tell PhysicsBody whether to generate collision events or not
 * (physical collisions will occur nevertheless)
 */
struct EventCategoryFlags {
	typedef uint32_t type;

	static constexpr type BODYPART					= 1<< 0;
	static constexpr type FOOD						= 1<< 1;
	static constexpr type STATIC					= 1<< 2;
	static constexpr type GAMETE					= 1<< 3;


	static constexpr type ALL						= 0xFFFFFFFF;
};

/*
 * use these categories to include or exclude physical collision between types of objects
 */
struct b2FilterCategory {
	static constexpr decltype(::b2Filter::categoryBits) NONE			= 0;
	static constexpr decltype(::b2Filter::categoryBits) DEFAULT			= 1;
};

/*
 * groups are used to make objects of the same index never collide (negative) or always collide (>=0).
 * different groups are handled by filter categories above.
 */
struct b2FilterGroup {
	static constexpr decltype(::b2Filter::groupIndex) NONE				= 0;
	static constexpr decltype(::b2Filter::groupIndex) FOOD_CHUNK		= -1;	// food chunks don't collide with each other
};

#endif /* OBJECTTYPESANDFLAGS_H_ */
