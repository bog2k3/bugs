/*
 * Camera.cpp
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#include "Camera.h"
#include "Viewport.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(Viewport* vp)
	: pViewport(vp)
	, matViewProj(1)
	, zoomLevel(100)
	, cameraPos(0)
{
	updateViewProj();
}

Camera::~Camera() {
}

void Camera::setZoomLevel(float zoom) {
	zoomLevel = zoom;
	updateViewProj();
}

void Camera::move(glm::vec2 delta) {
	cameraPos += delta;
	updateViewProj();
}

void Camera::moveTo(glm::vec2 where) {
	cameraPos = where;
	updateViewProj();
}

void Camera::updateViewProj() {
	float halfWidth = pViewport->getWidth() / zoomLevel * 0.5f;
	float halfHeight = pViewport->getHeight() / zoomLevel * 0.5f;
	float left = cameraPos.x - halfWidth;
	float right = cameraPos.x + halfWidth;
	float top = cameraPos.y + halfHeight;
	float bottom = cameraPos.y - halfHeight;
	float znear = -10.f;
	float zfar = 10.f;
	matViewProj = glm::ortho(left, right, bottom, top, znear, zfar);
}
