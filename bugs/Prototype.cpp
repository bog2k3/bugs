/*
 * Prototype.cpp
 *
 *  Created on: Oct 29, 2017
 *      Author: bog
 */

#include "Prototype.h"

#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/math/constants.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/utils/rand.h>
#include <boglfw/World.h>

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

#include <vector>
#include <algorithm>

// ------------------------------------------------------------------------------

b2Body *b1, *b2;
b2Joint *joint;

// ------------------------------------------------------------------------------

void Prototype::initialize() {
	b2BodyDef bdef;
	bdef.type = b2_dynamicBody;

	bdef.position = {-0.51f, 0.f};
	b1 = World::getInstance().getPhysics()->CreateBody(&bdef);

	bdef.position = {0.51f, 0.f};
	b2 = World::getInstance().getPhysics()->CreateBody(&bdef);

	b2PolygonShape shape;
	shape.SetAsBox(0.5f, 0.3f);
	b2FixtureDef fdef;
	fdef.shape = &shape;
	fdef.density = 1.f;

	b1->CreateFixture(&fdef);
	b2->CreateFixture(&fdef);

	b2WeldJointDef jdef;
	jdef.bodyA = b1;
	jdef.bodyB = b2;
	jdef.localAnchorA = {0.55f, 0.05f};
	jdef.localAnchorB = {-0.55f, -0.05f};
	jdef.referenceAngle = 0;

	joint = World::getInstance().getPhysics()->CreateJoint(&jdef);
}

void Prototype::terminate() {
	World::getInstance().getPhysics()->DestroyBody(b1);
	World::getInstance().getPhysics()->DestroyBody(b2);
	b1 = b2 = nullptr;
	joint = nullptr;
}

void Prototype::draw(Viewport* vp) {
	if (!enabled_)
		return;
	auto anchorA = joint->GetAnchorA();
	float localAngle = pointDirection(b2g(anchorA - b1->GetPosition()));
	float bodyAngle = b1->GetAngle();
	float angle = pointDirection(b2g(b2->GetPosition() - b1->GetPosition()));
	auto pos = b2g(anchorA);
	Shape3D::get()->drawLine({pos - glm::rotate(glm::vec2(0.05f, 0), angle), 0},
		{pos + glm::rotate(glm::vec2(0.05f, 0), angle), 0},
		{1.f, 1.f, 0.f});
	Shape3D::get()->drawLine({pos - glm::rotate(glm::vec2(0, 0.02f), angle), 0},
		{pos + glm::rotate(glm::vec2(0, 0.02f), angle), 0},
		{1.f, 1.f, 0.f});
}

void Prototype::update(float dt) {
	if (!enabled_)
		return;
}

void Prototype::onKeyDown(int key) {
	if (!enabled_)
		return;

	switch (key) {
	case GLFW_KEY_N:
		break;
	}
}

void Prototype::onKeyUp(int key) {
	if (!enabled_)
		return;
}
