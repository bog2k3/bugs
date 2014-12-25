/*
 * GeneDefinitions.h
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENEDEFINITIONS_H_
#define GENETICS_GENEDEFINITIONS_H_

enum gene_development_command {
	GENE_DEV_INVALID = 0,
	GENE_DEV_GROW,			// grows a new body part in a specific direction
	GENE_DEV_SPLIT,			// split the up-closest joint and duplicate all of its descendants
	GENE_DEV_END
};

enum gene_part_type {
	GENE_PART_INVALID = 0,
	GENE_PART_BONE,			// bone
	GENE_PART_MUSCLE,		// muscle
	GENE_PART_SENSOR,		// sensor - one of the sensor types
	GENE_PART_GRIPPER,		// gripper - used to move around, like a foot
	GENE_PART_END
};

enum gene_sensor_type {
	GENE_SENSOR_INVALID = 0,
	GENE_SENSOR_EYE,		// eye sight
	GENE_SENSOR_SMELL,		// smell - proximity to certain materials
	GENE_SENSOR_END
};

enum gene_attribute_type {
	GENE_ATTRIB_INVALID = 0,
	GENE_ATTRIB_SIZE,			// represents the surface area of the part
	GENE_ATTRIB_ASPECT_RATIO,	// aspect_ratio = length / width
	GENE_ATTRIB_DENSITY,		// density - for bones
	GENE_ATTRIB_PARENT_OFFSET,	// sideways offset from parent joint, in % of current's part's width (default=0%)

	GENE_ATTRIB_END
};

#endif /* GENETICS_GENEDEFINITIONS_H_ */
