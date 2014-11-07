/*
 * Renderer.cpp
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

Renderer::~Renderer() {
	// TODO Auto-generated destructor stub
}

Renderer::Renderer()
		: zoomLevel(100.f), screenWidth(640), screenHeight(480), cameraPos(0) {
	updateViewProj();
}

void Renderer::registerRenderable(Renderable r) {
	if (r == nullptr)
		throw new std::invalid_argument("argument cannot be null!");
	renderComponents.push_back(r);
}

void Renderer::render() {
	for (auto r : renderComponents) {
		r();
	}

	Renderer r;
}

void Renderer::setZoomLevel(float zoom) {
	zoomLevel = zoom;
	updateViewProj();
}

void Renderer::setScreenSize(int width, int height) {
	screenWidth = width;
	screenHeight = height;
	updateViewProj();
}

void Renderer::moveCamera(glm::vec2 delta) {
	cameraPos += delta;
	updateViewProj();
}

void Renderer::moveCameraTo(glm::vec2 where) {
	cameraPos = where;
	updateViewProj();
}

void Renderer::updateViewProj() {
	float halfWidth = screenWidth / zoomLevel * 0.5f;
	float halfHeight = screenHeight / zoomLevel * 0.5f;
	float left = cameraPos.x - halfWidth;
	float right = cameraPos.x + halfWidth;
	float top = cameraPos.y + halfHeight;
	float bottom = cameraPos.y - halfHeight;
	float znear = -10.f;
	float zfar = 10.f;
	matViewProj = glm::ortho(left, right, bottom, top, znear, zfar);
}
