#include <algorithm>
#include "math2D.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

glm::vec2 rayIntersectBox(float length, float width, float direction) {
	float hw = width * 0.5f, hl = length * 0.5f;	// half width and length
	// bring the angle between [-PI, +PI]
	float phiQ = atanf(width/length);
	float relativeAngle = limitAngle(direction, 2*PI-phiQ);
	if (relativeAngle < phiQ) {
		// front edge
		glm::vec2 ret(hl, sinf(relativeAngle) * hw);
		return ret;
	} else if (relativeAngle < PI-phiQ  || relativeAngle > PI+phiQ) {
		// left or right edge
		glm::vec2 ret(cosf(relativeAngle) * hl, relativeAngle < PI ? hw : -hw);
		return ret;
	} else {
		// back edge
		glm::vec2 ret(-hl, sinf(relativeAngle) * hw);
		return ret;
	}
}
