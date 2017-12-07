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
				/* X- */ BodyPartType::BONE, /* X+ */ BodyPartType::INVALID
			},
			/* Y+ */ {
				/* X- */ BodyPartType::GRIPPER, /* X+ */ BodyPartType::MOUTH
			},
		},
		/* Z+ */ {
			/* Y- */ {
				/* X- */ BodyPartType::INVALID, /* X+ */ BodyPartType::INVALID
			},
			/* Y+ */ {
				/* X- */ BodyPartType::MUSCLE, /* X+ */ BodyPartType::EGGLAYER
			},
		},
	},
	/* W+ */ {
		/* Z- */ {
			/* Y- */ {
				/* X- */ BodyPartType::INVALID, /* X+ */ BodyPartType::INVALID
			},
			/* Y+ */ {
				/* X- */ BodyPartType::SENSOR_PROXIMITY, /* X+ */ BodyPartType::INVALID
			},
		},
		/* Z+ */ {
			/* Y- */ {
				/* X- */ BodyPartType::INVALID, /* X+ */ BodyPartType::INVALID
			},
			/* Y+ */ {
				/* X- */ BodyPartType::SENSOR_COMPASS, /* X+ */ BodyPartType::SENSOR_SIGHT
			},
		},
	}
};

#endif /* GENETICS_PROTEINHYPERSPACE_H_ */
