/*
 * text.h
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_GLTEXT_H_
#define RENDEROPENGL_GLTEXT_H_

#include "IRenderable.h"
#include <string>

class Renderer;

class GLText : public IRenderable{
public:
	GLText(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar);
	~GLText() override;

	void print(const std::string text, int x, int y, int size);

	void render(Viewport* pCrtViewport) override;
	void purgeRenderQueue() override;

private:
	unsigned Text2DTextureID;             	// Texture containing the font
	unsigned Text2DVertexBufferID;         	// Buffer containing the vertices
	unsigned Text2DUVBufferID;             	// UVs
	unsigned Text2DShaderID;               	// Program used to disaply the text
	unsigned vertexPosition_screenspaceID; 	// Location of the program's "vertexPosition_screenspace" attribute
	unsigned vertexUVID;                   	// Location of the program's "vertexUV" attribute
	unsigned viewportHalfSizeID;
	unsigned Text2DUniformID;              	// Location of the program's texture attribute
	int rows, cols, firstChar;
	float cellRatio; 						// cellWeight / cellHidth
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;
};

#endif /* RENDEROPENGL_GLTEXT_H_ */
