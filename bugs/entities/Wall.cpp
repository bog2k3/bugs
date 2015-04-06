/*
 * Wall.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#include "Wall.h"
#include "../math/math2D.h"
#include "../World.h"
#include "../serialization/BinaryStream.h"
#include <Box2D/Box2D.h>

Wall::Wall(glm::vec2 const &from, glm::vec2 const &to, float width)
	: body_(ObjectTypes::WALL, this, EventCategoryFlags::STATIC, 0)
	, from_(from)
	, to_(to)
	, width_(width)
{
	glm::vec2 delta(to-from);
	float length = glm::length(delta);
	float angle = pointDirectionNormalized(delta / length);
	PhysicsProperties props((from + to)*0.5f, angle, false, glm::vec2(0), 0);
	body_.create(props);

	b2PolygonShape shp;
	shp.SetAsBox(length * 0.5f, width*0.5f);
	b2FixtureDef fd;
	fd.friction = 0.5f;
	fd.restitution = 0.5f;
	fd.shape = &shp;
	body_.b2Body_->CreateFixture(&fd);
}

Wall::~Wall() {
}

void Wall::deserialize(BinaryStream &stream) {
	glm::vec2 from, to;
	float width;
	stream >> from.x >> from.y >> to.x >> to.y >> width;
	World::getInstance()->takeOwnershipOf(std::unique_ptr<Wall>(new Wall(from, to, width)));
}

void Wall::serialize(BinaryStream &stream) {
	stream << from_.x << from_.y;
	stream << to_.x << to_.y;
	stream << width_;
}
