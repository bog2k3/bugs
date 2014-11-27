/*
 * box2glm.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#ifndef MATH_BOX2GLM_H_
#define MATH_BOX2GLM_H_

#include <Box2D/Common/b2Math.h>
#include <Box2D/Common/b2Draw.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

inline glm::vec2 b2g(b2Vec2 const& v) {
	return glm::vec2(v.x, v.y);
}

inline glm::vec3 b2g(b2Vec3 const& v) {
	return glm::vec3(v.x, v.y, v.z);
}

inline glm::vec3 b2g(b2Color const& v) {
	return glm::vec3(v.r, v.g, v.b);
}

inline b2Vec2 g2b(glm::vec2 const &v) {
	return b2Vec2(v.x, v.y);
}

#endif /* MATH_BOX2GLM_H_ */
