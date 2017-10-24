/*
 * Camera.cpp
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#include "Viewport.h"
#include "../math/math3D.h"

#include "../utils/log.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "Camera.h"

Camera::Camera(Viewport* vp)
	: pViewport_(vp)
	, fov_(PI/3)
	, matView_(1)
	, matProj_(1)
	, position_(0)
	, direction_(0, 0, 1)
{
	setOrthoZoom(100);
}

Camera::~Camera() {
}

void Camera::setFOV(float fov) {
	fov_ = fov;
	updateProj();
}

void Camera::setOrthoZoom(float zoom) {
	zoomLevel_ = zoom;
	float width = pViewport_->width() / zoomLevel_;
	float height = pViewport_->height() / zoomLevel_;
	orthoSize_ = { width, height };
	fov_ = 0;
	updateProj();
}

void Camera::setOrtho(glm::vec2 size) {
	orthoSize_ = size;
	fov_ = 0;
	zoomLevel_ = pViewport_->width() / orthoSize_.x;
	updateProj();
}

void Camera::move(glm::vec3 delta) {
	position_ += delta;
	updateView();
}

void Camera::moveTo(glm::vec3 where) {
	position_ = where;
	updateView();
}

void Camera::lookAt(glm::vec3 where) {
	direction_ = glm::normalize(where - position_);
	updateView();
}

void Camera::updateView() {
	matView_ = glm::lookAtLH(position_, position_ + direction_, {0, 1, 0});
}

void Camera::updateProj() {
	float zNear = 0.5f;
	float zFar = 50.f;
	if (fov_ == 0) {
		// set ortho
		matProj_ = glm::ortho(-orthoSize_.x * 0.5f, orthoSize_.x * 0.5f, -orthoSize_.y * 0.5f, orthoSize_.y * 0.5f, zNear, zFar);
	} else {
		// set perspective
		matProj_ = glm::perspectiveFovLH(fov_, (float)pViewport_->width(), (float)pViewport_->height(), zNear, zFar);
	}
}

