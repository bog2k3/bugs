/*
 *
 * GeneDefinitions.h
 *  Created on: Dec 3, 2014
 *	  Author: bog
 */

#ifndef GENETICS_GENEDEFINITIONS_H_
#define GENETICS_GENEDEFINITIONS_H_

#include <cstdint>

// ----------------------------------- gene_type -------------------------------//

enum class gene_type : uint8_t {

	INVALID = 0,
	NO_OP = 1,					// no operation gene; dummy.
	STOP = 2,					// signals the ribosome to stop reading genes after this one
	SKIP = 3,					// control gene -> skip next N genes if all conditions are met
	PROTEIN = 4,				// protein gene -> produces a specific protein type in the cell
	OFFSET = 5,					// controls the relative genome offset of a child cell (left or right)
	DIVISION_PARAM = 6,			// controls the parameters of division in this cell like affinity, ratio, angle etc
	JOINT_ATTRIBUTE = 7,		// controls attributes of the joint that would be created between children of this cell during division
	MUSCLE_ATTRIBUTE = 8,		// controls attributes of the muscles (left/right) around the joint created by division
	PART_ATTRIBUTE = 9,		// body part attribute - establishes characteristics of certain body parts
	BODY_ATTRIBUTE = 10,		// body attribute - controls specific whole-body attributes that do not belong to a specific part,
									// such as metabolic parameters
	VMS_OFFSET = 11,			// offsets all VMS coordinates specified in the cell (inputs and outputs, neuron positions)
	NEURON = 12,				// creates a new neuron at the specified location (neuron will belong the the current cell)
	SYNAPSE = 13,				// creates or alters a synapse between a VMS input coordinate and a VMS output coordinate (cummulative weight)
	TRANSFER_FUNC = 14,			// controls the transfer function of a neuron (cummulative)
	NEURAL_BIAS = 15,			// neural bias (cummulative) - is added to the weighted sum of the inputs
	NEURON_OUTPUT_COORD = 16,	// output coord (in VMS) from a neuron - where the axon lies
	NEURON_INPUT_COORD = 17,	// input coord (in VMS) to a neuron - where the dendrites lie
	NEURAL_PARAM = 18,			// neural parameter - used by some types of neurons for specific purposes

	END = 19
};

// ----------------------------------- gene_protein_type -------------------------------//

// proteins move the target point in potential body-part hyper-space along different axes.
// The position of the target point relative to each axis determines what kind of body part a given cell will specialize into
// There are 4 axes (X, Y, Z, W) of the 4-dimensional hyper-space which is split into 16 distinct regions joining at the origin.
// Each of these regions (they are the equivalent of quadrants in a 2-dimensional plane) correspond to a body part type or to nothing.

typedef uint8_t gene_protein_type;

constexpr gene_protein_type GENE_PROT_INVALID = 0;
constexpr gene_protein_type GENE_PROT_X = 1;
constexpr gene_protein_type GENE_PROT_Y = 2;
constexpr gene_protein_type GENE_PROT_Z = 3;
constexpr gene_protein_type GENE_PROT_W = 4;
constexpr gene_protein_type GENE_PROT_END = 5;

// ----------------------------------- gene_division_param_type -------------------------------//

typedef uint8_t gene_division_param_type;

constexpr gene_division_param_type GENE_DIVISION_INVALID = 0;
constexpr gene_division_param_type GENE_DIVISION_AFFINITY = 1;			// if >0 cell will divide
constexpr gene_division_param_type GENE_DIVISION_ANGLE = 2;				// relative (to cell orientation) division angle
constexpr gene_division_param_type GENE_DIVISION_RATIO = 3;				// division ratio (left / right); absolute value is used
constexpr gene_division_param_type GENE_DIVISION_MIRROR = 4;			// if >0 right side will be mirrored
constexpr gene_division_param_type GENE_DIVISION_REORIENT = 5;			// if >0 children will be reoriented by the division axis
constexpr gene_division_param_type GENE_DIVISION_SEPARATE = 6;			// if >0 children will be separated (not connected by a joint), otherwise a joint is created
constexpr gene_division_param_type GENE_DIVISION_BOND_BIAS = 7;			// >0 biases bonds to the left, <0 to the right (tanh value maps to [-1,_1])
constexpr gene_division_param_type GENE_DIVISION_END = 8;

// ----------------------------------- gene_joint_attribute_type_type -------------------------------//

typedef uint8_t gene_joint_attribute_type;

constexpr gene_joint_attribute_type GENE_JOINT_ATTR_INVALID = 0;
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_TYPE = 1;				// if >0 type is pivot joint, else weld joint
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_LOW_LIMIT = 2;			// low angle limit for joint
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_HIGH_LIMIT = 3;			// high angle limit for joint
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_RESET_TORQUE = 4;		// torque that moves the joint back into rest position when no forces act on it
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_MASS_RATIO = 5;			// proportion of parent cell mass that goes into the joint
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_DENSITY = 6;				// [kg/m^2] denser joints take more mass to make, but are stronger
constexpr gene_joint_attribute_type GENE_JOINT_ATTR_END = 7;

// ----------------------------------- gene_muscle_attribute_type_type -------------------------------//

typedef uint8_t gene_muscle_attribute_type;

constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_INVALID = 0;
constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_MASS_RATIO = 1;		// proportion of parent cell mass that goes into making the muscle
//constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_ASPECT_RATIO = 2;		// length / width
constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_INPUT_COORD = 2;		// VMS input coordinate for muscle
constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_INSERT_OFFSET1 = 3;		// lateral insertion offset from joint in radians (left of joint) - clamped to [0, PI/2]
constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_INSERT_OFFSET2 = 4;		// lateral insertion offset from joint in radians (right of joint) - clamped to [0, PI/2]
constexpr gene_muscle_attribute_type GENE_MUSCLE_ATTR_END = 5;


// ----------------------------------- gene_part_attribute_type -------------------------------//

typedef uint8_t gene_part_attribute_type;

constexpr gene_part_attribute_type GENE_ATTRIB_INVALID = 0;
constexpr gene_part_attribute_type GENE_ATTRIB_LOCAL_ROTATION = 1;		// rotates the part around its own center
constexpr gene_part_attribute_type GENE_ATTRIB_ASPECT_RATIO = 2;		// aspect_ratio = length / width
constexpr gene_part_attribute_type GENE_ATTRIB_GENERIC1 = 3;			// generic attribute - meaning depends on specific type of body part
constexpr gene_part_attribute_type GENE_ATTRIB_GENERIC2 = 4;			// generic attribute - meaning depends on specific type of body part
constexpr gene_part_attribute_type GENE_ATTRIB_VMS_COORD1 = 5;			// input/output (depends on motor/sensor) coord in VMS;
constexpr gene_part_attribute_type GENE_ATTRIB_VMS_COORD2 = 6;			// input/output (depends on motor/sensor) coord in VMS;
constexpr gene_part_attribute_type GENE_ATTRIB_VMS_COORD3 = 7;			// input/output (depends on motor/sensor) coord in VMS;
constexpr gene_part_attribute_type GENE_ATTRIB_VMS_COORD4 = 8;			// input/output (depends on motor/sensor) coord in VMS;
constexpr gene_part_attribute_type GENE_ATTRIB_VMS_COORD6 = 9;			// input/output (depends on motor/sensor) coord in VMS;
constexpr gene_part_attribute_type GENE_ATTRIB_END = 10;

// ----------------------------------- gene_body_attribute_type -------------------------------//

typedef uint8_t gene_body_attribute_type;

constexpr gene_body_attribute_type GENE_BODY_ATTRIB_INVALID = 0;

// fraction of zygote mass that will be transformed into fat during development, to act as the initial energy supply of the bug
//constexpr gene_body_attribute_type GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO = 1;	// OBSOLETE - fat ratio is implicitly determined by unspecialized cells

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
