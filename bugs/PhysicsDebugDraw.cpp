/*
 * PhysicsDebugDraw.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#include "PhysicsDebugDraw.h"
#include "renderOpenGL/Shape2D.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "math/box2glm.h"

PhysicsDebugDraw::PhysicsDebugDraw(ObjectRenderContext ctx) : rc(ctx) {
	// TODO Auto-generated constructor stub
}

PhysicsDebugDraw::~PhysicsDebugDraw() {
	// TODO Auto-generated destructor stub
}

/// Draw a closed polygon provided in CCW order.
void PhysicsDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	rc.shape->drawPolygon((glm::vec2*)vertices, (int)vertexCount, 0, b2g(color));
}

/// Draw a solid closed polygon provided in CCW order.
void PhysicsDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	DrawPolygon(vertices, vertexCount, color);
}

/// Draw a circle.
void PhysicsDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {

}

/// Draw a solid circle.
void PhysicsDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
	DrawCircle(center, radius, color);
}

/// Draw a line segment.
void PhysicsDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
	rc.shape->drawLine(b2g(p1), b2g(p2), 0, b2g(color));
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void PhysicsDebugDraw::DrawTransform(const b2Transform& xf) {

}
