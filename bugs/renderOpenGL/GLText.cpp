/*
 * text.cpp
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#include "shader.hpp"
#include "TextureLoader.h"
#include "Renderer.h"
#include "Viewport.h"

#include <vector>
#include <GL/glew.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLText.h"
using namespace glm;

GLText::GLText(Renderer* renderer, const char * texturePath, int rows, int cols, char firstChar, int defaultSize)
	: rows(rows), cols(cols), firstChar(firstChar), defaultSize_(defaultSize)
{
	renderer->registerRenderable(this);
	cellRatio = (float)rows/cols;
	// Initialize texture
	textureID = TextureLoader::loadFromPNG(texturePath, nullptr, nullptr);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Initialize VBO
	glGenBuffers(1, &vertexBufferID);
	glGenBuffers(1, &UVBufferID);
	glGenBuffers(1, &colorBufferID);

	// Initialize Shader
	shaderID = Shaders::createProgram("data/shaders/text.vert", "data/shaders/text.frag");

	// Get a handle for our buffers
	vertexPosition_screenspaceID = glGetAttribLocation(shaderID, "vertexPosition_screenspace");
	vertexUVID = glGetAttribLocation(shaderID, "vertexUV");
	vertexColorID = glGetAttribLocation(shaderID, "vertexColor");

	// Initialize uniforms' IDs
	viewportHalfSizeID = glGetUniformLocation(shaderID, "viewportHalfSize");
	u_textureID = glGetUniformLocation( shaderID, "myTextureSampler" );
}

GLText::~GLText() {
	// Delete buffers
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteBuffers(1, &UVBufferID);
	glDeleteBuffers(1, &colorBufferID);

	// Delete texture
	glDeleteTextures(1, &textureID);

	// Delete shader
	glDeleteProgram(shaderID);
}

glm::vec2 GLText::getTextRect(const std::string& text, int fontSize) {
	unsigned int length = text.length();
	float xSize = fontSize*cellRatio;
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

void GLText::print(const std::string &text, int x, int y, int z, int size, glm::vec3 const& color) {
	print(text, x, y, z, size, glm::vec4(color, 1));
}

void GLText::print(const std::string &text, int x, int y, int z, int size, glm::vec4 const& color) {
	unsigned int length = text.length();
	float xSize = size*cellRatio;
	glm::vec4 altColor = color;
	if (size < defaultSize_)
		altColor.a *= (float)defaultSize_ / size;
	float zf = -z * 0.01f;

	// Fill buffers
	int initialX = x;
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

		vertices.push_back(vertex_up_left   );
		vertices.push_back(vertex_down_left );
		vertices.push_back(vertex_up_right  );

		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);

		float uv_x = ((character - firstChar) % cols) / (float)cols;
		float uv_y = 1.f - ((character - firstChar) / cols) / (float)rows;

		glm::vec2 uv_up_left    = glm::vec2( uv_x          , uv_y );
		glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/cols, uv_y );
		glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/cols, uv_y - 1.0f/rows );
		glm::vec2 uv_down_left  = glm::vec2( uv_x          , uv_y - 1.0f/rows );
		UVs.push_back(uv_up_left   );
		UVs.push_back(uv_down_left );
		UVs.push_back(uv_up_right  );

		UVs.push_back(uv_down_right);
		UVs.push_back(uv_up_right);
		UVs.push_back(uv_down_left);

		for (int ci=0; ci<6; ci++)
			colors.push_back(altColor);
	}
}

void GLText::render(Viewport* pCrtViewport) {
	// Bind shader
	glUseProgram(shaderID);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(u_textureID, 0);

	vec2 halfVP(pCrtViewport->getWidth() / 2, pCrtViewport->getHeight() / 2);
	glUniform2fv(viewportHalfSizeID, 1, &halfVP[0]);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(vertexPosition_screenspaceID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPosition_screenspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(vertexUVID);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(UVs[0]), &UVs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 3rd attribute buffer : vertex colors
	glEnableVertexAttribArray(vertexColorID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(vertexColorID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw call
	glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(vertexPosition_screenspaceID);
	glDisableVertexAttribArray(vertexUVID);
	glDisableVertexAttribArray(vertexColorID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLText::purgeRenderQueue() {
	vertices.clear();
	UVs.clear();
	colors.clear();
}
