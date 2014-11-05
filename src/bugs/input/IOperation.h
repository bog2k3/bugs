/*
 * IOperation.h
 *
 *  Created on: Nov 4, 2014
 *      Author: bog
 */

#ifndef INPUT_IOPERATION_H_
#define INPUT_IOPERATION_H_

class InputEvent {
public:
	enum EVENT_TYPE {
		EV_MOUSE_MOVED,
		EV_MOUSE_DOWN,
		EV_MOUSE_UP,
		EV_MOUSE_SCROLL,
		EV_KEY_DOWN,
		EV_KEY_UP,
	} const type;
	float x, y;
	float dx, dy, dz;
	enum MOUSE_BUTTON {
		MB_LEFT,
		MB_MIDDLE,
		MB_RIGHT
	} mouseButton;
	int key;

	InputEvent(EVENT_TYPE type, float x, float y, float dx, float dy, int dz, MOUSE_BUTTON button, int key)
		: type(type), x(x), y(y), dx(dx), dy(dy), dz(dz), mouseButton(button), key(key), mIsConsumed(false)
	{
	}

	bool isConsumed() {
		return mIsConsumed;
	}
	void consume() {
		mIsConsumed = true;
	}
private:
	bool mIsConsumed;
};

class IOperation {
public:
	virtual ~IOperation() {
	}

	virtual void handleInput(InputEvent& ev) = 0;
	virtual void update(float dt) {}
};

#endif /* INPUT_IOPERATION_H_ */
