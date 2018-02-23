/*
 * ProteinHyperspace.h
 *
 *  Created on: Dec 7, 2017
 *      Author: bog
 */

#ifndef GENETICS_PROTEINHYPERSPACE_H_
#define GENETICS_PROTEINHYPERSPACE_H_

#include "../body-parts/BodyPartTypes.h"

static constexpr BodyPartType proteinHyperspace[2][2][2][2] = {
	/* W- */ {
		/* Z- */ {
			/* Y- */ {
				/* X- */ BodyPartType::FAT, /* X+ */ BodyPartType::EGGLAYER
			},
			/* Y+ */ {
				/* X- */ BodyPartType::MOUTH, /* X+ */ BodyPartType::FAT
			},
		},
		/* Z+ */ {
			/* Y- */ {
				/* X- */ BodyPartType::SENSOR_PROXIMITY, /* X+ */ BodyPartType::BONE
			},
			/* Y+ */ {
				/* X- */ BodyPartType::BONE, /* X+ */ BodyPartType::GRIPPER
			},
		},
	},
	/* W+ */ {
		/* Z- */ {
			/* Y- */ {
				/* X- */ BodyPartType::SENSOR_SIGHT, /* X+ */ BodyPartType::GRIPPER
			},
			/* Y+ */ {
				/* X- */ BodyPartType::GRIPPER, /* X+ */ BodyPartType::BONE
			},
		},
		/* Z+ */ {
			/* Y- */ {
				/* X- */ BodyPartType::BONE, /* X+ */ BodyPartType::SENSOR_COMPASS
			},
			/* Y+ */ {
				/* X- */ BodyPartType::EGGLAYER, /* X+ */ BodyPartType::SENSOR_SIGHT
			},
		},
	}
};

#endif /* GENETICS_PROTEINHYPERSPACE_H_ */
