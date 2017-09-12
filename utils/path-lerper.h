/*
 * path-lerper.h
 *
 *  Created on: May 18, 2017
 *      Author: bog
 */

#ifndef UTILS_PATH_LERPER_H_
#define UTILS_PATH_LERPER_H_

#include "../math/math3D.h"
#include "../utils/log.h"

#include <vector>

enum class PathNodeType {
	vertex,		// regular path vertex (will flow towards the next vertex unless jump or redirect or end of path)
	jump,		// abrupt jump to a path vertex
	redirect,	// smooth redirect towards a path vertex
};

template<class NodeValue>
struct PathNode {
	PathNodeType type;		// type of node from above ^
	NodeValue value;		// your custom value goes here
	unsigned targetIndex; 	// for jump or redirect
};

template<class NodeValue>
NodeValue genericLerp(NodeValue const& n1, NodeValue const& n2, float factor) {
	return n1 * (1-factor) + n2 * factor;
}


template<class NodeValue>
float genericDistance(NodeValue const& n1, NodeValue const& n2) {
	return abs<float>(n2 - n1);
}

template<class NodeValue,
	class LerpFunction = decltype(genericLerp<NodeValue>)*,
	class DistanceFunction = decltype(genericDistance<NodeValue>)*>
class PathLerper {
public:
	PathLerper(LerpFunction lerpFn = genericLerp<NodeValue>, DistanceFunction distFn = genericDistance<NodeValue>);
	void setPath(std::vector<PathNode<NodeValue>> path);
	void appendNode(NodeValue val);
	void appendJump(unsigned target);
	void appendRedirect(unsigned target);

	void start(float speed, unsigned initialIndex = 0);

	void update(float dt);

	NodeValue value() const { return last_; }
	NodeValue vertex(int i) const { return path_[i].value; }

private:
	std::vector<PathNode<NodeValue>> path_;
	float cruiseSpeed_ = 0;
	int pathIndex_ = -1;
	NodeValue origin_;
	NodeValue last_;
	float segmentLength_ = 0;
	float lerpFactor_ = 0;
	float lerpSpeed_ = 0;
	float maxLerpSpeed_ = 0;
	bool jumpNext_ = false;

	LerpFunction lerpFn_;
	DistanceFunction distFn_;
};

//------------------------------------------------------------------------------------------------
// implementation
//------------------------------------------------------------------------------------------------

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
PathLerper<NodeValue, LerpFunction, DistanceFunction>::PathLerper(LerpFunction lerpFn, DistanceFunction distFn)
	: lerpFn_(lerpFn)
	, distFn_(distFn)
{
}

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
void PathLerper<NodeValue, LerpFunction, DistanceFunction>::setPath(std::vector<PathNode<NodeValue>> path)
{
	path_ = path;
	if (cruiseSpeed_ > 0) {
		start(cruiseSpeed_, 0);
	}
}

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
void PathLerper<NodeValue, LerpFunction, DistanceFunction>::appendNode(NodeValue n) {
	path_.push_back(PathNode<NodeValue> {
		PathNodeType::vertex,
		n,
		0
	});
}

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
void PathLerper<NodeValue, LerpFunction, DistanceFunction>::appendJump(unsigned to) {
	path_.push_back(PathNode<NodeValue> {
		PathNodeType::jump,
		{},
		to
	});
}

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
void PathLerper<NodeValue, LerpFunction, DistanceFunction>::appendRedirect(unsigned to) {
	path_.push_back(PathNode<NodeValue> {
		PathNodeType::redirect,
		{},
		to
	});
}

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
void PathLerper<NodeValue, LerpFunction, DistanceFunction>::start(float speed, unsigned initial) {
	cruiseSpeed_ = speed;
	pathIndex_ = initial;
	origin_ = path_[pathIndex_].value;
	last_ = origin_;
	segmentLength_ = 0.f;
	lerpFactor_ = 0.99f;
	maxLerpSpeed_ = cruiseSpeed_ / segmentLength_;
	lerpSpeed_ = 1;
}

template<class NodeValue,
	class LerpFunction,
	class DistanceFunction>
void PathLerper<NodeValue, LerpFunction, DistanceFunction>::update(float dt) {
	if (path_.empty())
		return;

	if (lerpFactor_ >= 1) {
		// reached the next vertex
		if (pathIndex_ > 0 && (unsigned)pathIndex_ == path_.size()) {
			path_.clear(); // finished the path
			return;
		}

		jumpNext_ = path_[pathIndex_].type == PathNodeType::jump;
		if (path_[pathIndex_].type == PathNodeType::jump ||
				path_[pathIndex_].type == PathNodeType::redirect) {
			pathIndex_ = clamp(path_[pathIndex_].targetIndex, 0u, (unsigned)path_.size() - 1);
			return;
		}
		if (jumpNext_) {
			jumpNext_ = false;
			pathIndex_++;
			return;
		}

		//DEBUGLOGLN("next path segment: " << pathIndex_);

		lerpFactor_ = 0;
		auto &next = path_[pathIndex_];
		origin_ = last_;
		segmentLength_ = distFn_(origin_, next.value);
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
	lerpFactor_ += dt; //lerpSpeed_ * dt;
	last_ = lerpFn_(origin_, path_[pathIndex_].value, clamp(lerpFactor_, 0.f, 1.f));
	if (lerpFactor_ >= 1) {
		//DEBUGLOGLN("finished segment " << pathIndex_);
		pathIndex_++;
	}
}

#endif /* UTILS_PATH_LERPER_H_ */
