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
#include "../genetics/CummulativeValue.h"

#include <map>
#include <vector>

class BodyCell: public Cell {
public:

	BodyCell(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide, std::vector<char> const& branch = {})
		: Cell(size, position, rotation, mirror, rightSide) {
		branch_ = branch;
		branch_.push_back(rightSide ? 'R' : 'L');

		initializeGeneValues();
	}

	// TODO as the decoding goes and density genes pop up, the cell must change it's size to reflect the new density
	// TODO when cell splits, part of its mass must be used for joint and muscles (if pivot joint)

	void initializeGeneValues();

private:
	friend class BodyPart;

	std::vector<char> branch_;

	std::map<gene_protein_type, CummulativeValue> mapProteins_;
	std::map<gene_division_param_type, CummulativeValue> mapDivisionParams_;
	std::map<gene_joint_attribute, CummulativeValue> mapJointAttribs_;
	std::map<gene_muscle_attribute, CummulativeValue> mapMuscleAttribs_;
	std::map<gene_part_attribute_type, std::vector<CummulativeValue>> mapAttributes_;
};

#endif /* BODY_PARTS_BODYCELL_H_ */
