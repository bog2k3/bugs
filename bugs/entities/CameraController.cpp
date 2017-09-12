/*
 * CameraController.cpp
 *
 *  Created on: May 7, 2017
 *      Author: bog
 */

#include "CameraController.h"
#include "../renderOpenGL/Camera.h"
#include "../math/math3D.h"

CameraController::CameraController(Camera* target)
	: camera_(target) {
}

CameraController::~CameraController() {
}

void CameraController::update(float dt) {
	/*if (path_.empty())
		return;

	if (lerpFactor_ >= 1) {
		// reached the next vertex
		if (pathIndex_ > 0 && (unsigned)pathIndex_ == path_.size()) {
			path_.clear(); // finished the path
			return;
		}

		jumpNext_ = path_[pathIndex_].type == pathNode::jump;
		if (path_[pathIndex_].type == pathNode::jump ||
				path_[pathIndex_].type == pathNode::redirect) {
			pathIndex_ = clamp(path_[pathIndex_].targetIndex, 0u, (unsigned)path_.size() - 1);
			return;
		}
		if (jumpNext_) {
			jumpNext_ = false;
			camera_->moveTo(path_[pathIndex_].position);
			lastLookAt_ = path_[pathIndex_].lookAtTarget;
			camera_->lookAt(lastLookAt_);
			pathIndex_++;
			return;
		}

		lerpFactor_ = 0;
		auto &next = path_[pathIndex_];
		origin_ = camera_->position();
		originLookAt_ = lastLookAt_;
		segmentLength_ = (next.position - origin_).length();
		maxLerpSpeed_ = cruiseSpeed_ / segmentLength_;
	}

	if (lerpFactor_ > 0.9f) { // nearing the end
		// slow down
		float delta = maxLerpSpeed_ * 0.5f * dt;
		if (lerpSpeed_ > delta)
			lerpSpeed_ -= delta;
	} else if (lerpSpeed_ < maxLerpSpeed_) {
		// speed up
		lerpSpeed_ += maxLerpSpeed_ * 0.5f * dt;
	}
	lerpFactor_ += lerpSpeed_ * dt;
	auto pos = lerp(origin_, path_[pathIndex_].position, clamp(lerpFactor_, 0.f, 1.f));
	auto target = lerp(originLookAt_, path_[pathIndex_].lookAtTarget, clamp(lerpFactor_, 0.f, 1.f));
	camera_->moveTo(pos);
	camera_->lookAt(target);
	lastLookAt_ = target;
	if (lerpFactor_ >= 1)
		pathIndex_++;*/

	pathLerper_.update(dt);
	camera_->moveTo(pathLerper_.value().position);
	camera_->lookAt(pathLerper_.value().lookAtTarget);
}

void CameraController::startBackAndForth(glm::vec3 p1, glm::vec3 p2, glm::vec3 lookAtTarget, float speed) {
	pathLerper_.setPath({
		PathNode<CameraNode>{PathNodeType::vertex, {p1, lookAtTarget}, 0},	//#0
		PathNode<CameraNode>{PathNodeType::vertex, {p2, lookAtTarget}, 0},	//#1
		PathNode<CameraNode>{PathNodeType::redirect, {{0,0,0}, {0,0,0}}, 0}	// -> #0
	});
	pathLerper_.start(speed);
}

/*void CameraController::startPath(std::vector<pathNode> path, float speed) {
	path_ = path;
	cruiseSpeed_ = speed;
	pathIndex_ = 0;
	origin_ = camera_->position();
	originLookAt_ = camera_->position() + camera_->direction();
	lastLookAt_ = originLookAt_;
	segmentLength_ = (camera_->position() - path[0].position).length();
	lerpFactor_ = 0;
	maxLerpSpeed_ = cruiseSpeed_ / segmentLength_;
	lerpSpeed_ = 0;
}*/

