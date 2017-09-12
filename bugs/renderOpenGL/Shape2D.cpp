/*
 * Shape2D.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#include "Shape2D.h"

#include "Renderer.h"
#include "Viewport.h"
#include "Camera.h"
#include "shader.hpp"
#include "../math/math3D.h"
#include "../utils/log.h"

#include <stdexcept>
#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLEW_NO_GLU
#include <GL/glew.h>

static Shape2D* instance = nullptr;

void Shape2D::init(Renderer* renderer) {
	instance = new Shape2D(renderer);
}

Shape2D* Shape2D::get() {
	assertDbg(instance && "must be initialized first!");
	return instance;
}

Shape2D::Shape2D(Renderer* renderer)
	: lineShaderProgram_(0)
	, indexPos_(0)
	, indexColor_(0)
	, indexMatViewport_(0)
{
	renderer->registerRenderable(this);
	lineShaderProgram_ = Shaders::createProgram("data/shaders/shape2d.vert", "data/shaders/shape2d.frag");
	if (lineShaderProgram_ == 0) {
		throw std::runtime_error("Unable to load line shaders!!");
	}
	indexPos_ = glGetAttribLocation(lineShaderProgram_, "vPos");
	indexColor_ = glGetAttribLocation(lineShaderProgram_, "vColor");
	indexMatViewport_ = glGetUniformLocation(lineShaderProgram_, "mViewportInverse");
}

Shape2D::~Shape2D() {
	glDeleteProgram(lineShaderProgram_);
}

void Shape2D::unload() {
	delete instance;
	instance = nullptr;
}

void Shape2D::render(Viewport* vp) {
	glUseProgram(lineShaderProgram_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendEquation(GL_BLEND_EQUATION_ALPHA);

	glUniformMatrix4fv(indexMatViewport_, 1, GL_FALSE, glm::value_ptr(vp->camera()->matProjView()));
	glEnableVertexAttribArray(indexPos_);
	glEnableVertexAttribArray(indexColor_);

	// set up viewport space settings:
	float sx = 2.f / (vp->width()-1);
	float sy = -2.f / (vp->height()-1);
	float sz = -1.e-2f;
	glm::mat4x4 matVP_to_UniformScale(glm::scale(glm::mat4(), glm::vec3(sx, sy, sz)));
	int vpw = vp->width(), vph = vp->height();
	glm::mat4x4 matVP_to_Uniform(glm::translate(matVP_to_UniformScale,
			glm::vec3(-vpw/2, -vph/2, 0)));
	glUniformMatrix4fv(indexMatViewport_, 1, GL_FALSE, glm::value_ptr(matVP_to_Uniform));
	glEnableVertexAttribArray(indexPos_);
	glEnableVertexAttribArray(indexColor_);

	// build triangle vertex buffer:
	static std::vector<glm::vec3> posBuf;
	posBuf.reserve(max(buffer_.size(), bufferTri_.size()));
	static std::vector<glm::vec4> colorBuf;
	colorBuf.reserve(max(buffer_.size(), bufferTri_.size()));

	// render triangle primitives:
	posBuf.clear();
	colorBuf.clear();
	for (auto &v : bufferTri_) {
		posBuf.push_back(glm::vec3(v.pos.x(vp), v.pos.y(vp), v.z));
		colorBuf.push_back(v.rgba);
	}
	glVertexAttribPointer(indexPos_, 3, GL_FLOAT, GL_FALSE, 0, &posBuf[0]);
	glVertexAttribPointer(indexColor_, 4, GL_FLOAT, GL_FALSE, 0, &colorBuf[0]);
	glDrawElements(GL_TRIANGLES, indicesTri_.size(), GL_UNSIGNED_SHORT, &indicesTri_[0]);

	// render line primitives:
	posBuf.clear();
	colorBuf.clear();
	for (auto &v : buffer_) {
		posBuf.push_back(glm::vec3(v.pos.x(vp), v.pos.y(vp), v.z));
		colorBuf.push_back(v.rgba);
	}
	glVertexAttribPointer(indexPos_, 3, GL_FLOAT, GL_FALSE, 0, &posBuf[0]);
	glVertexAttribPointer(indexColor_, 4, GL_FLOAT, GL_FALSE, 0, &colorBuf[0]);
	for (unsigned i=0; i<viewportFiltersLine_.size(); i++) {
		if (viewportFiltersLine_[i].empty()
				|| viewportFiltersLine_[i].find(vp->name()) != viewportFiltersLine_[i].end()) {
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, &indices_[i*2]);
		}
	}

	glDisable(GL_BLEND);
}

void Shape2D::purgeRenderQueue() {
	buffer_.clear();
	indices_.clear();
	bufferTri_.clear();
	indicesTri_.clear();
	viewportFiltersLine_.clear();
	viewportFiltersTri_.clear();
}

void Shape2D::drawLine(ViewportCoord point1, ViewportCoord point2, float z, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawLine(point1, point2, z, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawLine(ViewportCoord point1, ViewportCoord point2, float z, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	buffer_.push_back({point1, z, rgba});
	indices_.push_back(buffer_.size()-1);
	buffer_.push_back({point2, z, rgba});
	indices_.push_back(buffer_.size()-1);
	viewportFiltersLine_.push_back(viewportFilter);
}

void Shape2D::drawLineList(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawLineList(verts, nVerts, z, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawLineList(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], z, rgba});
		indices_.push_back(buffer_.size()-1);
		viewportFiltersLine_.push_back(viewportFilter);
	}
}

void Shape2D::drawLineStrip(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawLineStrip(verts, nVerts, z, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawLineStrip(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], z, rgba});
		indices_.push_back(buffer_.size()-1);
		if (i > 0 && i < nVerts-1)
			indices_.push_back(buffer_.size()-1);
		viewportFiltersLine_.push_back(viewportFilter);
	}
}

void Shape2D::drawPolygon(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawPolygon(verts, nVerts, z, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawPolygon(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], z, rgba});
		indices_.push_back(buffer_.size()-1);
		if (i > 0) {
			indices_.push_back(buffer_.size()-1);
			viewportFiltersLine_.push_back(viewportFilter);
		}
	}
	indices_.push_back(buffer_.size()-nVerts);
	viewportFiltersLine_.push_back(viewportFilter);
}

void Shape2D::drawPolygonFilled(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawPolygonFilled(verts, nVerts, z, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawPolygonFilled(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	//TODO must tesselate into triangles
}

void Shape2D::drawRectangle(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawRectangle(pos, z, size, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawRectangle(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	ViewportCoord coords[] {
		pos,
		pos + size.y(),
		pos + size,
		pos + size.x()
	};
	drawPolygon(coords, 4, z, rgba, viewportFilter);
}

void Shape2D::drawRectangleCentered(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawRectangleCentered(pos, z, size, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawRectangleCentered(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	auto hSize = size * 0.5f;
	ViewportCoord coords[] {
		pos - hSize,
		pos - hSize + hSize.y(),
		pos + hSize,
		pos + hSize - hSize.y()
	};
	drawPolygon(coords, 4, z, rgba, viewportFilter);
}

void Shape2D::drawRectangleFilled(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawRectangleFilled(pos, z, size, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawRectangleFilled(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	ViewportCoord coords[] {
		pos,
		pos + size.y(),
		pos + size,
		pos + size.x()
	};
	drawPolygonFilled(coords, 4, z, rgba, viewportFilter);
}

void Shape2D::drawCircle(ViewportCoord pos, float radius, float z, int nSides, glm::vec3 rgb, std::set<std::string> viewportFilter) {
	drawCircle(pos, radius, z, nSides, glm::vec4(rgb, 1), viewportFilter);
}

void Shape2D::drawCircle(ViewportCoord pos, float radius, float z, int nSides, glm::vec4 rgba, std::set<std::string> viewportFilter) {
	// make a polygon out of the circle
	float phiStep = 2 * PI * 1.f / nSides;
	ViewportCoord *v = new ViewportCoord[nSides];
	float phi = 0;
	for (int i=0; i<nSides; i++) {
		v[i] = pos + ViewportCoord{cosf(phi) * radius, sinf(phi) * radius};
		phi += phiStep;
	}
	drawPolygon(v, nSides, z, rgba, viewportFilter);
	delete [] v;
}
