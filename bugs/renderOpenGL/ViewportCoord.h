/*
 * ViewportCoord.h
 *
 *  Created on: May 6, 2017
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_VIEWPORTCOORD_H_
#define RENDEROPENGL_VIEWPORTCOORD_H_

#include <glm/vec2.hpp>

#include <functional>
#include <vector>

class Viewport;
class Adjustment;

class ViewportCoord {
public:
	enum anchors {
		left		= 1,
		right		= 2,
		top			= 4,
		bottom		= 8
	};
	enum type {
		absolute,		// absolute viewport coordinates (in pixels)
		percent,		// coordinates are percentages of viewport size
		callback		// coordinates will be provided by user callback on a per-viewport basis
	};

	using userCallback = std::function<float(Viewport* vp)>;

	ViewportCoord() = default;

	// builds an absolute coordinate (in pixels) or relative (in percents of viewport size)
	ViewportCoord(float x, float y, type t = absolute, anchors a = (anchors)(left | top))
		: anchor_(a), type_(t), x_(x), y_(y) {
	}

	ViewportCoord(glm::vec2 abs) : ViewportCoord(abs.x, abs.y, absolute) {}

	// builds a deferred coordinate representation that will be computed at the actual time of rendering, on a per-viewport basis
	// the callbacks must return absolute values (anchors still apply)
	ViewportCoord(userCallback x, userCallback y, anchors a = (anchors)(left | top))
		: anchor_(a), type_(callback), xc_(x), yc_(y) {
	}

	//ViewportCoord(ViewportCoord const& c);

	// creates a new coordinate representation scaled by a factor
	ViewportCoord scale(float s) const;

	ViewportCoord operator+(ViewportCoord x) const;
	ViewportCoord operator-(ViewportCoord x) const;
	ViewportCoord operator*(float s) const { return scale(s); }
	ViewportCoord operator/(float s) const { return scale(1.f / s); }

	// get only x or y component:
	ViewportCoord x() const;
	ViewportCoord y() const;

	// compute the actual pixel value within a viewport:
	float x(Viewport* vp) const;
	float y(Viewport* vp) const;

#ifdef DEBUG
	void debugPrint();
#endif

private:
	anchors anchor_ = (anchors)(left | top);
	type type_ = absolute;
	float x_ = 0;
	float y_ = 0;
	userCallback xc_;
	userCallback yc_;

	ViewportCoord(anchors a, type t, float x, float y, userCallback xc, userCallback yc)
		: anchor_(a), type_(t), x_(x), y_(y), xc_(xc), yc_(yc) {
	}

	std::vector<Adjustment> deferredAdjustments_;

#ifdef DEBUG
	void debugPrintInternal(int indent);
#endif
};

struct Adjustment {
	enum {
		add,
		scale
	} type;
	ViewportCoord value;
	float weightX, weightY;
};

#endif /* RENDEROPENGL_VIEWPORTCOORD_H_ */
