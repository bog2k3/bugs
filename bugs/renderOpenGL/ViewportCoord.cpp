/*
 * ViewportCoord.cpp
 *
 *  Created on: May 6, 2017
 *      Author: bogdan
 */

#include "ViewportCoord.h"
#include "Viewport.h"

#include "../utils/bitFlags.h"

#ifdef DEBUG
#include "../utils/log.h"
#include <string>
#endif

float ViewportCoord::x(Viewport* vp) const {
	float x = x_;
	if (type_ == callback) {
		if (xc_)
			x += xc_(vp);
	} else if (type_ == percent)
		x *= vp->width() * 0.01f;
	if (anchor_ & right)
		x = vp->width() - x;
	for (auto& a : deferredAdjustments_) {
		if (a.type == Adjustment::scale)
			x *= a.weightX;
		else
			x += a.weightX * a.value.x(vp);
	}
	return x;
}

float ViewportCoord::y(Viewport* vp) const {
	float y = y_;
	if (type_ == callback) {
		if (yc_)
			y += yc_(vp);
	} else if (type_ == percent)
		y *= vp->height() * 0.01f;
	if (anchor_ & bottom)
		y = vp->height() - y;
	for (auto& a : deferredAdjustments_) {
		if (a.type == Adjustment::scale)
			y *= a.weightY;
		else
			y += a.weightY * a.value.y(vp);
	}
	return y;
}

ViewportCoord ViewportCoord::scale(float s) const {
	if (s == 0)
		return {0, 0};
	auto ret = *this;
	ret.deferredAdjustments_.push_back({Adjustment::scale, {0, 0}, s, s});
	return ret;
}

ViewportCoord ViewportCoord::operator+(ViewportCoord x) const {
	auto ret = *this;
	if (x.x_ != 0 || x.xc_ || x.y_ != 0 || x.yc_) {
		float signX = (x.anchor_ & right) ? -1.f : +1.f;
		float signY = (x.anchor_ & bottom) ? -1.f : +1.f;
		x.anchor_ = left | top;
		ret.deferredAdjustments_.push_back({Adjustment::add, x, signX, signY});
	}
	return ret;
}

ViewportCoord ViewportCoord::operator-(ViewportCoord x) const {
	auto ret = *this;
	if (x.x_ != 0 || x.xc_ || x.y_ != 0 || x.yc_) {
		float signX = (x.anchor_ & right) ? +1.f : -1.f;
		float signY = (x.anchor_ & bottom) ? +1.f : -1.f;
		x.anchor_ = left | top;
		ret.deferredAdjustments_.push_back({Adjustment::add, x, signX, signY});
	}
	return ret;
}

ViewportCoord ViewportCoord::x() const {
	ViewportCoord ret {anchor_, type_, x_, 0, xc_, nullptr};
	for (auto &a : deferredAdjustments_)
		if (a.type == Adjustment::scale || a.value.deferredAdjustments_.size() || a.value.x_ != 0 || a.value.xc_)
		ret.deferredAdjustments_.push_back({a.type, a.value.x(), a.weightX, 0});
	return ret;
}

ViewportCoord ViewportCoord::y() const {
	ViewportCoord ret {anchor_, type_, 0, y_, nullptr, yc_};
	for (auto &a : deferredAdjustments_)
		if (a.type == Adjustment::scale || a.value.deferredAdjustments_.size() || a.value.y_ != 0 || a.value.yc_)
		ret.deferredAdjustments_.push_back({a.type, a.value.y(), 0, a.weightY});
	return ret;
}

#ifdef DEBUG
void ViewportCoord::debugPrint() {
	debugPrintInternal(0);
}

void ViewportCoord::debugPrintInternal(int indent) {
	LOGLN(std::string(indent, ' ') << "{")
	LOGLN(std::string(indent+2, ' ') << (anchor_ & left ? "left" : "right") << ": " << x_ << (type_ & percent ? "%" : "px"))
	LOGLN(std::string(indent+2, ' ') << (anchor_ & top ? "top" : "bottom") << ": " << y_ << (type_ & percent ? "%" : "px"))
	if (deferredAdjustments_.size()) {
		LOGLN(std::string(indent+2, ' ') << "adjustments: [");
		for (auto &a : deferredAdjustments_) {
			if (a.type == Adjustment::scale)
				LOGLN(std::string(indent+4, ' ') << "scale: " << a.weightX)
			else {
				LOGLN(std::string(indent+4, ' ') << "combine (" << a.weightX << ", " << a.weightY << ")")
				a.value.debugPrintInternal(indent+4);
			}
		}
		LOGLN(std::string(indent+2, ' ') << "]");
	}
	LOGLN(std::string(indent, ' ') << "}");
}
#endif

