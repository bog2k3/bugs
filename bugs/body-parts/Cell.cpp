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
	: position_(position)
	, angle_(limitAngle(rotation, 2*PI))
	, size_(size), division_angle_(randf() * 2*PI), mirror_(mirror), rightSide_(rightSide)
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

void Cell::bond(Cell* other) {
	float angle = rangle(limitAngle(pointDirection(other->position_ - position_), 2*PI));
	neighbours_.push_back({angle, other});
	float oAngle = other->rangle(limitAngle(pointDirection(position_ - other->position_), 2*PI));
	other->neighbours_.push_back({oAngle, this});
}

void Cell::fixOverlap(std::set<Cell*> &marked) {
	// 1. push all overlapping cells away until they touch on the edges
	// 2. pull all bonded cells inward until they touch on the edges
	// 3. mark all cells that have been moved and their neighbors
	// 4. repeat until no more marked cells

	std::set<Cell*> newMarked;

	while (!marked.empty()) {
		for (Cell* c : marked) {
			for (auto &n : c->neighbours_) {
				auto diff = n.other->position_ - c->position_;
				float angle = pointDirection(diff);
				float dist = glm::length(diff);
				constexpr float tolerance = 1e-3f;
				float overlap = (c->radius(c->rangle(angle)) + n.other->radius(n.other->rangle(angle+PI))) - dist;
				if (abs(overlap) <= tolerance)
					continue;
				glm::vec2 offset = glm::normalize(n.other->position_ - c->position_) * overlap;
				float ratio = c->size_ / (c->size_ + n.other->size_);

				c->position_ -= offset * (1-ratio);
				n.other->position_ += offset * ratio;

				newMarked.insert(c);
				newMarked.insert(n.other);
			}
		}
		marked.swap(newMarked);
		newMarked.clear();
	}
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
void Cell::divide(std::vector<Cell*> &cells, float ratio, bool reorientate, bool mirror) {
	float ls = size_ * ratio / (ratio + 1);
	float rs = size_ / (ratio + 1);
	float lr = sqrtf(ls / PI);
	float rr = sqrtf(rs / PI);
	float offset_angle = wangle(division_angle_ + PI/2);
	glm::vec2 offsetDir = {cosf(offset_angle), sinf(offset_angle)};
	glm::vec2 lC = position_ + offsetDir * lr;
	glm::vec2 rC = position_ - offsetDir * rr;
	float la = reorientate ? wangle(division_angle_) : angle_;
	float ra = reorientate ? wangle(division_angle_) : (mirror ? angle_ + 2*division_angle_ : angle_);

	Cell* cl = new Cell(ls, lC, la, mirror_, false);
	Cell* cr = new Cell(rs, rC, ra, mirror != mirror_, true);

	// create bond between siblings:
	cl->bond(cr);

	// inherit parent's bonds:
	for (auto &n : neighbours_) {
		// 1. remove old bond
		Cell* other = n.other;
		other->neighbours_.erase(std::find_if(other->neighbours_.begin(), other->neighbours_.end(), [this](link& l) {
			return l.other == this;
		}));
		constexpr float maxTolerrance = PI/16;
		float diff = angleDiff(division_angle_, n.angle);
		if ( abs(diff) <= maxTolerrance || PI - abs(diff) <= maxTolerrance) {
			// bond will be split
			other->bond(cl);
			other->bond(cr);

			continue;
		}
		// bond will be inherited by only one side
		if (diff > 0)
			other->bond(cl);
		else
			other->bond(cr);
	}

	auto it = std::find(cells.begin(), cells.end(), this);
	*it = cl;
	cells.push_back(cr);

	std::set<Cell*> overlapping {cl, cr};
	for (auto n : neighbours_) {
		overlapping.insert(n.other);
	}

	fixOverlap(overlapping);
	for (auto c : cells)
		c->updateBonds();

	delete this;
}
