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

bool angleSpanOverlap(float angle1, float span1, float angle2, float span2, bool sweepPositive, float &outMargin) {
	// will set outMargin to negative if overlap, or positive shortest gap around element if no overlap
	// move angle1 in origin for convenience:
	angle2 = limitAngle(angle2 - angle1, PI);
	angle1 = 0;
	float a1p = span1*0.5f, a1n = -span1*0.5f;
	float a2p = angle2 + span2*0.5f, a2n = angle2 - span2*0.5f;
	if (a2p*a2n >= 0) {
		// both ends of span2 are on the same side of angle 1
		if (a2p > 0) { // both on the positive side
			outMargin = min(a2p, a2n) - a1p;
			if (!sweepPositive)
				outMargin = 2*PI-outMargin-span1-span2;
		} else {// both on the negative side
			outMargin = a1n - max(a2n, a2p);
			if (sweepPositive)
				outMargin = 2*PI-outMargin-span1-span2;
		}
	} else { // ends of span2 are on different sides of angle 1
		float d1 = a2n-a1p, d2 = a1n-a2p;
		outMargin = angle2 > 0 ? d1 : d2; // the smallest distance between the spans (positive) or the greatest overlap (negative)
		if (sweepPositive == (angle2<=0))
			outMargin = 2*PI-outMargin-span1-span2;
	}

	return outMargin < 0;
}
