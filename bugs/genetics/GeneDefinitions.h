/*
 *
 * GeneDefinitions.h
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENEDEFINITIONS_H_
#define GENETICS_GENEDEFINITIONS_H_

#include <cstdint>

#define ENABLE_START_MARKER_GENES

// ----------------------------------- gene_type -------------------------------//

typedef uint8_t gene_type;

constexpr gene_type GENE_TYPE_INVALID = 0;
constexpr gene_type GENE_TYPE_NO_OP = 1;			// no operation gene; dummy.
#ifdef ENABLE_START_MARKER_GENES
constexpr gene_type GENE_TYPE_START_MARKER = 2;		// marks the beginning of a separate sequence of genes
#endif
constexpr gene_type GENE_TYPE_STOP = 3;				// signals the ribosome to stop reading genes after this one
constexpr gene_type GENE_TYPE_SKIP = 4;				// control gene -> skip next N genes if all conditions are met
constexpr gene_type GENE_TYPE_PROTEIN = 5;			// protein gene -> produces a specific protein type in a body-part's segment
constexpr gene_type GENE_TYPE_OFFSET = 6;			// controls the relative genome offset of a child part
constexpr gene_type GENE_TYPE_JOINT_OFFSET = 7;		// controls the relative genome offset of this part's upstream Joint relative to this one
constexpr gene_type GENE_TYPE_PART_ATTRIBUTE = 8;	// body part attribute - establishes characteristics of certain body parts
constexpr gene_type GENE_TYPE_BODY_ATTRIBUTE = 9;	// body attribute - controls specific whole-body attributes that do not belong to a specific part,
													// such as metabolic parameters
constexpr gene_type GENE_TYPE_SYNAPSE = 10;			// creates or alters a synapse between neurons (cummulative weight)
constexpr gene_type GENE_TYPE_TRANSFER_FUNC = 11;			// controls the transfer function of a neuron (cummulative)
constexpr gene_type GENE_TYPE_NEURAL_BIAS = 12;		// neural bias (cummulative) - is added to the weighted sum of the inputs
constexpr gene_type GENE_TYPE_NEURON_OUTPUT_COORD = 13;		// output coord (in MVMS) from a neuron
constexpr gene_type GENE_TYPE_NEURON_INPUT_COORD = 14;		// input coord (in SVMS) to a neuron
constexpr gene_type GENE_TYPE_NEURAL_PARAM = 15;	// neural parameter - used by some types of neurons for specific purposes

constexpr gene_type GENE_TYPE_END = 15;

// ----------------------------------- gene_protein_type -------------------------------//

// proteins move the target point in potential body-part hyper-space along different axes.
// The position of the target point relative to each axis determines what kind of body part a given segment will grow
// There are 4 axes (X, Y, Z, W) of the 4-dimensional hyper-space which is split into 16 distinct regions joining at the origin.
// Each of these regions (they are the equivalent of quadrants in a 2-dimensional plane) correspond to a body part type or to nothing.

typedef uint8_t gene_protein_type;

constexpr gene_protein_type GENE_PROT_NONE = 0;
constexpr gene_protein_type GENE_PROT_A = 2;		// X-
constexpr gene_protein_type GENE_PROT_B = 3;		// X+
constexpr gene_protein_type GENE_PROT_C = 4;		// Y-
constexpr gene_protein_type GENE_PROT_D = 5;		// Y+
constexpr gene_protein_type GENE_PROT_E = 6;		// Z-
constexpr gene_protein_type GENE_PROT_F = 7;		// Z+
constexpr gene_protein_type GENE_PROT_G = 8;		// W-
constexpr gene_protein_type GENE_PROT_H = 9;		// W+
constexpr gene_protein_type GENE_PROT_END = 10;

// ----------------------------------- gene_part_attribute_type -------------------------------//

typedef uint8_t gene_part_attribute_type;

constexpr gene_part_attribute_type GENE_ATTRIB_INVALID = 0;
constexpr gene_part_attribute_type GENE_ATTRIB_LOCAL_ROTATION = 1;		// rotates the part around its own center
constexpr gene_part_attribute_type GENE_ATTRIB_ATTACHMENT_OFFSET = 2;	// sideways offset from parent joint, in % of current's part's width (default=0%)
constexpr gene_part_attribute_type GENE_ATTRIB_SIZE = 3;				// represents the surface area of the part
constexpr gene_part_attribute_type GENE_ATTRIB_ASPECT_RATIO = 4;		// aspect_ratio = length / width
constexpr gene_part_attribute_type GENE_ATTRIB_DENSITY = 5;				// density - for bones
constexpr gene_part_attribute_type GENE_ATTRIB_JOINT_LOW_LIMIT = 6;		// low angle limit for up-stream joint
constexpr gene_part_attribute_type GENE_ATTRIB_JOINT_HIGH_LIMIT = 7;	// high angle limit for up-stream joint
constexpr gene_part_attribute_type GENE_ATTRIB_JOINT_RESET_TORQUE = 8;	// torque that moves the up-stream joint back into rest position when no forces act on it
constexpr gene_part_attribute_type GENE_ATTRIB_EGG_EJECT_SPEED = 9;		// speed with which eggs are ejected from egg-layers
constexpr gene_part_attribute_type GENE_ATTRIB_MOTOR_INPUT_COORD = 10;	// input coord in VMS; uses attrib index
constexpr gene_part_attribute_type GENE_ATTRIB_SENSOR_OUTPUT_COORD = 11;	// output coord in VMS; uses attrib index
constexpr gene_part_attribute_type GENE_ATTRIB_END = 12;

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
