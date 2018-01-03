/*
 * BodyCell.h
 *
 *  Created on: Dec 27, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_BODYCELL_H_
#define BODY_PARTS_BODYCELL_H_

#include "Cell.h"
#include "../genetics/GeneDefinitions.h"
#include "../genetics/CumulativeValue.h"
#include "BodyConst.h"

#include <map>
#include <vector>

class BodyCell: public Cell {
public:

	BodyCell(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide, std::vector<char> const& branch = {})
		: Cell(size, position, rotation, mirror, rightSide)
	{
		branch_ = branch;
		branch_.push_back(rightSide ? 'R' : 'L');

		initializeGeneValues();
	}

	// returns the radius in a given local angle (relative to cell's orientation)
	// that is distance from center to cell's outline - it may be non-uniform for non-circle cells
	float radius(float angle) const override;

	// TODO when cell splits, part of its mass must be used for joint and muscles (if pivot joint)
	// TODO at the end of decoding, accumulated rotation attribute gene value must be used to alter the rotation of the cell
	// 		effectively the cell's orientation should become
	//			angle_ + mapAttributes_[GENE_ATTRIB_LOCAL_ROTATION]
	//		this means also altering all links relative angles in order to keep them in the same physical positions

	void initializeGeneValues();

	float density() const { return density_; }

private:
	friend class BodyPart;

	std::vector<char> branch_;
	float density_ = BodyConst::FatDensity;

	glm::vec4 proteinValues_ {0}; // hyper-space position for current cell
	std::map<gene_division_param_type, CumulativeValue> mapDivisionParams_;
	std::map<gene_joint_attribute, CumulativeValue> mapJointAttribs_;
	std::map<gene_muscle_attribute, CumulativeValue> mapMuscleAttribs_;
	std::map<gene_part_attribute_type, CumulativeValue> mapAttributes_;
};

#endif /* BODY_PARTS_BODYCELL_H_ */
