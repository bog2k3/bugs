/*
 * Prototype.cpp
 *
 *  Created on: Oct 29, 2017
 *      Author: bog
 */

#include "Prototype.h"
#include "renderOpenGL/RenderContext.h"
#include "renderOpenGL/Shape3D.h"
#include "math/constants.h"
#include "utils/rand.h"
#include "World.h"
#include "physics/PhysicsBody.h"

#include <Box2D/Box2D.h>
#include <GLFW/glfw3.h>

#include <vector>

// ------------------------------------------------------------------------------

struct cell {
	static constexpr float density = 0.1f;

	float size; // area
	float division_angle;

	PhysicsBody body;
	b2Fixture* fixture;

	struct link {
		float angle;
		b2WeldJoint* joint;
		cell* other;
	};
	std::vector<link> neighbours;

	float radius() { return sqrtf(size / M_PI); }
	glm::vec2 position() { return body.getPosition(); }
	float rotation() { return body.getRotation(); }

	cell(float size, glm::vec2 position, float rotation)
		: size(size), division_angle(randf() * 2*PI)
		, body(ObjectTypes::PROTOTYPE, this, 0, 0)
		, fixture(nullptr)
	{
		World::getInstance()->queueDeferredAction([this, position, rotation] {
			body.create(PhysicsProperties{position, rotation});

			b2CircleShape shape;
			shape.m_p.Set(0, 0);
			shape.m_radius = radius();

			b2FixtureDef fixDef;
			fixDef.density = density;
			fixDef.friction = 0.2f;
			fixDef.restitution = 0.3f;
			fixDef.shape = &shape;

			body.b2Body_->CreateFixture(&fixDef);
		});
	}

	cell(cell &&) = delete;
	cell(cell const&) = delete;
	~cell() = default;

	void divide(float ratio, bool reorientate, bool mirror) {

	}
};

std::vector<cell*> cells;
unsigned selected = 0;

// ------------------------------------------------------------------------------

void Prototype::draw(RenderContext const& ctx) {
	if (!enabled_)
		return;

	if (selected < cells.size()) {
		float rad = cells[selected]->radius();
		glm::vec2 pos = cells[selected]->position();
		Shape3D::get()->drawRectangleXOYCentered(pos, {2*rad, 2*rad}, 0.f, {0, 1, 0});
	}
	for (auto c : cells) {
		glm::vec2 v2 = c->position();
		v2.x += cosf(c->rotation()) * c->radius();
		v2.y += sinf(c->rotation()) * c->radius();
		Shape3D::get()->drawLine({c->position(), 0}, {v2, 0}, {1, 1, 0});
		v2 = c->position();
		v2.x += cosf(c->division_angle + c->rotation()) * c->radius() * 1.2f;
		v2.y += sinf(c->division_angle + c->rotation()) * c->radius() * 1.2f;
		glm::vec2 v1 = 2.f * c->position() - v2;
		Shape3D::get()->drawLine({v1, 0}, {v2, 0}, {1, 0, 0});
	}
}

void Prototype::update(float dt) {
	if (!enabled_)
		return;

	if (cells.size() == 0) {
		// create initial cell
		cells.push_back(new cell(5.f, {0, 0}, randf() * 2*PI));
	}
}

void Prototype::onKeyDown(int key) {
	if (!enabled_)
		return;

	switch (key) {
	case GLFW_KEY_N:
		selected = (selected+1) % cells.size();
		break;
	case GLFW_KEY_1:
		cells[selected]->divide(false);
	case GLFW_KEY_2:
		cells[selected]->divide(true);
	}
}

void Prototype::onKeyUp(int key) {
	if (!enabled_)
		return;

}
