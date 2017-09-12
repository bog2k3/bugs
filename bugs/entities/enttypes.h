/*
 * enttypes.h
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#ifndef ENTITIES_ENTTYPES_H_
#define ENTITIES_ENTTYPES_H_

enum class EntityType {	// bitfield
	BUG					= 0x01,
	GAMETE				= 0x02,
	WALL				= 0x04,
	FOOD_DISPENSER		= 0x08,
	FOOD_CHUNK			= 0x10,
	LABEL				= 0x20,

	CAMERA_CTRL			= 0x40,
	PATH_CONTROLLER		= 0x80,
	BOX					= 0x100,
	SIGNAL_VIEWER		= 0x200,

	ALL					= 0xFFFF,
};

#endif /* ENTITIES_ENTTYPES_H_ */

