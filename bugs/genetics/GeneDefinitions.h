/*
 *
 * GeneDefinitions.h
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENEDEFINITIONS_H_
#define GENETICS_GENEDEFINITIONS_H_

#include <cstdint>

// ----------------------------------- gene_type -------------------------------//

typedef uint8_t gene_type;

constexpr gene_type GENE_TYPE_INVALID = 0;
constexpr gene_type GENE_TYPE_NO_OP = 1;			// no operation gene; dummy.
constexpr gene_type GENE_TYPE_STOP = 2;				// signals the ribosome to stop reading genes after this one
constexpr gene_type GENE_TYPE_SKIP = 3;				// control gene -> skip next N genes if all conditions are met
constexpr gene_type GENE_TYPE_DEVELOPMENT = 4;		// developmental gene - commands the growth of the body
constexpr gene_type GENE_TYPE_PART_ATTRIBUTE = 5;	// body part attribute - establishes characteristics of certain body parts
constexpr gene_type GENE_TYPE_BODY_ATTRIBUTE = 6;	// body attribute - controls specific whole-body attributes that do not belong to a specific part,
													// such as metabolic parameters
constexpr gene_type GENE_TYPE_SYNAPSE = 7;			// creates or alters a synapse between neurons (cummulative weight)
constexpr gene_type GENE_TYPE_FEEDBACK_SYNAPSE = 8;	// creates or alters a feedback synapse (from motor command neuron to other neuron - cummulative)
constexpr gene_type GENE_TYPE_TRANSFER = 9;			// controls the transfer function of a neuron (cummulative)
constexpr gene_type GENE_TYPE_NEURAL_CONST = 10;		// neural constant (cummulative) - used in various computations
constexpr gene_type GENE_TYPE_END = 11;

// ----------------------------------- gene_development_command -------------------------------//

typedef uint8_t gene_development_command;

constexpr gene_development_command GENE_DEV_INVALID = 0;
constexpr gene_development_command GENE_DEV_GROW = 1;			// grows a new body part in a specific direction
constexpr gene_development_command GENE_DEV_SPLIT = 2;			// split the up-closest joint and duplicate all of its descendants
constexpr gene_development_command GENE_DEV_END = 3;

// ----------------------------------- gene_part_type -------------------------------//

typedef uint8_t gene_part_type;

constexpr gene_part_type GENE_PART_INVALID = 0;
constexpr gene_part_type GENE_PART_MOUTH = 1;
constexpr gene_part_type GENE_PART_BONE = 2;		// bone
constexpr gene_part_type GENE_PART_GRIPPER = 3;		// gripper - used to move around, like a foot
constexpr gene_part_type GENE_PART_SENSOR = 4;		// sensor - one of the sensor types
constexpr gene_part_type GENE_PART_EGGLAYER = 5;	// sexual organ - used to grown and lay gamettes
constexpr gene_part_type GENE_PART_END = 6;

// ----------------------------------- gene_sensor_type -------------------------------//

typedef uint8_t gene_sensor_type;

constexpr gene_sensor_type GENE_SENSOR_INVALID = 0;
constexpr gene_sensor_type GENE_SENSOR_EYE = 1;			// eye sight
constexpr gene_sensor_type GENE_SENSOR_SMELL = 2;		// smell - proximity to certain materials
constexpr gene_sensor_type GENE_SENSOR_JOINT_ANGLE = 3;
constexpr gene_sensor_type GENE_SENSOR_GRIPPER_STATE = 4;
constexpr gene_sensor_type GENE_SENSOR_END = 5;

// ----------------------------------- gene_part_attribute_type -------------------------------//

typedef uint8_t gene_part_attribute_type;

constexpr gene_part_attribute_type GENE_ATTRIB_INVALID = 0;
constexpr gene_part_attribute_type GENE_ATTRIB_LOCAL_ROTATION = 1;		// rotates the part around its own center
constexpr gene_part_attribute_type GENE_ATTRIB_ATTACHMENT_OFFSET = 2;	// sideways offset from parent joint, in % of current's part's width (default=0%)
constexpr gene_part_attribute_type GENE_ATTRIB_SIZE = 3;				// represents the surface area of the part
constexpr gene_part_attribute_type GENE_ATTRIB_ASPECT_RATIO = 4;		// aspect_ratio = length / width
constexpr gene_part_attribute_type GENE_ATTRIB_DENSITY = 5;				// density - for bones
constexpr gene_part_attribute_type GENE_ATTRIB_JOINT_LOW_LIMIT = 6;		// low angle limit for joint
constexpr gene_part_attribute_type GENE_ATTRIB_JOINT_HIGH_LIMIT = 7;	// high angle limit for joint
constexpr gene_part_attribute_type GENE_ATTRIB_JOINT_RESET_TORQUE = 8;	// torque that moves the joint back into rest position when no forces act on it
constexpr gene_part_attribute_type GENE_ATTRIB_EGG_EJECT_SPEED = 9;		// speed with which eggs are ejected from egg-layers
constexpr gene_part_attribute_type GENE_ATTRIB_END = 10;

// ----------------------------------- gene_body_attribute_type -------------------------------//

typedef uint8_t gene_body_attribute_type;

constexpr gene_body_attribute_type GENE_BODY_ATTRIB_INVALID = 0;

// fraction of zygote mass that will be transformed into fat during development, to act as the initial energy supply of the bug
constexpr gene_body_attribute_type GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO = 1;

// minimum fat to body mass ratio that enables growth and reproduction. If fat mass ratio falls below
// this threshold, growth is stalled until the balance is restored. Also, an egg will not be generated if the
// remaining fat ratio falls below this threshold.
constexpr gene_body_attribute_type GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO = 2;

// target lean body mass (excluding fat) of adult body. The body grows until it reaches this target, then growth stops.
constexpr gene_body_attribute_type GENE_BODY_ATTRIB_ADULT_LEAN_MASS = 3;
constexpr gene_body_attribute_type GENE_BODY_ATTRIB_GROWTH_SPEED = 4;				// [kg/s] speed at which growth happens
constexpr gene_body_attribute_type GENE_BODY_ATTRIB_EGG_MASS = 5;					// [kg] how big eggs should be?
constexpr gene_body_attribute_type GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO = 6;	// [*] what fraction of growth food goes into creating eggs?

constexpr gene_body_attribute_type GENE_BODY_ATTRIB_END = 7;

#endif /* GENETICS_GENEDEFINITIONS_H_ */
