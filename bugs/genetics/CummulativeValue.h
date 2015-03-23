/*
 * CummulativeValue.h
 *
 *  Created on: Dec 22, 2014
 *      Author: bogdan
 */

#ifndef GENETICS_CUMMULATIVEVALUE_H_
#define GENETICS_CUMMULATIVEVALUE_H_

#include <cassert>
#include <cmath>
#include "../utils/assert.h"

struct CummulativeValue {
	CummulativeValue() : value_(0), cachedValue_(0), cacheUpdated_(false), n_(0), factor_(1.f) {}
	explicit CummulativeValue(float initial) : value_(initial), cachedValue_(value_), cacheUpdated_(true), n_(1), factor_(1.f) {}
	inline operator float() {
		if (!cacheUpdated_)
			updateCache();
		assertDbg(!std::isnan(cachedValue_));
		return cachedValue_;
	}
	inline float get() {
		return *this;
	}
	inline void changeAbs(float change) {
		value_ += change;
		n_++;
		cacheUpdated_ = false;
	}
	inline void changeRel(float factor) {
		factor_ *= factor;
		cacheUpdated_ = false;
	}
	inline void reset(float initialValue) {
		*this = CummulativeValue(initialValue);
	}
	inline bool hasValue() const { return n_ > 0; }
	inline float clamp(float min, float max) {
		if (get() < min)
			return min;
		if (get() > max)
			return max;
		return get();
	}
private:
	inline void updateCache() {
		assertDbg(n_ > 0 && "trying to read empty (uninitialized) CummulativeValue !!!");
		cachedValue_ = value_ * factor_ / n_;
		cacheUpdated_ = true;
	}

	float value_;
	float cachedValue_;
	bool cacheUpdated_;
	int n_;
	float factor_;
};


#endif /* GENETICS_CUMMULATIVEVALUE_H_ */
