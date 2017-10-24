/*
 * Shape3D.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: bogdan
 */

#include "Shape3D.h"

#include "Renderer.h"
#include "Viewport.h"
#include "Camera.h"
#include "shader.hpp"
#include "../math/math3D.h"
#include "../utils/log.h"

#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

#define GLEW_NO_GLU
#include <GL/glew.h>

static Shape3D* instance = nullptr;

void Shape3D::init(Renderer* renderer) {
	instance = new Shape3D(renderer);
}

Shape3D* Shape3D::get() {
	assertDbg(instance && "must be initialized first!");
	return instance;
}

Shape3D::Shape3D(Renderer* renderer)
	: lineShaderProgram_(0)
	, indexPos_(0)
	, indexColor_(0)
	, indexMatProjView_(0)
{
	renderer->registerRenderable(this);
	lineShaderProgram_ = Shaders::createProgram("data/shaders/shape3d.vert", "data/shaders/shape3d.frag");
	if (lineShaderProgram_ == 0) {
		throw std::runtime_error("Unable to load shape3D shaders!!");
	}
	indexPos_ = glGetAttribLocation(lineShaderProgram_, "vPos");
	indexColor_ = glGetAttribLocation(lineShaderProgram_, "vColor");
	indexMatProjView_ = glGetUniformLocation(lineShaderProgram_, "mProjView");
}

Shape3D::~Shape3D() {
	glDeleteProgram(lineShaderProgram_);
}

void Shape3D::unload() {
	delete instance;
	instance = nullptr;
}

void Shape3D::render(Viewport* vp) {
	glUseProgram(lineShaderProgram_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendEquation(GL_BLEND_EQUATION_ALPHA);

	glUniformMatrix4fv(indexMatProjView_, 1, GL_FALSE, glm::value_ptr(vp->camera()->matProjView()));
	glEnableVertexAttribArray(indexPos_);
	glEnableVertexAttribArray(indexColor_);

	// render world-space line primitives:
	glVertexAttribPointer(indexPos_, 3, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), &buffer_[0].pos);
	glVertexAttribPointer(indexColor_, 4, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), &buffer_[0].rgba);
	glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, &indices_[0]);

	glDisable(GL_BLEND);
}

void Shape3D::purgeRenderQueue() {
	buffer_.clear();
	indices_.clear();
}

void Shape3D::drawLine(glm::vec3 point1, glm::vec3 point2, glm::vec3 rgb) {
	drawLine(point1, point2, glm::vec4(rgb, 1));
}

void Shape3D::drawLine(glm::vec3 point1, glm::vec3 point2, glm::vec4 rgba) {
	buffer_.push_back({point1, rgba});
	indices_.push_back(buffer_.size()-1);
	buffer_.push_back({point2, rgba});
	indices_.push_back(buffer_.size()-1);
}

void Shape3D::drawLineList(glm::vec3 verts[], int nVerts, glm::vec3 rgb) {
	drawLineList(verts, nVerts, glm::vec4(rgb, 1));
}

void Shape3D::drawLineList(glm::vec3 verts[], int nVerts, glm::vec4 rgba) {
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], rgba});
		indices_.push_back(buffer_.size()-1);
	}
}

void Shape3D::drawLineStrip(glm::vec3 verts[], int nVerts, glm::vec3 rgb) {
	drawLineStrip(verts, nVerts, glm::vec4(rgb, 1));
}

void Shape3D::drawLineStrip(glm::vec3 verts[], int nVerts, glm::vec4 rgba) {
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], rgba});
		indices_.push_back(buffer_.size()-1);
		if (i > 0 && i < nVerts-1)
			indices_.push_back(buffer_.size()-1);
	}
}

void Shape3D::drawPolygon(glm::vec3 verts[], int nVerts, glm::vec3 rgb) {
	drawPolygon(verts, nVerts, glm::vec4(rgb, 1));
}

void Shape3D::drawPolygon(glm::vec3 verts[], int nVerts, glm::vec4 rgba) {
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], rgba});
		indices_.push_back(buffer_.size()-1);
		if (i > 0) {
			indices_.push_back(buffer_.size()-1);
		}
	}
	indices_.push_back(buffer_.size()-nVerts);
}

void Shape3D::drawRectangleXOY(glm::vec2 pos, glm::vec2 size, glm::vec3 rgb) {
	drawRectangleXOY(pos, size, glm::vec4(rgb, 1));
}

void Shape3D::drawRectangleXOY(glm::vec2 pos, glm::vec2 size, glm::vec4 rgba) {
	glm::vec3 sz = {size, 0};
	glm::vec3 pos3 = {pos, 0};
	glm::vec3 coords[] {
		pos3,
		pos3 + sz.y,
		pos3 + sz,
		pos3 + sz.x
	};
	drawPolygon(coords, 4, rgba);
}

void Shape3D::drawRectangleXOYCentered(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec3 rgb) {
	drawRectangleXOYCentered(pos, size, rotation, glm::vec4(rgb, 1));
}

void Shape3D::drawRectangleXOYCentered(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec4 rgba) {
	float halfW = size.x * 0.5f;
	float halfH = size.y * 0.5f;
	s_lineVertex sVertex;
	sVertex.rgba = rgba;
	// top left
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(-halfW, halfH), rotation) + pos, 0);
	buffer_.push_back(sVertex);
	indices_.push_back(buffer_.size()-1);
	// top right
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(halfW, halfH), rotation) + pos, 0);
	buffer_.push_back(sVertex);
	indices_.push_back(buffer_.size()-1);
	indices_.push_back(buffer_.size()-1);
	// bottom right
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(halfW, -halfH), rotation) + pos, 0);
	buffer_.push_back(sVertex);
	indices_.push_back(buffer_.size()-1);
	indices_.push_back(buffer_.size()-1);
	// bottom left
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(-halfW, -halfH), rotation) + pos, 0);
	buffer_.push_back(sVertex);
	indices_.push_back(buffer_.size()-1);
	indices_.push_back(buffer_.size()-1);
	// top left again
	indices_.push_back(buffer_.size()-4);
}

void Shape3D::drawCircleXOY(glm::vec2 pos, float radius, int nSides, glm::vec3 rgb) {
	drawCircleXOY(pos, radius, nSides, glm::vec4(rgb, 1));
}

void Shape3D::drawCircleXOY(glm::vec2 pos, float radius, int nSides, glm::vec4 rgba) {
	// make a polygon out of the circle
	float phiStep = 2 * PI * 1.f / nSides;
	glm::vec3 *v = new glm::vec3[nSides];
	float phi = 0;
	for (int i=0; i<nSides; i++) {
		v[i] = glm::vec3(pos, 0) + glm::vec3{cosf(phi) * radius, sinf(phi) * radius, 0};
		phi += phiStep;
	}
	drawPolygon(v, nSides, rgba);
	delete [] v;
}
