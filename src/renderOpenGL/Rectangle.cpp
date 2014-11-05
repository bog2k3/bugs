/*
 * Rectangle.cpp
 *
 *  Created on: Oct 29, 2014
 *      Author: bog
 */

#include "Rectangle.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "IRenderer.h"
#include "shader.hpp"

bool Rectangle::initialized = false;
std::vector<Rectangle::s_rectangle> Rectangle::buffer;
GLuint Rectangle::shaderProgram = 0;
GLuint Rectangle::indexPos = 0;
GLuint Rectangle::indexColor = 0;
GLuint Rectangle::indexMatViewProj = 0;
IRenderer* Rectangle::renderer = nullptr;

void Rectangle::initialize(IRenderer* renderer) {
	Rectangle::renderer = renderer;
	renderer->registerRenderable(&render);
	shaderProgram = Shaders::createProgram("data/shaders/rect.vert", "data/shaders/rect.frag");
	if (shaderProgram == 0) {
		throw new std::runtime_error("Unable to load rectangle shaders!!");
	}
	indexPos = glGetAttribLocation(shaderProgram, "vPos");
	indexColor = glGetAttribLocation(shaderProgram, "vColor");
	indexMatViewProj = glGetUniformLocation(shaderProgram, "mViewProj");
}
void Rectangle::draw(float center_x, float center_y, float z, float width, float height, float rotation,
		float red, float green, float blue) {
	s_rectangle s;
	float halfW = width * 0.5f;
	float halfH = height * 0.5f;
	float sinT = sin(rotation);
	float cosT = cos(rotation);
	// top-left:
	s.pos[2] = z;
	s.pos[0] = center_x - halfW*cosT - halfH*sinT, s.pos[1] = center_y - halfW*sinT + halfH*cosT;
	s.rgb[0] = red, s.rgb[1] = green, s.rgb[2] = blue;
	buffer.push_back(s);
	// top-right:
	s.pos[0] = center_x + halfW*cosT - halfH*sinT, s.pos[1] = center_y + halfW*sinT + halfH*cosT;
	buffer.push_back(s);
	// bottom-right:
	s.pos[0] = center_x + halfW*cosT + halfH*sinT, s.pos[1] = center_y + halfW*sinT - halfH*cosT;
	buffer.push_back(s);
	// bottom-left:
	s.pos[0] = center_x - halfW*cosT + halfH*sinT, s.pos[1] = center_y - halfW*sinT - halfH*cosT;
	buffer.push_back(s);
	// top-left again:
	buffer.push_back(buffer[buffer.size()-4]);
}
void Rectangle::render() {
	glUniformMatrix4fv(indexMatViewProj, 1, GL_FALSE, glm::value_ptr(renderer->getMatViewProj()));
	glVertexAttribPointer(indexPos, 3, GL_FLOAT, GL_FALSE, sizeof(s_rectangle), &buffer[0].pos);
	glEnableVertexAttribArray(indexPos);
	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, sizeof(s_rectangle), &buffer[0].rgb);
	glEnableVertexAttribArray(indexColor);
	glUseProgram(shaderProgram);

	glDrawArrays(GL_LINE_STRIP, 0, buffer.size());
	buffer.clear();
}
