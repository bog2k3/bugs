/*
 * text.cpp
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#include <vector>
#include <GL/glew.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Text.h"
using namespace glm;

#include "shader.hpp"
#include "TextureLoader.h"

unsigned Text2DTextureID;              // Texture containing the font
unsigned Text2DVertexBufferID;         // Buffer containing the vertices
unsigned Text2DUVBufferID;             //                       UVs
unsigned Text2DShaderID;               // Program used to disaply the text
unsigned vertexPosition_screenspaceID; // Location of the program's "vertexPosition_screenspace" attribute
unsigned vertexUVID;                   // Location of the program's "vertexUV" attribute
unsigned viewportHalfSizeID;
unsigned Text2DUniformID;              // Location of the program's texture attribute
int rows, cols, first_char;
float cellRatio; // cellWeight / cellHidth

void GLText::initialize(const char * texturePath, int rows_, int cols_, char firstChar_) {
	rows = rows_, cols = cols_, first_char = firstChar_;
	cellRatio = (float)rows/cols;
	// Initialize texture
	Text2DTextureID = TextureLoader::loadFromPNG(texturePath, nullptr, nullptr);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Initialize VBO
	glGenBuffers(1, &Text2DVertexBufferID);
	glGenBuffers(1, &Text2DUVBufferID);

	// Initialize Shader
	Text2DShaderID = Shaders::createProgram("data/shaders/text.vert", "data/shaders/text.frag");

	// Get a handle for our buffers
	vertexPosition_screenspaceID = glGetAttribLocation(Text2DShaderID, "vertexPosition_screenspace");
	vertexUVID = glGetAttribLocation(Text2DShaderID, "vertexUV");

	// Initialize uniforms' IDs
	viewportHalfSizeID = glGetUniformLocation(Text2DShaderID, "viewportHalfSize");
	Text2DUniformID = glGetUniformLocation( Text2DShaderID, "myTextureSampler" );
}

void GLText::print(const std::string text, int x, int y, int size) {
	unsigned int length = text.length();
	float xSize = size*cellRatio;

	// Fill buffers
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;
	int initialX = x;
	for ( unsigned int i=0 ; i<length ; i++ ) {
		char character = text[i];
		if (character == '\t') {
			x += 4 * xSize;
			continue;
		}
		if (character == '\n') {
			y -= size * 0.75f;
			x = initialX;
			continue;
		}

		glm::vec2 vertex_up_left    = glm::vec2(x      , y+size );
		glm::vec2 vertex_up_right   = glm::vec2(x+xSize, y+size );
		glm::vec2 vertex_down_right = glm::vec2(x+xSize, y      );
		glm::vec2 vertex_down_left  = glm::vec2(x      , y      );

		x += xSize;

		vertices.push_back(vertex_up_left   );
		vertices.push_back(vertex_down_left );
		vertices.push_back(vertex_up_right  );

		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);

		float uv_x = ((character - first_char) % cols) / (float)cols;
		float uv_y = 1.f - ((character - first_char) / cols) / (float)rows;

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
	}
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);

	// Bind shader
	glUseProgram(Text2DShaderID);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(Text2DUniformID, 0);

	vec2 halfVP(400,300);
	glUniform2fv(viewportHalfSizeID, 1, &halfVP[0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(vertexPosition_screenspaceID);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glVertexAttribPointer(vertexPosition_screenspaceID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(vertexUVID);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw call
	glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(vertexPosition_screenspaceID);
	glDisableVertexAttribArray(vertexUVID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLText::cleanup() {
	// Delete buffers
	glDeleteBuffers(1, &Text2DVertexBufferID);
	glDeleteBuffers(1, &Text2DUVBufferID);

	// Delete texture
	glDeleteTextures(1, &Text2DTextureID);

	// Delete shader
	glDeleteProgram(Text2DShaderID);
}
