/*
 * Prototype.h
 *
 *  Created on: Oct 29, 2017
 *      Author: bog
 */

#ifndef PROTOTYPE_H_
#define PROTOTYPE_H_

/*
 * Wrapper class for prototyping new features
 */

class RenderContext;

class Prototype {
public:
	void enable(bool enable) { enabled_ = enable; }

	void initialize();
	void terminate();
	void draw(RenderContext const& ctx);
	void update(float dt);

	void onKeyDown(int key);
	void onKeyUp(int key);

private:
	bool enabled_ = false;
};

#endif /* PROTOTYPE_H_ */
