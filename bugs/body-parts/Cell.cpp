/*
 * Cell.cpp
 *
 *  Created on: Nov 7, 2017
 *      Author: bog
 */

#include "Cell.h"

#include <boglfw/math/math3D.h>
#include <boglfw/utils/rand.h>

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

std::set<Cell*> Cell::fixOverlap(std::set<Cell*> &marked) {
	// 1. push all overlapping cells away until they touch on the edges
	// 2. pull all bonded cells inward until they touch on the edges
	// 3. mark all cells that have been moved and their neighbors
	// 4. repeat until no more marked cells

	std::set<Cell*> newMarked;
	std::set<Cell*> allAffected;

	constexpr float angleChangeTolerance = PI * 0.05f;

	/*
	 * while (!marked.empty()) {
		std::map<Cell*, glm::vec2> totalCellOffsets;
		std::set<std::pair<Cell*, Cell*>> affectedBonds;
		for (Cell* c : marked) {
			for (auto &n : c->neighbours_) {
				if (affectedBonds.find({c, n.other}) != affectedBonds.end())
					continue;	// we already treated this bond this round
				affectedBonds.insert({n.other, c});

				auto diff = n.other->position_ - c->position_;
				float angle = pointDirection(diff);
				float dist = glm::length(diff);
				constexpr float tolerance = 1e-3f;
				float overlap = (c->radius(c->rangle(angle)) + n.other->radius(n.other->rangle(angle+PI)) + n.offset) - dist;
				if (abs(overlap) <= tolerance)
					continue;
				glm::vec2 offset = glm::normalize(n.other->position_ - c->position_) * overlap;
				float ratio = c->size_ / (c->size_ + n.other->size_);

//				c->position_ -= offset * (1-ratio);
//				n.other->position_ += offset * ratio;
				totalCellOffsets[c] -= offset * (1-ratio);
				totalCellOffsets[n.other] += offset * ratio;

				newMarked.insert(c);
				newMarked.insert(n.other);
				allAffected.insert(c);
				allAffected.insert(n.other);
			}
		}
		for (Cell* c : marked) {
			c->position_ += totalCellOffsets[c];
		}
		marked.swap(newMarked);
		newMarked.clear();
	}
	 */

	constexpr int maxIterations = 10;
	int nIterations = 0;
	while (!marked.empty() && nIterations < maxIterations) {
		std::set<std::pair<Cell*, Cell*>> affectedBonds;
		std::map<Cell*, glm::vec2> totalCellOffset;
		// step 1: compute cell offsets to put the bonding points in contact
		for (Cell* c : marked) {
			for (auto &n : c->neighbours_) {
				if (affectedBonds.find({c, n.other}) != affectedBonds.end())
					continue;	// we already treated this bond this round
				affectedBonds.insert({n.other, c});
				constexpr float tolerance = 1e-3f;
// bond-contact based method ------------------------
				/*
				auto it = std::find_if(n.other->neighbours_.begin(), n.other->neighbours_.end(), [c](link const& l) {
					return l.other == c;
				});
				assert(it != n.other->neighbours_.end());
				link &otherN = *it;

				float thisBondRadius = c->radius(n.angle);
				float thisWldBondAngle = c->wangle(n.angle);
				glm::vec2 thisAnchor = c->position_ + glm::vec2(cos(thisWldBondAngle), sin(thisWldBondAngle)) * thisBondRadius;

				float otherBondRadius = n.other->radius(otherN.angle);
				float otherWldBondAngle = n.other->wangle(otherN.angle);
				glm::vec2 otherAnchor = n.other->position_ + glm::vec2(cos(otherWldBondAngle), sin(otherWldBondAngle)) * otherBondRadius;

				auto diff = otherAnchor - thisAnchor;
				float dist = glm::length(diff);
				if (dist <= tolerance)
					continue;

				float ratio = c->size_ / (c->size_ + n.other->size_);
				auto thisOffs = diff * (1-ratio);
				auto otherOffs = -diff * ratio;
//				c->position_ += thisOffs;
//				n.other->position_ += otherOffs;
 */
// ------------------- end bond-contact based

// center-based push override ----------------------------
				auto diff = n.other->position_ - c->position_;
				float angle = pointDirection(diff);
				auto dist = glm::length(diff);
				float overlap = c->radius(c->rangle(angle)) + n.other->radius(n.other->rangle(angle+PI)) + n.offset - dist;
				if (abs(overlap) <= tolerance)
					continue;
				glm::vec2 offset = glm::normalize(diff) * overlap;
				float ratio = c->size_ / (c->size_ + n.other->size_);
				constexpr float overshootFactor = 1.f; // overshoot slightly to speed up convergence
				auto thisOffs = -offset * (1-ratio) * overshootFactor;
				auto otherOffs = offset * ratio * overshootFactor;
// ------------------------ end center-based push override

				totalCellOffset[c] += thisOffs;
				totalCellOffset[n.other] += otherOffs;

				newMarked.insert(c);
				newMarked.insert(n.other);
				allAffected.insert(c);
				allAffected.insert(n.other);
			}
		}
		// step 2: apply the computed offsets to cells
		for (auto &p : totalCellOffset) {
			p.first->position_ += p.second;
		}
		// step 3: offset the bond angles slightly toward the new positions - this is required because the new configuration may be
		// unsolvable with the original bond angles
		/*float totalAngleChange = 0;
		affectedBonds.clear();
		for (Cell* c : marked) {
			for (auto &n : c->neighbours_) {
				if (affectedBonds.find({c, n.other}) != affectedBonds.end())
					continue;	// we already treated this bond this round
				affectedBonds.insert({n.other, c});
				auto it = std::find_if(n.other->neighbours_.begin(), n.other->neighbours_.end(), [c](link const& l) {
					return l.other == c;
				});
				assert(it != n.other->neighbours_.end());
				link &otherN = *it;

				float wldBondAngle = pointDirection(n.other->position_ - c->position_);
				float angleChangeRatio = 0.1f; // [0.0 .. 1.0] 0.0 keeps it in place, 1.0 moves it completely to the actual bond angle
				float newAngle = lerp(n.angle, c->rangle(wldBondAngle), angleChangeRatio);
				float newOtherAngle = lerp(otherN.angle, n.other->rangle(PI + wldBondAngle), angleChangeRatio);

				float thisAngleChange = abs(newAngle - n.angle);
				float otherAngleChange = abs(newOtherAngle - otherN.angle);
				totalAngleChange += thisAngleChange + otherAngleChange;

				if (thisAngleChange >= angleChangeTolerance)
					newMarked.insert(c);
				if (otherAngleChange >= angleChangeTolerance)
					newMarked.insert(n.other);

				n.angle = newAngle;
				otherN.angle = newOtherAngle;
			}
		}*/
		/*if (totalAngleChange < angleChangeTolerance)*/ {
			// check how much all the cells have been offsetted together to avoid infinite looping by moving them back and forth
//			glm::vec2 totalOffs = std::accumulate(totalCellOffset.begin(), totalCellOffset.end(), glm::vec2(0), [](glm::vec2 v, auto p) {
//				return v + glm::vec2{abs(p.second.x), abs(p.second.y)};
//			});
//			constexpr float perCellTolerance = 0.00005f;
//			float totalTolerance = perCellTolerance * newMarked.size();
//			if (glm::length(totalOffs) < totalTolerance)
//				break;
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
