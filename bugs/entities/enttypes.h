/*
 * enttypes.h
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#ifndef ENTITIES_ENTTYPES_H_
#define ENTITIES_ENTTYPES_H_

namespace EntityType {	// bitfield
	enum Values {
		BUG					= 1,
		GAMETE				= 2,
		WALL				= 4,
		FOOD_DISPENSER		= 8,
		FOOD_CHUNK			= 16
	};
}

#endif /* ENTITIES_ENTTYPES_H_ */
