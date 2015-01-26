/*
 * Egg.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#include "Egg.h"
#include "../genetics/Gene.h"
#include "../math/math2D.h"
#include "../body-parts/BodyConst.h"
#include <Box2D/Box2D.h>

Egg::Egg(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass)
	: chromosome_(ch)
	, body_(ObjectTypes::EGG, this, EventCategoryFlags::EGG, EventCategoryFlags::EGG)
{
	PhysicsProperties props(pos, 0, true, speed, 0);
	body_.create(props);

	float size = mass * BodyConst::ZygoteDensityInv;
	b2CircleShape shp;
	shp.m_radius = sqrtf(size * PI_INV);
	b2FixtureDef fd;
	fd.density = BodyConst::ZygoteDensity;
	fd.friction = 0.5f;
	fd.restitution = 0.5f;
	fd.shape = &shp;
	body_.b2Body_->CreateFixture(&fd);

	body_.onCollision.add(std::bind(&Egg::onCollision, this, std::placeholders::_1, std::placeholders::_2));
}

Egg::~Egg() {
	// TODO Auto-generated destructor stub
}

void Egg::onCollision(PhysicsBody* pOther, float impulse) {

}
