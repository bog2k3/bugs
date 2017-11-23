/*
 * Cell.h
 *
 *  Created on: Nov 7, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_CELL_H_
#define BODY_PARTS_CELL_H_

#include <vector>
#include <set>

#include <glm/vec2.hpp>

class Cell {
public:

	struct link {
		float angle;	// relative to cell orientation angle
		Cell* other;
	};

	Cell(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide);
	Cell(Cell &&) = delete;
	Cell(Cell const&) = delete;
	virtual ~Cell() = default;

	// returns the radius in a given local angle (relative to cell's orientation)
	// that is distance from center to cell's outline - it may be non-uniform for non-circle cells
	virtual float radius(float angle) const;
	// computes a world angle from a cell-relative angle, taking into account mirroring and cell orientation
	float wangle(float angle);
	// computes a relative angle (to cell orientation) from a world angle, taking into account mirroring and cell orientation
	float rangle(float angle);

	const glm::vec2& position() const { return position_; }

	void bond(Cell* other);
	void updateBonds();
	/*
	 * ratio = size_left / size_right
	 * reorientate: true to align the newly spawned cells with the division axis, false to keep parent orientation
	 * mirror: true to mirror the right side - it's orientation will be mirrored with respect to division axis, and it's angles will be CW
	 */
	void divide(std::vector<Cell*> &cells, float ratio, bool reorientate, bool mirror);

	static void fixOverlap(std::set<Cell*> &marked);

private:
	glm::vec2 position_;
	float angle_;
	float size_; // area
	float division_angle_;	// relative to cell orientation angle
	bool mirror_ = false;
	bool rightSide_ = false;
	std::vector<link> neighbours_;

};

#endif /* BODY_PARTS_CELL_H_ */
