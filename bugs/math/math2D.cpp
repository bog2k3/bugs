#include <algorithm>
#include "math2D.h"

glm::vec2 rayIntersectBox(float width, float height, float direction) {
	float hw = width * 0.5f, hl = height * 0.5f;	// half width and length
	// bring the angle between [-PI, +PI]
	float relativeAngle = limitAngle(direction, 7*PI/4);
	if (relativeAngle < PI/4) {
		// front edge
		return glm::vec2(hl, sinf(relativeAngle) * hw);
	} else if (relativeAngle < 3*PI/4 || relativeAngle > 5*PI/4) {
		// left or right edge
		return glm::vec2(cosf(relativeAngle) * hl, relativeAngle < PI ? hw : -hw);
	} else {
		// back edge
		return glm::vec2(-hl, sinf(relativeAngle) * hw);
	}
}
