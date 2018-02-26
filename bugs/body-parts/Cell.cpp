/*
 * Cell.cpp
 *
 *  Created on: Nov 7, 2017
 *      Author: bog
 */

#include "Cell.h"

#include <boglfw/math/math3D.h>
#include <boglfw/utils/rand.h>

#include <algorithm>

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

	while (!marked.empty()) {
		for (Cell* c : marked) {
			for (auto &n : c->neighbours_) {
				auto diff = n.other->position_ - c->position_;
				float angle = pointDirection(diff);
				float dist = glm::length(diff);
				constexpr float tolerance = 1e-3f;
				float overlap = (c->radius(c->rangle(angle)) + n.other->radius(n.other->rangle(angle+PI)) + n.offset) - dist;
				if (abs(overlap) <= tolerance)
					continue;
				glm::vec2 offset = glm::normalize(n.other->position_ - c->position_) * overlap;
				float ratio = c->size_ / (c->size_ + n.other->size_);

				c->position_ -= offset * (1-ratio);
				n.other->position_ += offset * ratio;

				newMarked.insert(c);
				newMarked.insert(n.other);
				allAffected.insert(c);
				allAffected.insert(n.other);
			}
		}
		marked.swap(newMarked);
		newMarked.clear();
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
