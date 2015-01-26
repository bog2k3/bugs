/*
 * Gamete.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#include "Gamete.h"
#include "../genetics/Gene.h"
#include "../math/math2D.h"
#include "../body-parts/BodyConst.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(0.1f, 0.4f, 1.f);

Gamete::Gamete(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass)
	: chromosome_(ch)
	, body_(ObjectTypes::GAMETE, this, EventCategoryFlags::GAMETE, EventCategoryFlags::GAMETE)
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

	body_.onCollision.add(std::bind(&Gamete::onCollision, this, std::placeholders::_1, std::placeholders::_2));
}

Gamete::~Gamete() {
	// TODO Auto-generated destructor stub
}

void Gamete::onCollision(PhysicsBody* pOther, float impulse) {

}

void Gamete::update(float dt) {
	// TODO attract other gamets
}

#ifdef DEBUG_DRAW_GAMETE
void Gamete::draw(RenderContext const& ctx) {
	static constexpr int nSeg = 7;
	static constexpr float lengthRatio = 0.8f;
	static constexpr float widthRatio = 0.2f;
	float radius = sqrtf(body_.b2Body_->GetMass() * BodyConst::ZygoteDensityInv * PI_INV);
	glm::vec2 origin = body_.getPosition();
	glm::vec2 i(glm::rotate(glm::vec2(1,0), body_.b2Body_->GetAngle()));
	glm::vec2 j(glm::rotate(glm::vec2(0,1), body_.b2Body_->GetAngle()));
	float width = widthRatio * radius;
	float segHeight = lengthRatio * radius * 2.f / nSeg;
	float offset = nSeg * 0.5f * segHeight;
	glm::vec2 v[nSeg+1];
	v[0] = origin -width*i + offset*j;
	for (int k=0; k<nSeg; k++) {
		float side = k%2 ? -1 : +1;
		v[k+1] = origin + width * side * i + (offset-(k+1)*segHeight) * j;
	}
	ctx.shape->drawLineStrip(v, nSeg+1, 0, debug_color);
}
#endif
