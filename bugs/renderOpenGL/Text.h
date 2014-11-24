/*
 * text.h
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_TEXT_H_
#define RENDEROPENGL_TEXT_H_

#include <string>

class GLText {
public:
	static void initialize(const char * texturePath, int rows, int cols, char firstChar);
	static void print(const std::string text, int x, int y, int size);
	static void cleanup();
};

#endif /* RENDEROPENGL_TEXT_H_ */
