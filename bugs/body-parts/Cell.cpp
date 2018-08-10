/*
 * Cell.cpp
 *
 *  Created on: Nov 7, 2017
 *      Author: bog
 */

#include "Cell.h"

#include <boglfw/math/math3D.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <glm/vec2.hpp>

#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

Cell::Cell(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide)
	: angle_(limitAngle(rotation, 2*PI))
	, position_(position)
	, size_(size), mirror_(mirror), rightSide_(rightSide)
{
}

float Cell::radius(float angle) const {
	return sqrtf(size_ / PI);
}

// computes a world angle from a cell-relative angle, taking into account mirroring and cell orientation
float Cell::wangle(float angle) {
	return angle_ + angle * (mirror_ ? -1 : 1);
}

// computes a relative angle (to cell orientation) from a world angle, taking into account mirroring and cell orientation
float Cell::rangle(float angle) {
	return angleDiff(angle_, angle) * (mirror_ ? -1 : 1);
}

void Cell::bond(Cell* other, bool isRightSide, float jointDiameter, Cell* jointParent) {
	float angle = rangle(limitAngle(pointDirection(other->position_ - position_), 2*PI));
	neighbours_.push_back({angle, jointDiameter, isRightSide, other, jointParent});
	float oAngle = other->rangle(limitAngle(pointDirection(position_ - other->position_), 2*PI));
	other->neighbours_.push_back({oAngle, jointDiameter, !isRightSide, this, jointParent});
}

std::set<Cell*> Cell::fixOverlap(std::set<Cell*> &marked, bool extraPrecision) {
	// 1. push all overlapping cells away until they touch on the edges
	// 2. pull all bonded cells inward until they touch on the edges
	// 3. mark all cells that have been moved and their neighbors
	// 4. repeat until no more marked cells

	std::set<Cell*> newMarked;
	std::set<Cell*> allAffected;
	std::map<Cell*, float> massRatios;	// this will hold mass ratios to adjust the apparent mass of each cell in order to control how much we push them
										// cells which will have a lot of forces act on them but the resultant force is small, will have their mass ratio high
										// in order to force the other cells to move around instead.
	const float MaxMassRatio = extraPrecision ? 10.f : 20.f;
	const float RM = MaxMassRatio;
	const float r2 = sqrt(2.f);
	const float a = -(2.f*r2)*(1.f - 1.f/(r2*RM) - (1-1.f/r2)*RM);
	const float b = 1.f/RM - RM - a;
	const float c = RM;
	auto massRatioFn = [a,b,c, MaxMassRatio](float r) {		// 2nd order polynomial function - see "making-of/20 mass ratio function.jpg" for details
		assert(r >= 0.f && r <= 1.01f);
		r = clamp(r, 0.f, 1.f);
		float y = a*r*r + b*r + c;
		assert(y >= 0.95f/MaxMassRatio && y <= 1.05*MaxMassRatio);
		return y;
	};

	const int maxIterations = extraPrecision ? 500 : 20;
	int nIterations = 0;
	while (!marked.empty() && nIterations < maxIterations) {
		std::set<std::pair<Cell*, Cell*>> affectedBonds;
		std::map<Cell*, glm::vec2> totalCellOffset;	// holds vector sums
		std::map<Cell*, float> totalCellOffsetMod;	// holds sums of vector moduli
		// step 1: compute cell displacements to fix overlap and gaps
		for (Cell* c : marked) {
			for (auto &n : c->neighbours_) {
				if (affectedBonds.find({c, n.other}) != affectedBonds.end())
					continue;	// we already treated this bond this round
				affectedBonds.insert({n.other, c});

				// common constants and variables -------
				const float toleranceFactor = extraPrecision ? 0.02f : 0.1f; // proportion of the smaller neighbor's radius
				constexpr float maxDisplacementRatio = 2.f; // ratio of displacement to radius
				// adjust for apparent mass alteration:
				float mr1 = massRatios[c]; if (mr1 == 0) mr1 = 1.f;
				float mr2 = massRatios[n.other]; if (mr2 == 0) mr2 = 1.f;
				float cmass = c->size_ * mr1;
				float omass = n.other->size_ * mr2;
				float ratio = cmass / (cmass + omass);	// light cells move more than heavy ones
				bool affectedBond = false;

				// center-push method ----------------
				auto diff = n.other->position_ - c->position_;
				float angle = pointDirection(diff);
				float cr = c->radius(c->rangle(angle));
				float nr = n.other->radius(n.other->rangle(angle+PI));
				auto dist = glm::length(diff);
				float overlap = cr + nr + n.offset - dist;
				if (abs(overlap) > toleranceFactor * min(cr, nr)) {
					glm::vec2 offset = glm::normalize(diff) * overlap;
					// compute displacements
					auto thisOffs = -offset * clamp(1-ratio, 0.f, cr*maxDisplacementRatio);
					auto otherOffs = offset * clamp(ratio, 0.f, nr*maxDisplacementRatio);

					totalCellOffset[c] += thisOffs;
					assert(!std::isinf(glm::length(totalCellOffset[c])));

					totalCellOffset[n.other] += otherOffs;
					assert(!std::isinf(glm::length(totalCellOffset[n.other])));

					totalCellOffsetMod[c] += glm::length(thisOffs);
					totalCellOffsetMod[n.other] += glm::length(otherOffs);

					affectedBond = true;
				}

				// bond-contact based method ------------------------
				auto it = std::find_if(n.other->neighbours_.begin(), n.other->neighbours_.end(), [c](link const& l) {
					return l.other == c;
				});
				assert(it != n.other->neighbours_.end());
				link &otherN = *it;

				float thisBondRadius = c->radius(n.angle) + n.offset*0.5f;
				float thisWldBondAngle = c->wangle(n.angle);
				glm::vec2 thisAnchor = c->position_ + glm::vec2(cos(thisWldBondAngle), sin(thisWldBondAngle)) * thisBondRadius;

				float otherBondRadius = n.other->radius(otherN.angle) + n.offset*0.5f;
				float otherWldBondAngle = n.other->wangle(otherN.angle);
				glm::vec2 otherAnchor = n.other->position_ + glm::vec2(cos(otherWldBondAngle), sin(otherWldBondAngle)) * otherBondRadius;

				auto bdiff = otherAnchor - thisAnchor;
				float bdist = glm::length(bdiff);
				if (bdist > toleranceFactor * min(thisBondRadius, otherBondRadius)) {
					// compute displacements
					auto thisOffs = bdiff * clamp(1-ratio, 0.f, thisBondRadius * maxDisplacementRatio);
					auto otherOffs = -bdiff * clamp(ratio, 0.f, otherBondRadius * maxDisplacementRatio);

					totalCellOffset[c] += thisOffs;
					totalCellOffset[n.other] += otherOffs;

					totalCellOffsetMod[c] += glm::length(thisOffs);
					totalCellOffsetMod[n.other] += glm::length(otherOffs);

					affectedBond = true;
				}

				// house keeping
				if (affectedBond) {
					newMarked.insert(c);
					newMarked.insert(n.other);
					allAffected.insert(c);
					allAffected.insert(n.other);
				}
			}
		}
		// step 2: apply the computed offsets to cells
		massRatios.clear();
		for (auto &p : totalCellOffset) {
			p.first->position_ += p.second;
			float modulusOfSums = glm::length(p.second);
			float sumOfModuli = totalCellOffsetMod[p.first];
			massRatios[p.first] = massRatioFn(modulusOfSums / sumOfModuli);
		}

		marked.swap(newMarked);
		newMarked.clear();
		nIterations++;
	}
	return allAffected;
}

void Cell::updateBonds() {
	for (auto &n : neighbours_) {
		n.angle = rangle(limitAngle(pointDirection(n.other->position_ - position_), 2*PI));
	}
}

/*
 * ratio = size_left / size_right
 * reorientate: true to align the newly spawned cells with the division axis, false to keep parent orientation
 * mirror: true to mirror the right side - it's orientation will be mirrored with respect to division axis, and it's angles will be CW
 */
std::pair<Cell*, Cell*> Cell::divide(float division_angle, float ratio, float bondBias, bool reorientate, bool mirror, bool dontBond) {
	division_angle = limitAngle(division_angle, PI);
	float ls = size_ * ratio / (ratio + 1);	// left size
	float rs = size_ / (ratio + 1);			// right size
	float lr = sqrtf(ls / PI);				// left radius
	float rr = sqrtf(rs / PI);				// right radius
	float jr = sqrtf(jointSize_ / PI);		// joint radius
	float offset_angle = wangle(division_angle + PI/2);
	glm::vec2 offsetDir = {cosf(offset_angle), sinf(offset_angle)};
	glm::vec2 lC = position_ + offsetDir * (rr + jr);
	glm::vec2 rC = position_ - offsetDir * (lr + jr);
	float la = reorientate ? wangle(division_angle) : angle_;
	float ra = reorientate ? wangle(division_angle) : (mirror ? angle_ + 2*division_angle : angle_);

	Cell* cl = createChild(ls, lC, la, mirror_, false);
	Cell* cr = createChild(rs, rC, ra, mirror != mirror_, true);

	// create bond between siblings:
	if (!dontBond)
		cl->bond(cr, false, jr*2, this);

	// compute offsetted division axis intersection point angles
	float offsDist = 1 - 2 * ls / size_; // offset / R    E[-1, 1]
	// apply bond bias:
	offsDist = clamp(offsDist - bondBias, -1.f, +1.f);
	float offsFactor = acos(offsDist); // E[0, PI]
	float w1 = division_angle + PI/2 - offsFactor;
	float w2 = division_angle + PI/2 + offsFactor;
	constexpr float maxTolerrance = PI/16;

	// inherit parent's bonds:
	for (auto &n : neighbours_) {
		// 1. remove old bond
		Cell* other = n.other;
		other->neighbours_.erase(std::remove_if(other->neighbours_.begin(), other->neighbours_.end(), [this](link& l) {
			return l.other == this;
		}), other->neighbours_.end());

		float diff1 = limitAngle(angleDiff(w1, n.angle), w2-w1);
		float diff2 = limitAngle(angleDiff(w2, n.angle), 2*PI-w2+w1);

		if (diff1 > maxTolerrance && diff2 < -maxTolerrance) {
			// bond will be inherited only by left side
			other->bond(cl, !n.isRightSide, n.offset, n.jointParent);
		} else if (diff1 < -maxTolerrance && diff2 > maxTolerrance) {
			// bond will be inherited only by right side
			other->bond(cr, !n.isRightSide, n.offset, n.jointParent);
		} else {
			// bond will be split
			other->bond(cl, !n.isRightSide, n.offset, n.jointParent);
			other->bond(cr, !n.isRightSide, n.offset, n.jointParent);
		}
	}

	deactivate(); // this cell is no longer active (it's out of the graph of cells)

	std::set<Cell*> overlapping {cl, cr};
	for (auto n : neighbours_) {
		overlapping.insert(n.other);
	}

	auto affected = fixOverlap(overlapping);
	for (auto c : affected)
		c->updateBonds();

	return {cl, cr};
}

Cell* Cell::createChild(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide) const {
	return new Cell(size, position, rotation, mirror, rightSide);
}
