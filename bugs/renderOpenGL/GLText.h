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
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Renderer;

class GLText : public IRenderable {
public:
	GLText(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar, int defaultSize);
	~GLText() override;

	// z is between [0..100] (bottom to top)
	void print(const std::string text, int x, int y, int z, int size, glm::vec3 const& color);
	// z is between [0..100] (bottom to top)
	void print(const std::string text, int x, int y, int z, int size, glm::vec4 const& color);

	void render(Viewport* pCrtViewport) override;
	void purgeRenderQueue() override;

private:
	unsigned textureID;             	// Texture containing the font
	unsigned vertexBufferID;         	// Buffer containing the vertices
	unsigned UVBufferID;             	// UVs
	unsigned colorBufferID;					// vertex colors
	unsigned shaderID;               	// Program used to disaply the text
	unsigned vertexPosition_screenspaceID; 	// Location of the program's "vertexPosition_screenspace" attribute
	unsigned vertexUVID;                   	// Location of the program's "vertexUV" attribute
	unsigned vertexColorID;
	unsigned viewportHalfSizeID;
	unsigned u_textureID;              	// Location of the program's texture attribute
	int rows, cols, firstChar;
	float cellRatio; 						// cellWeight / cellHidth
	int defaultSize_;					// text size from the texture
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec4> colors;
};

#endif /* RENDEROPENGL_GLTEXT_H_ */
