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
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Renderer;
class ViewportCoord;

class GLText : public IRenderable {
public:
	static GLText* get();
	virtual ~GLText() override;
	static void init(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar, int defaultSize);

	// restricts rendering of following commands to only one viewport
	void setViewportFilter(std::string viewportName);
	// resets the viewport filter
	void resetViewportFilter();

	// z is between [0..100] (bottom to top)
	void print(const std::string &text, ViewportCoord pos, int z, int size, glm::vec3 const& color);
	// z is between [0..100] (bottom to top)
	void print(const std::string &text, ViewportCoord pos, int z, int size, glm::vec4 const& color);

	glm::vec2 getTextRect(std::string const& text, int fontSize);

protected:
	void render(Viewport* pCrtViewport) override;
	void purgeRenderQueue() override;
	void unload() override;
	GLText(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar, int defaultSize);

private:
	unsigned textureID_;             	// Texture containing the font
	unsigned vertexBufferID_;         	// Buffer containing the vertices
	unsigned UVBufferID_;             	// UVs
	unsigned colorBufferID_;					// vertex colors
	unsigned shaderID_;               	// Program used to disaply the text
	unsigned vertexPosition_screenspaceID_; 	// Location of the program's "vertexPosition_screenspace" attribute
	unsigned vertexUVID_;                   	// Location of the program's "vertexUV" attribute
	unsigned vertexColorID_;
	unsigned viewportHalfSizeID_;
	unsigned translationID_;
	unsigned u_textureID_;              	// Location of the program's texture attribute
	int rows_, cols_, firstChar_;
	float cellRatio_; 						// cellWeight / cellHidth
	int defaultSize_;					// text size from the texture
	std::vector<glm::vec3> vertices_;	// these are relative to item's position (below)
	std::vector<glm::vec2> UVs_;
	std::vector<glm::vec4> colors_;
	std::vector<ViewportCoord> itemPositions_;
	std::string viewportFilter_;
	std::vector<std::string> viewportFilters_;
	std::vector<int> verticesPerItem_;
};

#endif /* RENDEROPENGL_GLTEXT_H_ */
