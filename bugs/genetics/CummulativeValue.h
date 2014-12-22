/*
 * CummulativeValue.h
 *
 *  Created on: Dec 22, 2014
 *      Author: bogdan
 */

#ifndef GENETICS_CUMMULATIVEVALUE_H_
#define GENETICS_CUMMULATIVEVALUE_H_

#include <cassert>

struct CummulativeValue {
	CummulativeValue(float initial) : value_(initial), cachedValue_(value_), cacheUpdated_(true), n_(1), factor_(1.f) {}
	operator float() {
		if (!cacheUpdated_)
			updateCache();
		return cachedValue_;
	}
	void changeAbs(float change) {
		value_ += change;
		n_++;
		cacheUpdated_ = false;
	}
	void changeRel(float factor) {
		factor_ *= factor;
		cacheUpdated_ = false;
	}
private:
	void updateCache() {
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
