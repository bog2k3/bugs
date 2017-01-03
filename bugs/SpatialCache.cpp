/*
 * SpatialCache.cpp
 *
 *  Created on: Jan 2, 2017
 *      Author: bog
 */

#include "SpatialCache.h"
#include "entities/Entity.h"
#include "math/aabb.h"
#include "perf/marker.h"

#include <cmath>

static constexpr float preferredCellSize = 5;	// meters
static constexpr int minCellsPerAxis = 5;		// less - faster cache, but less accurate, which results in more time spent in user code
static constexpr int maxCellsPerAxis = 256;		// the more, the better, but more memory may be used

SpatialCache::SpatialCache(float left, float right, float top, float bottom)
	: left_(left), right_(right), top_(top), bottom_(bottom)
{
	// decide the number of cells:
	width_ = std::min(maxCellsPerAxis, std::max<int>(minCellsPerAxis, (right - left) / preferredCellSize));
	height_ = std::min(maxCellsPerAxis, std::max<int>(minCellsPerAxis, (top - bottom) / preferredCellSize));

	// these will usually be roughly equal
	cellWidth_ = (right - left) / width_;
	cellHeight_ = (top - bottom) / height_;
	cellRadius_ = std::max(cellWidth_, cellHeight_);

	// now create the cells:
	cells_ = new cell*[height_];
	for (int i=0; i<height_; i++) {
		cells_[i] = new cell[width_];
		for (int j=0; j<width_; j++)
			cells_[i][j].pos_ = glm::vec2(left + (j+0.5f)*cellWidth_, top - (i+0.5f)*cellHeight_);
	}
}

SpatialCache& SpatialCache::operator = (SpatialCache &&c) {
	left_ = c.left_;
	right_ = c.right_;
	top_ = c.top_;
	bottom_ = c.bottom_;
	cellWidth_ = c.cellWidth_;
	cellHeight_ = c.cellHeight_;
	cellRadius_ = c.cellRadius_;
	width_ = c.width_;
	height_ = c.height_;
	cells_ = c.cells_;
	c.cells_ = nullptr;
	return *this;
}

SpatialCache::~SpatialCache() {
	if (!cells_)
		return;
	for (int i=0; i<height_; i++)
		delete [] cells_[i];
	delete [] cells_;
}

void SpatialCache::getCachedEntities(std::vector<Entity*> &out, glm::vec2 const& pos, float radius, bool clipToCircle, int frameID,
			getEntitiesFromBox2DFunc getFn, validateEntityFunc validFn)
{
	if (!cells_) {
		// pass-through
		LOGPREFIX("SpatialCache");
		LOGLN("WARNING! Cache not constructed properly, working as pass-through!");
		getFn(pos, radius, out);
		return;
	}
	PERF_MARKER_FUNC;
	float left = pos.x - radius;
	float right = pos.x + radius;
	float top = pos.y + radius;
	float bottom = pos.y - radius;
	aabb area(glm::vec2(left, bottom), glm::vec2(right, top));

	int i1 = clamp<int>(floorf(top_ - top) / cellHeight_, 0, height_-1);
	int i2 = clamp<int>(ceilf(top_ - bottom) / cellHeight_, 0, height_-1);

	int j1 = clamp<int>(floorf(left - left_) / cellWidth_, 0, width_-1);
	int j2 = clamp<int>(ceilf(right - left_) / cellWidth_, 0, width_-1);

	for (int i=i1; i<i2; i++)
		for (int j=j1; j<j2; j++) {
			bool expectedLocked = false;
			while (!cells_[i][j].locked_.compare_exchange_weak(expectedLocked, true, std::memory_order_acq_rel, std::memory_order_relaxed))
				expectedLocked = false; // spin wait
			if (cells_[i][j].lastUpdateFrame_ != frameID) {
				PERF_MARKER("update-cache");
				// update cell
				cells_[i][j].entities_.clear();
				getFn(cells_[i][j].pos_, cellRadius_, cells_[i][j].entities_);
				cells_[i][j].lastUpdateFrame_ = frameID;
			}
			for (Entity* e : cells_[i][j].entities_) {
				// test against requested area:
				aabb eAABB = eAABB= e->getAABB();
				if (eAABB.intersect(area).empty())
					continue;
				if (clipToCircle && !eAABB.intersectCircle(pos, radius))
					continue;
				// user validation:
				if (!validFn(e))
					continue;
				out.push_back(e);
			}
			cells_[i][j].locked_.store(false, std::memory_order_release);
		}
}
