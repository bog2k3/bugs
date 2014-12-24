/*
 * Line.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#include "Shape2D.h"

#include "Renderer.h"
#include "Viewport.h"
#include "Camera.h"
#include "shader.hpp"
#include "../math/math2D.h"

#include <stdexcept>
#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLEW_NO_GLU
#include <GL/glew.h>

Shape2D::Shape2D(Renderer* renderer)
	: shaderProgram(0)
	, indexPos(0)
	, indexColor(0)
	, indexMatViewProj(0)
{
	renderer->registerRenderable(this);
	shaderProgram = Shaders::createProgram("data/shaders/rect.vert", "data/shaders/rect.frag");
	if (shaderProgram == 0) {
		throw new std::runtime_error("Unable to load line shaders!!");
	}
	indexPos = glGetAttribLocation(shaderProgram, "vPos");
	indexColor = glGetAttribLocation(shaderProgram, "vColor");
	indexMatViewProj = glGetUniformLocation(shaderProgram, "mViewProj");
}

Shape2D::~Shape2D() {
	glDeleteProgram(shaderProgram);
}

void Shape2D::render(Viewport* vp) {
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(indexMatViewProj, 1, GL_FALSE, glm::value_ptr(vp->getCamera()->getMatViewProj()));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(indexPos, 3, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), &buffer[0].pos);
	glEnableVertexAttribArray(indexPos);
	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), &buffer[0].rgb);
	glEnableVertexAttribArray(indexColor);

	glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
}

void Shape2D::drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec3 rgb) {
	s_lineVertex s;
	// point1:
	s.pos = glm::vec3(point1, z);
	s.rgb = rgb;
	buffer.push_back(s);
	indices.push_back(buffer.size()-1);
	// point2:
	s.pos = glm::vec3(point2, z);
	buffer.push_back(s);
	indices.push_back(buffer.size()-1);
}

void Shape2D::drawLineList(glm::vec2* verts, int nVerts, float z, glm::vec3 rgb) {
	s_lineVertex s;
	for (int i=0; i<nVerts; i++) {
		s.pos = glm::vec3(verts[i], z);
		s.rgb = rgb;
		buffer.push_back(s);
		indices.push_back(buffer.size()-1);
	}
}

void Shape2D::drawLineStrip(glm::vec2* verts, int nVerts, float z, glm::vec3 rgb) {
	s_lineVertex s;
	for (int i=0; i<nVerts; i++) {
		s.pos = glm::vec3(verts[i], z);
		s.rgb = rgb;
		buffer.push_back(s);
		indices.push_back(buffer.size()-1);
		if (i > 0 && i < nVerts-1)
			indices.push_back(buffer.size()-1);
	}
}

inline void Shape2D::transformViewportToWorld(glm::vec2* vIn, glm::vec2* vOut, int n, Viewport const& vp) {
	for (int i=0; i<n; i++)
		vOut[i] = vp.unproject(vIn[i]);
}

void Shape2D::drawLineStripViewport(glm::vec2* verts, int nVerts, float z, glm::vec3 rgb, Viewport const& vp) {
	glm::vec2* vertsWorld = new glm::vec2[nVerts];
	transformViewportToWorld(verts, vertsWorld, nVerts, vp);
	drawLineStrip(vertsWorld, nVerts, z, rgb);
	delete [] vertsWorld;
}

void Shape2D::drawPolygon(glm::vec2 *verts, int nVerts, float z, glm::vec3 rgb) {
	s_lineVertex sVertex;
	sVertex.rgb = rgb;
	for (int i=0; i<nVerts; i++) {
		sVertex.pos = glm::vec3(verts[i], z);
		buffer.push_back(sVertex);
		indices.push_back(buffer.size()-1);
		if (i > 0)
			indices.push_back(buffer.size()-1);
	}
	indices.push_back(buffer.size()-nVerts);
}

void Shape2D::drawRectangle(glm::vec2 pos, float z, glm::vec2 size, float rotation, glm::vec3 rgb) {
	float halfW = size.x * 0.5f;
	float halfH = size.y * 0.5f;
	s_lineVertex sVertex;
	sVertex.rgb = rgb;
	// top left
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(-halfW, halfH), rotation) + pos, z);
	buffer.push_back(sVertex);
	indices.push_back(buffer.size()-1);
	// top right
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(halfW, halfH), rotation) + pos, z);
	buffer.push_back(sVertex);
	indices.push_back(buffer.size()-1);
	indices.push_back(buffer.size()-1);
	// bottom right
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(halfW, -halfH), rotation) + pos, z);
	buffer.push_back(sVertex);
	indices.push_back(buffer.size()-1);
	indices.push_back(buffer.size()-1);
	// bottom left
	sVertex.pos = glm::vec3(glm::rotate(glm::vec2(-halfW, -halfH), rotation) + pos, z);
	buffer.push_back(sVertex);
	indices.push_back(buffer.size()-1);
	indices.push_back(buffer.size()-1);
	// top left again
	indices.push_back(buffer.size()-4);
}

void Shape2D::purgeRenderQueue() {
	buffer.clear();
	indices.clear();
}

void Shape2D::drawCircle(glm::vec2 pos, float radius, float z, int nSides, glm::vec3 rgb) {
	// make a polygon out of the circle
	float phiStep = 2 * PI * 1.f / nSides;
	glm::vec2 *v = new glm::vec2[nSides];
	float phi = 0;
	for (int i=0; i<nSides; i++) {
		v[i].x = radius * cosf(phi) + pos.x;
		v[i].y = radius * sinf(phi) + pos.y;
		phi += phiStep;
	}
	drawPolygon(v, nSides, z, rgb);
	delete [] v;
}
