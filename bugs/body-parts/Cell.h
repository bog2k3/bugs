/*
 * Cell.h
 *
 *  Created on: Nov 7, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_CELL_H_
#define BODY_PARTS_CELL_H_

#include <glm/vec2.hpp>

#include <vector>
#include <set>

class Cell {
public:

	struct link {
		float angle;	// relative to cell orientation angle
		float offset;	// distance between cells to accomodate for pivot joint
		bool isRightSide;	// true if this side of the joint was initially the "right-side" of the splitting cell
		Cell* other;
		Cell* jointParent;	// this is the original cell that created the joint; it contains the joint's attributes
	};

	Cell(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide);
	Cell(Cell &&) = delete;
	Cell(Cell const&) = delete;
	virtual ~Cell() = default;

	bool isActive() const { return active_; }

	// returns the radius in a given local angle (relative to cell's orientation)
	// that is distance from center to cell's outline - it may be non-uniform for non-circle cells
	virtual float radius(float angle) const;
	// computes a world angle from a cell-relative angle, taking into account mirroring and cell orientation
	float wangle(float angle);
	// computes a relative angle (to cell orientation) from a world angle, taking into account mirroring and cell orientation
	float rangle(float angle);

	const glm::vec2& position() const { return position_; }
	float angle() const { return angle_; }
	float size() const { return size_; }
	bool isMirrored() const { return mirror_; }

	unsigned neighbourCount() const { return neighbours_.size(); }
	link neighbour(unsigned index) const { return neighbours_[index]; }

	void bond(Cell* other, bool isRightSide, float jointDiameter, Cell* jointParent);
	void updateBonds();

	// set the size of the pivot joint bond that will be created between the children during division;
	// initial value is zero, which means cells are closely welded together
	void setJointSize(float size) { jointSize_ = size; }

	/*
	 * ratio = size_left / size_right
	 * reorientate: true to align the newly spawned cells with the division axis, false to keep parent orientation
	 * mirror: true to mirror the right side - it's orientation will be mirrored with respect to division axis, and it's angles will be CW
	 */
	std::pair<Cell*, Cell*> divide(float division_angle, float ratio, float bondBias, bool reorientate, bool mirror, bool dontBond);

	void deactivate() { active_ = false; }

	static std::set<Cell*> fixOverlap(std::set<Cell*> &marked);

protected:
	virtual Cell* createChild(float size, glm::vec2 position, float rotation, bool mirror, bool rightSide) const;
	float angle_;
	glm::vec2 position_;

private:
	friend class Ribosome;

	bool active_ = true;
	float size_; // area
	bool mirror_ = false;
	bool rightSide_ = false;
	std::vector<link> neighbours_;
	float jointSize_ = 0;

};

#endif /* BODY_PARTS_CELL_H_ */
