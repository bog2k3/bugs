/*
 * constants.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_CONSTANTS_H_
#define GUI_CONSTANTS_H_

enum class MouseButtons {
	Left = 0,
	Right,
	Middle
};

// bit flags, can be combined together:
enum class Anchors {
	Left = 1,
	Right = 2,
	Top = 4,
	Bottom = 8,
};
constexpr Anchors operator | (Anchors a1, Anchors a2) {
	return (Anchors)((int)a1 | (int)a2);
}

#endif /* GUI_CONSTANTS_H_ */
