#pragma once

#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
#include "constants.h"

template<typename T> constexpr T sqr(T const &x) { return x*x; }
template<typename T> inline void xchg(T &x1, T &x2) { T aux = x1; x1 = x2; x2 = aux; }
template<typename T> constexpr T min(T const &x, T const &y) { return x < y ? x : y; }
template<typename T> constexpr T max(T const &x, T const &y) { return x > y ? x : y; }
template<typename T> constexpr T sign(T const& x) { return x > 0 ? T(+1) : (x < 0 ? T(-1) : T(0)); }
template<typename T> constexpr T abs(T const& x) { return x < 0 ? -x : x; }

inline glm::vec2 getNormalVector(glm::vec2 v) { return glm::vec2(-v.y, v.x); }

float constexpr eqEps(float f1, float f2) { return abs(f1 - f2) < EPS; }

constexpr float cross2D(const glm::vec2 &v1, const glm::vec2 &v2) {
	return (v1.x*v2.y) - (v1.y*v2.x);
}

/*
 * computes the angle from (0,0) in direction p. p is assumed to be normalized
 */
inline float pointDirectionNormalized(glm::vec2 const &p) {
	float sina = cross2D(glm::vec2(1,0), p);
	if (p.x < 0)
		return asinf(sina) + PI*0.5f*sign(sina);
	else
		return asinf(sina);
}

/*
 * computes the angle from (0,0) in direction p. p can have any arbitrary length
 */
inline float pointDirection(glm::vec2 const &p) {
	return pointDirectionNormalized(glm::normalize(p));
}

/**
 * brings an angle into a user defined range (bisector being the max angle where the circle is cut):
 * 	[bisector-2*PI, bisector]
 * for example, providing PI/2 as bisector, the angle will be brought into this interval:
 * 	[-3*PI/2, PI/2]
 */
inline float limitAngle(float a, float bisector) {
	while (a > bisector)
		a -= 2*PI;
	while (a < bisector - 2*PI)
		a += 2*PI;
	return a;
}

inline glm::vec2 vec3xy(glm::vec3 const &in) {
	return glm::vec2(in.x, in.y);
}

/**
 * computes the distance from point P to the line defined by lineOrigin and lineDirection.
 * lineDirection is assumed to be normalized.
 */
inline float distPointLine(glm::vec2 P, glm::vec2 lineOrigin, glm::vec2 lineDirection) {
	glm::vec2 OP = P - lineOrigin;
	return glm::length(OP - lineDirection * glm::dot(OP, lineDirection));
}

template<typename T> T constexpr clamp(T x, T a, T b) {
	return x < a ? a : (x > b ? b : x);
}

/**
 * linearly interpolates between a and b by factor t
 * t is assumed to be in [0.0, 1.0]
 * use clamp on t before calling if unsure
 */
template<typename T> T constexpr lerp(T a, T b, float t) {
	return a * (1-t) + b*t;
}

/**
 * sample a value from an array by linearly interpolating across neighbor values
 * The position is a float index, giving the center of the sample kernel
 * The size of the kernel is 1.0
 * the function doesn't do bound checking on the initial position,
 * but it is safe to use on the first or last locations in the vector - will not sample neighbors outside the vector
 */
template<typename T> inline T lerp_lookup(const T* v, int nV, float position) {
	int index = int(position);
	float lerpFact = position - index;
	float value = v[index];
	if (lerpFact < 0.5f && index > 0) {
		// lerp with previous value
		value = lerp(v[index-1], value, lerpFact + 0.5f);
	} else if (lerpFact > 0.5f && index < nV-1) {
		// lerp with next value
		value = lerp(value, v[index+1], lerpFact - 0.5f);
	}
	return value;
}

/**
 * Casts a ray from the box's center in the given direction and returns the coordinates of the point
 * on the edge of the box that is intersected by the ray
 */
glm::vec2 rayIntersectBox(float width, float height, float direction);
