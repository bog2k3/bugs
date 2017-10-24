/*
 * text.cpp
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#include "GLText.h"
#include "Renderer.h"
#include "Viewport.h"
#include "TextureLoader.h"
#include "shader.hpp"
#include "ViewportCoord.h"

#include "../utils/assert.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

#include <vector>

using namespace glm;

//#define DISABLE_MIPMAPS

static GLText* instance = nullptr;

void GLText::init(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar, int defaultSize) {
	instance = new GLText(renderer, texturePath, rows, cols, firstChar, defaultSize);
}

GLText* GLText::get() {
	assertDbg(instance && "must be initialized first!");
	return instance;
}

void GLText::unload() {
	delete instance;
	instance = nullptr;
}

GLText::GLText(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar, int defaultSize)
	: rows_(rows), cols_(cols), firstChar_(firstChar), defaultSize_(defaultSize)
{
	renderer->registerRenderable(this);
	cellRatio_ = (float)rows/cols;
	// Initialize texture
	textureID_ = TextureLoader::loadFromPNG(texturePath, nullptr, nullptr);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID_);
#ifndef DISABLE_MIPMAPS
	glGenerateMipmap(GL_TEXTURE_2D);
#endif

	// Initialize VBO
	glGenBuffers(1, &vertexBufferID_);
	glGenBuffers(1, &UVBufferID_);
	glGenBuffers(1, &colorBufferID_);

	// Initialize Shader
	shaderID_ = Shaders::createProgram("data/shaders/text.vert", "data/shaders/text.frag");
	if (!shaderID_)
		throw std::runtime_error("Could not load text shaders!");

	// Get a handle for our buffers
	vertexPosition_screenspaceID_ = glGetAttribLocation(shaderID_, "vertexPosition_screenspace");
	vertexUVID_ = glGetAttribLocation(shaderID_, "vertexUV");
	vertexColorID_ = glGetAttribLocation(shaderID_, "vertexColor");

	// Initialize uniforms' IDs
	viewportHalfSizeID_ = glGetUniformLocation(shaderID_, "viewportHalfSize");
	translationID_ = glGetUniformLocation(shaderID_, "translation");
	u_textureID_ = glGetUniformLocation( shaderID_, "myTextureSampler" );
}

GLText::~GLText() {
	// Delete buffers
	glDeleteBuffers(1, &vertexBufferID_);
	glDeleteBuffers(1, &UVBufferID_);
	glDeleteBuffers(1, &colorBufferID_);

	// Delete texture
	glDeleteTextures(1, &textureID_);

	// Delete shader
	glDeleteProgram(shaderID_);
}

void GLText::setViewportFilter(std::string viewportName) {
	viewportFilter_ = viewportName;
}

void GLText::resetViewportFilter() {
	viewportFilter_ = "";
}

glm::vec2 GLText::getTextRect(const std::string& text, int fontSize) {
	unsigned int length = text.length();
	float xSize = fontSize*cellRatio_;
	int x = 0;
	int y = 0;
	int lineX = 0;
	int lineH = fontSize * 0.75f;
	for ( unsigned int i=0 ; i<length ; i++ ) {
		char character = text[i];
		if (character == '\t') {
			lineX += 4 * xSize;
			continue;
		}
		if (character == '\n') {
			y += lineH;
			if (lineX > x)
				x = lineX;
			lineX = 0;
			continue;
		}
		lineX += xSize;
	}
	if (lineX > x)
		x = lineX;
	return glm::vec2(x, y+lineH);
}

void GLText::print(const std::string &text, ViewportCoord pos, int z, int size, glm::vec3 const& color) {
	print(text, pos, z, size, glm::vec4(color, 1));
}

void GLText::print(const std::string &text, ViewportCoord pos, int z, int size, glm::vec4 const& color) {
	unsigned int length = text.length();
	float xSize = size*cellRatio_;
	glm::vec4 altColor = color;
	if (size < defaultSize_)
		altColor.a *= (float)defaultSize_ / size;
	float zf = -z * 0.01f;

	// Fill buffers
	itemPositions_.push_back(pos);
	viewportFilters_.push_back(viewportFilter_);
	verticesPerItem_.push_back(length * 6);
	int x = 0;
	int y = 0;
	int initialX = 0;
	for ( unsigned int i=0 ; i<length ; i++ ) {
		char character = text[i];
		if (character == '\t') {
			x += 4 * xSize;
			continue;
		}
		if (character == '\n') {
			y += size * 0.75f;
			x = initialX;
			continue;
		}

		glm::vec3 vertex_up_left    = glm::vec3(x      , y-size, zf);
		glm::vec3 vertex_up_right   = glm::vec3(x+xSize, y-size, zf);
		glm::vec3 vertex_down_right = glm::vec3(x+xSize, y,      zf);
		glm::vec3 vertex_down_left  = glm::vec3(x      , y,      zf);

		x += xSize;

		vertices_.push_back(vertex_up_left   );
		vertices_.push_back(vertex_down_left );
		vertices_.push_back(vertex_up_right  );

		vertices_.push_back(vertex_down_right);
		vertices_.push_back(vertex_up_right);
		vertices_.push_back(vertex_down_left);

		float uv_x = ((character - firstChar_) % cols_) / (float)cols_;
		float uv_y = 1.f - ((character - firstChar_) / cols_) / (float)rows_;

		glm::vec2 uv_up_left    = glm::vec2( uv_x          , uv_y );
		glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/cols_, uv_y );
		glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/cols_, uv_y - 1.0f/rows_ );
		glm::vec2 uv_down_left  = glm::vec2( uv_x          , uv_y - 1.0f/rows_ );
		UVs_.push_back(uv_up_left   );
		UVs_.push_back(uv_down_left );
		UVs_.push_back(uv_up_right  );

		UVs_.push_back(uv_down_right);
		UVs_.push_back(uv_up_right);
		UVs_.push_back(uv_down_left);

		for (int ci=0; ci<6; ci++)
			colors_.push_back(altColor);
	}
}

void GLText::render(Viewport* pCrtViewport) {
	// Bind shader
	glUseProgram(shaderID_);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(u_textureID_, 0);

	vec2 halfVP(pCrtViewport->width() / 2, pCrtViewport->height() / 2);
	glUniform2fv(viewportHalfSizeID_, 1, &halfVP[0]);

	// 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_[0]), &vertices_[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosition_screenspaceID_);
	glVertexAttribPointer(vertexPosition_screenspaceID_, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID_);
	glBufferData(GL_ARRAY_BUFFER, UVs_.size() * sizeof(UVs_[0]), &UVs_[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexUVID_);
	glVertexAttribPointer(vertexUVID_, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 3rd attribute buffer : vertex colors
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID_);
	glBufferData(GL_ARRAY_BUFFER, colors_.size() * sizeof(colors_[0]), &colors_[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColorID_);
	glVertexAttribPointer(vertexColorID_, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// do the drawing:
	uint offset = 0;
	for (uint i=0; i<itemPositions_.size(); i++) {
		if (viewportFilters_[i].empty() || viewportFilters_[i] == pCrtViewport->name()) {
			glm::vec2 translate(itemPositions_[i].x(pCrtViewport), itemPositions_[i].y(pCrtViewport));
			glUniform2fv(translationID_, 1, &translate[0]);
			glDrawArrays(GL_TRIANGLES, offset, verticesPerItem_[i] );
		}
		offset += verticesPerItem_[i];
	}

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(vertexPosition_screenspaceID_);
	glDisableVertexAttribArray(vertexUVID_);
	glDisableVertexAttribArray(vertexColorID_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLText::purgeRenderQueue() {
	vertices_.clear();
	UVs_.clear();
	colors_.clear();
	itemPositions_.clear();
	verticesPerItem_.clear();
}
