/*
 * ioModif.h
 *
 *  Created on: Dec 9, 2016
 *      Author: bog
 */

#ifndef UTILS_IOMODIF_H_
#define UTILS_IOMODIF_H_

#include <ostream>

namespace ioModif {
enum Code {
	RESET = 0,

	BOLD = 1,
	ITALIC = 3,
	DARK = 2,
	UNDERLINE = 4,
	SELECTED = 7,
	STRIKETHROUGH = 9,

	NO_BOLD = 21,
	NO_ITALIC = 23,
	NO_DARK = 22,
	NO_UNDERLINE = 24,
	NO_SELECTED = 27,
	NO_STRIKETHROUGH = 29,

	FG_BLACK = 30,
	FG_RED = 31,
	FG_GREEN = 32,
	FG_ORANGE = 33,
	FG_BLUE = 34,
	FG_MAGENTA = 35,
	FG_CYAN = 36,
	FG_GRAY = 37,
	FG_WHITE = 38,
	FG_DEFAULT = 39,
	FG_DARK_GRAY = 90,
	FG_LIGHT_RED = 91,
	FG_LIGHT_GREEN = 92,
	FG_LIGHT_YELLOW = 93,
	FG_LIGHT_BLUE = 94,
	FG_LIGHT_MAGENTA = 95,
	FG_LIGHT_CYAN = 96,

	BG_BLACK = 40,
	BG_RED = 41,
	BG_GREEN = 42,
	BG_ORANGE = 43,
	BG_LIGHT_BLUE = 44,
	BG_MAGENTA = 45,
	BG_CYAN = 46,
	BG_GRAY = 47,
	BG_DEFAULT = 49,
};

using byte = unsigned char;

struct FG_RGB {
	int r, g, b;
	FG_RGB(byte r, byte g, byte b)
		: r(r), g(g), b(b) {
	}
	FG_RGB operator * (double mx) {
		return {byte(r*mx), byte(g*mx), byte(b*mx)};
	}
};

struct BG_RGB {
	int r, g, b;
	BG_RGB(byte r, byte g, byte b)
		: r(r), g(g), b(b) {
	}
	BG_RGB operator * (double mx) {
		return {byte(r*mx), byte(g*mx), byte(b*mx)};
	}
};

std::ostream& operator<<(std::ostream& os, FG_RGB fg) {
	return os << "\033[38;2;" << fg.r << ";" << fg.g << ";" << fg.b << "m";
}

std::ostream& operator<<(std::ostream& os, BG_RGB bg) {
	return os << "\033[48;2;" << bg.r << ";" << bg.g << ";" << bg.b << "m";
}

std::ostream& operator<<(std::ostream& os, Code code) {
	return os << "\033[" << static_cast<int>(code) << "m";
}
}

#endif /* UTILS_IOMODIF_H_ */
