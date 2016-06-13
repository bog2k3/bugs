/*
 * aabb.h
 *
 *  Created on: Jun 13, 2016
 *      Author: bog
 */

#ifndef MATH_AABB_H_
#define MATH_AABB_H_

#include "box2glm.h"
#include <glm/vec2.hpp>
#include <limits>

struct aabb {
	glm::vec2 vMin;
	glm::vec2 vMax;

	aabb()
		: vMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
		, vMax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()){
		// default creates empty
	}

	aabb(glm::vec2 vMin, glm::vec2 vMax)
		: vMin(vMin), vMax(vMax) {
	}

	aabb(b2AABB const& b2aabb)
		: vMin(b2g(b2aabb.lowerBound))
		, vMax(b2g(b2aabb.upperBound)) {
	}

	operator b2AABB() const {
		b2AABB b;
		b.lowerBound = g2b(vMin);
		b.upperBound = g2b(vMax);
		return b;
	}

	aabb(const aabb& x) = default;
	aabb& operator = (aabb const& x) = default;

	bool empty() {
		return vMin.x > vMax.x || vMin.y > vMax.y;
	}

	aabb reunion(aabb const& x) {
		aabb o(*this);
		if (o.vMin.x > x.vMin.x)
			o.vMin.x = x.vMin.x;
		if (o.vMin.y > x.vMin.y)
			o.vMin.y = x.vMin.y;
		if (o.vMax.x < x.vMax.x)
			o.vMax.x = x.vMax.x;
		if (o.vMax.y < x.vMax.y)
			o.vMax.y = x.vMax.y;
		return o;
	}

	aabb intersect(aabb const& x) {
		// TODO implement when needed
		return aabb();
	}
};

#endif /* MATH_AABB_H_ */
