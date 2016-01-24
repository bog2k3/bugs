/*
 * enttypes.h
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#ifndef ENTITIES_ENTTYPES_H_
#define ENTITIES_ENTTYPES_H_

typedef unsigned EntityType;

static constexpr EntityType ENTITY_BUG					= 1;
static constexpr EntityType ENTITY_GAMETE				= 2;
static constexpr EntityType ENTITY_WALL					= 3;
static constexpr EntityType ENTITY_FOOD_DISPENSER		= 4;
static constexpr EntityType ENTITY_FOOD_CHUNK			= 5;

#endif /* ENTITIES_ENTTYPES_H_ */
