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

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <map>
#include <vector>

class Neuron;

class BodyCell: public Cell {
public:

	BodyCell(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide, std::vector<char> branch = {})
		: Cell(size, position, rotation, mirror, rightSide)
		, initialAngle_(rotation)
	{
		branch_.swap(branch);
		initializeGeneValues();
	}

	// returns the radius in a given local angle (relative to cell's orientation)
	// that is distance from center to cell's outline - it may be non-uniform for non-circle cells
	float radius(float angle) const override;

	void initializeGeneValues();

	float density() const { return density_; }
	float muscleMass(bool right) const { return right ? muscleMassRight_ : muscleMassLeft_; }
	float jointMass() const { return jointMass_; }

	void updateRotation();

	std::pair<BodyCell*, BodyCell*> divide();

	void transform(glm::mat4 const& m, float rot);

	std::map<gene_joint_attribute_type, CumulativeValue> mapJointAttribs_;
	std::map<gene_muscle_attribute_type, CumulativeValue> mapLeftMuscleAttribs_;
	std::map<gene_muscle_attribute_type, CumulativeValue> mapRightMuscleAttribs_;
	std::map<gene_part_attribute_type, CumulativeValue> mapAttributes_;

#ifdef DEBUG
	bool matchBranch(const char* branchCode) {
		uint i=0;
		for (; i<branch_.size(); i++) {
			if (branch_[i] != branchCode[i])
				return false;
		}
		return branchCode[i] == 0;
	}

	std::string getBranchString() const {
		return std::string(branch_.begin(), branch_.end());
	}
#endif

protected:
	Cell* createChild(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide) const override;

private:
	friend class BodyPart;
	friend class Ribosome;

	std::vector<char> branch_;
	float density_ = BodyConst::ZygoteDensity;
	float (*radiusFn)(BodyCell const& cell, float angle) = nullptr;

	float initialAngle_;
	glm::vec4 proteinValues_ {0}; // hyper-space position for current cell
	std::map<gene_division_param_type, CumulativeValue> mapDivisionParams_;

	float muscleMassLeft_ = 0;
	float muscleMassRight_ = 0;
	float jointMass_ = 0;
};

#endif /* BODY_PARTS_BODYCELL_H_ */
