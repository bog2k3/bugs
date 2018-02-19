/*
 * BodyPartTypes.h
 *
 *  Created on: Dec 7, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_BODYPARTTYPES_H_
#define BODY_PARTS_BODYPARTTYPES_H_

#include <ostream>

enum class BodyPartType {
	INVALID = 0,

	FAT,					// is created from unspecialized cells
	BONE,
	JOINT_PIVOT,
	JOINT_WELD,
	MUSCLE,
	GRIPPER,
	ZYGOTE_SHELL,
	SENSOR_PROXIMITY,		// many outputs for multiple types of entities
	SENSOR_COMPASS,			// 1 output - absolute orientation in world
	SENSOR_SIGHT,			// array of outputs for pixels (multiple channels maybe?)
	MOUTH,
	EGGLAYER,
};

inline std::ostream& operator << (std::ostream& str, BodyPartType const& type) {
	return str << (unsigned)type;
}


#endif /* BODY_PARTS_BODYPARTTYPES_H_ */
