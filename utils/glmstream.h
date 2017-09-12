/*
 * glmstream.h
 *
 *  Created on: May 29, 2017
 *      Author: alexandra
 */

#ifndef UTILS_GLMSTREAM_H_
#define UTILS_GLMSTREAM_H_

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/quaternion.hpp>

inline std::ostream& operator <<(std::ostream& str, glm::fquat const& q) {
	return str << "{" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << "}";
}

inline std::ostream& operator <<(std::ostream& str, glm::vec3 const& v) {
	return str << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

inline std::ostream& operator <<(std::ostream& str, glm::vec2 const& v) {
	return str << "{" << v.x << ", " << v.y << "}";
}



#endif /* UTILS_GLMSTREAM_H_ */
