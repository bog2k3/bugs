/*
 * SpatialCache.h
 *
 *  Created on: Jan 2, 2017
 *      Author: bog
 */

#ifndef SPATIALCACHE_H_
#define SPATIALCACHE_H_

#include <glm/vec2.hpp>
#include <vector>
#include <functional>
#include <atomic>

class Entity;

class SpatialCache {
public:
	SpatialCache() = default;	// default ctor - cache will act as a simple pass-through when constructed like this

	// provide extents on X and Y axis to cover with this cache
	SpatialCache(float left, float right, float top, float bottom);
	~SpatialCache();

	SpatialCache& operator = (SpatialCache &&c);

	using getEntitiesFromBox2DFunc = std::function<void(glm::vec2 const& pos, float radius, std::vector<Entity*> &out)>;
	using validateEntityFunc = std::function<bool(Entity*)>;
	void getCachedEntities(std::vector<Entity*> &out, glm::vec2 const& pos, float radius, bool clipToCircle, int frameID,
			getEntitiesFromBox2DFunc getFn, validateEntityFunc validFn);

private:
	struct cell {
		std::vector<Entity*> entities_;
		glm::vec2 pos_;	// center of cell
		int lastUpdateFrame_ = 0;
		std::atomic<bool> locked_ {false};
	};
	cell** cells_ = nullptr;

	// in meters:
	float left_=0, right_=0, top_=0, bottom_=0;
	float cellWidth_=0, cellHeight_=0;
	float cellRadius_ = 0; // this is an approximation

	// in number of cells:
	int width_=0, height_=0;
};

#endif /* SPATIALCACHE_H_ */
