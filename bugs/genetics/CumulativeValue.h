/*
 * CumulativeValue.h
 *
 *  Created on: Dec 22, 2014
 *      Author: bogdan
 */

#ifndef GENETICS_CumulativeValue_H_
#define GENETICS_CumulativeValue_H_

#include <boglfw/utils/assert.h>

#include <cassert>
#include <cmath>

struct CumulativeValue {
	CumulativeValue() {}
	explicit CumulativeValue(float initial) : value_(initial), cachedValue_(value_), cacheUpdated_(true), n_(1), factor_(1.f) {}
	inline operator float() const {
		if (!hasValue())
			return 0;
		if (!cacheUpdated_)
			updateCache();
		assertDbg(!std::isnan(cachedValue_));
		return cachedValue_;
	}
	inline float get() const {
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
		*this = CumulativeValue(initialValue);
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
	inline void updateCache() const {
		assertDbg(n_ > 0 && "trying to read empty (uninitialized) CumulativeValue !!!");
		cachedValue_ = value_ * factor_ / n_;
		cacheUpdated_ = true;
	}

	float value_ = 0;
	mutable float cachedValue_ = 0;
	mutable bool cacheUpdated_ = false;
	int n_ = 0;
	float factor_ = 1;
};


#endif /* GENETICS_CumulativeValue_H_ */
