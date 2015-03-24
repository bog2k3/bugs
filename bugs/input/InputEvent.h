/*
 * InputEvent.h
 *
 *  Created on: Nov 7, 2014
 *      Author: bog
 */

#ifndef INPUT_INPUTEVENT_H_
#define INPUT_INPUTEVENT_H_

class InputEvent {
public:
	enum EVENT_TYPE {
		EV_MOUSE_MOVED,
		EV_MOUSE_DOWN,
		EV_MOUSE_UP,
		EV_MOUSE_SCROLL,
		EV_KEY_DOWN,
		EV_KEY_UP,
		EV_KEY_CHAR,
	} const type;
	float x, y;
	float dx, dy, dz;
	enum MOUSE_BUTTON {
		MB_NONE=-1,
		MB_LEFT=0,
		MB_RIGHT,
		MB_MIDDLE,
	} mouseButton;
	int key;
	char ch;

	InputEvent(EVENT_TYPE type, float x, float y, float dx, float dy, int dz, MOUSE_BUTTON button, int key, char ch)
		: type(type), x(x), y(y), dx(dx), dy(dy), dz(dz), mouseButton(button), key(key), ch(ch)
	{
	}

	bool isConsumed() {
		return mIsConsumed;
	}
	void consume() {
		mIsConsumed = true;
	}
private:
	bool mIsConsumed = false;
};

#endif /* INPUT_INPUTEVENT_H_ */
