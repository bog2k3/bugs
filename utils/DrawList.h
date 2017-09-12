/*
 * DrawList.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef DRAWLIST_H_
#define DRAWLIST_H_

#include "drawable.h"
#include <vector>
#include <algorithm>

class DrawList {
public:
	DrawList() {}
	~DrawList() {}

	void add(drawable_wrap w) {
		list_.push_back(w);
	}

	void remove(drawable_wrap w) {
		list_.erase(std::remove_if(list_.begin(), list_.end(), [&w] (const drawable_wrap& x) {
			return x.equal_value(w);
		}), list_.end());
	}

	void draw(RenderContext const& ctx) {
		for (auto &w : list_)
			w.draw(ctx);
	}

private:
	std::vector<drawable_wrap> list_;
};


#endif /* DRAWLIST_H_ */
