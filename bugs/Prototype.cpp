/*
 * Prototype.cpp
 *
 *  Created on: Oct 29, 2017
 *      Author: bog
 */

#include "Prototype.h"
#include "renderOpenGL/RenderContext.h"
#include "renderOpenGL/Shape3D.h"
#include "math/constants.h"
#include "utils/rand.h"
#include "World.h"

#include <GLFW/glfw3.h>

#include <vector>
#include <algorithm>

// ------------------------------------------------------------------------------

struct cell {
	glm::vec2 position_;
	float angle_;
	float size_; // area
	float division_angle_;	// relative to cell orientation angle
	bool mirror_ = false;

	struct link {
		float angle;	// relative to cell orientation angle
		cell* other;
	};
	std::vector<link> neighbours_;

	float radius() { return sqrtf(size_ / PI); }

	cell(float size, glm::vec2 position, float rotation, bool mirror)
		: position_(position), angle_(rotation), size_(size), division_angle_(randf() * 2*PI), mirror_(mirror)
	{
	}

	cell(cell &&) = delete;
	cell(cell const&) = delete;
	~cell() = default;

	// computes a world angle from a cell-relative angle, taking into account mirroring and cell orientation
	float wangle(float angle) {
		return angle_ + angle * (mirror_ ? -1 : 1);
	}

	// computes a relative angle (to cell orientation) from a world angle, taking into account mirroring and cell orientation
	float rangle(float angle) {
		return angleDiff(angle_, angle) * (mirror_ ? -1 : 1);
	}

	void bond(cell* other) {
		float angle = rangle(limitAngle(pointDirection(other->position_ - position_), 2*PI));
		neighbours_.push_back({angle, other});
		float oAngle = other->rangle(limitAngle(pointDirection(position_ - other->position_), 2*PI));
		other->neighbours_.push_back({oAngle, this});
	}

	void fixOverlap(std::set<cell*> &overlapping) {

	}

	/*
	 * ratio = size_left / size_right
	 * reorientate: true to align the newly spawned cells with the division axis, false to keep parent orientation
	 * mirror: true to mirror the right side - it's orientation will be mirrored with respect to division axis, and it's angles will be CW
	 */
	void divide(std::vector<cell*> &cells, float ratio, bool reorientate, bool mirror) {
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

		cell* cl = new cell(ls, lC, la, false);
		cell* cr = new cell(rs, rC, ra, mirror);

		// create bond between siblings:
		float lba = reorientate ? -PI/2 : -PI/2 + division_angle_;
		float rba = reorientate ? (mirror ? -PI/2 : PI/2) : (mirror ? -PI/2 + division_angle_ : PI/2 + division_angle_);
		cl->neighbours_.push_back({lba, cr});
		cr->neighbours_.push_back({rba, cl});

		// inherit parent's bonds:
		for (auto &n : neighbours_) {
			// 1. remove old bond
			cell* other = n.other;
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
			if ((diff > 0) != mirror_)
				other->bond(cl);
			else
				other->bond(cr);
		}

		auto it = std::find(cells.begin(), cells.end(), this);
		*it = cl;
		cells.push_back(cr);

		std::set<cell*> overlapping {cl, cr};
		for (auto n : neighbours_) {
			overlapping.insert(n.other);
		}

		fixOverlap(overlapping);

		delete this;
	}
};

std::vector<cell*> cells;
unsigned selected = 0;

// ------------------------------------------------------------------------------

void Prototype::draw(RenderContext const& ctx) {
	if (!enabled_)
		return;

	if (selected < cells.size()) {
		float rad = cells[selected]->radius();
		glm::vec2 pos = cells[selected]->position_;
		Shape3D::get()->drawRectangleXOYCentered(pos, {2*rad, 2*rad}, 0.f, {0, 1, 0});
	}
	for (auto c : cells) {
		Shape3D::get()->drawCircleXOY(c->position_, c->radius(), 12, {0.8f, 0.8f, 0.8f});
		glm::vec2 v2 = c->position_;
		v2.x += cosf(c->angle_) * c->radius();
		v2.y += sinf(c->angle_) * c->radius();
		Shape3D::get()->drawLine({c->position_, 0}, {v2, 0}, {1, 1, 0, 0.7f});
		v2 = c->position_;
		v2.x += cosf(c->wangle(c->division_angle_)) * c->radius() * 1.2f;
		v2.y += sinf(c->wangle(c->division_angle_)) * c->radius() * 1.2f;
		glm::vec2 v1 = c->position_ - (v2 - c->position_) * 0.7f;
		Shape3D::get()->drawLine({v1, 0}, {v2, 0}, {1, 0, 0, 0.5f});

		for (auto l : c->neighbours_) {
			glm::vec2 v1 = c->position_;
			glm::vec2 v2 = v1;
			v2.x += cosf(c->wangle(l.angle)) * c->radius();
			v2.y += sinf(c->wangle(l.angle)) * c->radius();
			v1 += (v2-v1) * 0.9f;
			Shape3D::get()->drawLine({v1, 0}, {v2, 0}, {0, 1, 1});
		}
	}
}

void Prototype::update(float dt) {
	if (!enabled_)
		return;

	if (cells.size() == 0) {
		// create initial cell
		cells.push_back(new cell(5.f, {0, 0}, randf() * 2*PI, false));
	}
}

void Prototype::onKeyDown(int key) {
	if (!enabled_)
		return;

	switch (key) {
	case GLFW_KEY_N:
		selected = (selected+1) % cells.size();
		break;
	case GLFW_KEY_1:
		cells[selected]->divide(cells, 1.f, false, false);
		break;
	case GLFW_KEY_2:
		cells[selected]->divide(cells, 1.f, true, false);
		break;
	case GLFW_KEY_3:
		cells[selected]->divide(cells, 1.f, false, true);
		break;
	case GLFW_KEY_4:
		cells[selected]->divide(cells, 1.f, true, true);
		break;
	}
}

void Prototype::onKeyUp(int key) {
	if (!enabled_)
		return;

}
