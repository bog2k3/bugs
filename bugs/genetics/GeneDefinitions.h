/*
 *
 * GeneDefinitions.h
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENEDEFINITIONS_H_
#define GENETICS_GENEDEFINITIONS_H_

enum gene_type {
	GENE_TYPE_INVALID = 0,
	GENE_TYPE_STOP,				// signals the ribosome to stop reading genes after this one
	GENE_TYPE_DEVELOPMENT,		// developmental gene - commands the growth of the body
	GENE_TYPE_PART_ATTRIBUTE,	// body part attribute - establishes characteristics of certain body parts
	GENE_TYPE_BODY_ATTRIBUTE,	// body attribute - controls specific whole-body attributes that do not belong to a specific part,
								// such as metabolic parameters
	GENE_TYPE_SYNAPSE,			// creates or alters a synapse between neurons (cummulative weight)
	GENE_TYPE_FEEDBACK_SYNAPSE,	// creates or alters a feedback synapse (from motor command neuron to other neuron - cummulative)
	GENE_TYPE_TRANSFER,			// controls the transfer function of a neuron (cummulative)
	GENE_TYPE_NEURAL_CONST,		// neural constant (cummulative) - used in various computations

	GENE_TYPE_END
};

enum gene_development_command {
	GENE_DEV_INVALID = 0,
	GENE_DEV_GROW,			// grows a new body part in a specific direction
	GENE_DEV_SPLIT,			// split the up-closest joint and duplicate all of its descendants
	GENE_DEV_END
};

enum gene_part_type {
	GENE_PART_INVALID = 0,
	GENE_PART_BONE,			// bone
	GENE_PART_GRIPPER,		// gripper - used to move around, like a foot
	GENE_PART_SENSOR,		// sensor - one of the sensor types
	GENE_PART_END
};

enum gene_sensor_type {
	GENE_SENSOR_INVALID = 0,
	GENE_SENSOR_EYE,		// eye sight
	GENE_SENSOR_SMELL,		// smell - proximity to certain materials
	GENE_SENSOR_JOINT_ANGLE,
	GENE_SENSOR_GRIPPER_STATE,
	GENE_SENSOR_END
};

enum gene_part_attribute_type {
	GENE_ATTRIB_INVALID = 0,
	GENE_ATTRIB_ATTACHMENT_ANGLE,	// modifies the original growth angle
	GENE_ATTRIB_LOCAL_ROTATION,		// rotates the part around its own center
	GENE_ATTRIB_ATTACHMENT_OFFSET,	// sideways offset from parent joint, in % of current's part's width (default=0%)
	GENE_ATTRIB_SIZE,			// represents the surface area of the part
	GENE_ATTRIB_ASPECT_RATIO,	// aspect_ratio = length / width
	GENE_ATTRIB_DENSITY,		// density - for bones
	GENE_ATTRIB_JOINT_LOW_LIMIT,	// low angle limit for joint
	GENE_ATTRIB_JOINT_HIGH_LIMIT,	// high angle limit for joint
	GENE_ATTRIB_JOINT_RESET_TORQUE,	// torque that moves the joint back into rest position when no forces act on it

	GENE_ATTRIB_END
};

enum gene_body_attribute_type {
	GENE_BODY_ATTRIB_INVALID = 0,
	GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO,	// fraction of zygote mass that will be transformed into fat
												// during development, to act as the initial energy supply of the bug
	GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO,		// minimum fat to body mass ratio that enables growth and reproduction.
												// If fat mass ratio falls below this threshold, growth is stalled until
												// the balance is restored. Also, an egg will not be generated if
												// the remaining fat ratio falls below this threshold.
	GENE_BODY_ATTRIB_ADULT_LEAN_MASS,			// target lean body mass (excluding fat) of adult body. The body grows
												// until it reaches this target, then growth stops.
	GENE_BODY_ATTRIB_GROWTH_SPEED,				// [kg/s] speed at which growth happens
	GENE_BODY_ATTRIB_EGG_MASS,					// [kg] how big eggs should be?
	GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO	// [*] what fraction of growth food goes into creating eggs?
};

#endif /* GENETICS_GENEDEFINITIONS_H_ */
