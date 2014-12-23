/*
 * text.h
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_TEXT_H_
#define RENDEROPENGL_TEXT_H_

#include "IRenderable.h"
#include <string>

class GLText : public IRenderable{
public:
	GLText(const char * texturePath, int rows, int cols, char firstChar);
	~GLText() override;

	void print(const std::string text, int x, int y, int size);
	void cleanup();

	void render(Viewport* pCrtViewport) override;
	void purgeRenderQueue() override;
};

#endif /* RENDEROPENGL_TEXT_H_ */
