/*
 * Gamete.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#include "Gamete.h"
#include "WorldConst.h"
#include "../genetics/Gene.h"
#include "../math/math2D.h"
#include "../body-parts/BodyConst.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../World.h"
#include "Bug.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(0.1f, 0.4f, 1.f);
static const int UPDATE_PERIOD = 10; // [frames]

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
}

void Gamete::onCollision(PhysicsBody* pOther, float impulse) {
	if (isZombie())
		return;
	// collision with another gamete. THIS is where the magic happens! :-)
	Gamete *other = (Gamete*)pOther->userPointer_;
	if (other->isZombie())
		return;
	if (abs(other->getChromosome().genes.size()-chromosome_.genes.size()) > (uint)WorldConst::MaxGenomeLengthDifference)
		return;
	Genome g;
	g.first = chromosome_;
	g.second = other->chromosome_;
	GeneticOperations::fixGenesSynchro(g);
	Bug *newlySpawnedBug = new Bug(g,
			body_.b2Body_->GetMass() + other->body_.b2Body_->GetMass(),
			(body_.getPosition() + other->body_.getPosition()) * 0.5f);
	World::getInstance()->takeOwnershipOf(newlySpawnedBug);
	destroy();
	other->destroy();
}

void Gamete::update(float dt) {
	if (isZombie())
		return;
	if (++updateSkipCounter_ < UPDATE_PERIOD)
		return;
	updateSkipCounter_ = 0;
	// attract other gamettes
	std::vector<b2Body*> bodies;
	World::getInstance()->getBodiesInArea(body_.getPosition(), WorldConst::GameteAttractRadius, true, bodies);
	for (auto b : bodies) {
		if (!b->GetUserData() || b->GetType() != b2_dynamicBody)
			continue;
		if (((PhysicsBody*)b->GetUserData())->userObjectType_ == ObjectTypes::GAMETE) {
			// found another gamete, let's attract each other
			// p.s.: only attract and fuse if they don't differ by more than N in the number of genes - N is the same as the new gene recorded history
			Gamete* other = (Gamete*)((PhysicsBody*)b->GetUserData())->userPointer_;
			if (abs(other->getChromosome().genes.size()-chromosome_.genes.size()) > (uint)WorldConst::MaxGenomeLengthDifference)
				continue;
			b2Vec2 force = body_.b2Body_->GetPosition() - b->GetPosition();
			float distance = force.Normalize();
			force *= WorldConst::GameteAttractForceFactor / (1+sqr(distance));
			force *= b->GetMass() * body_.b2Body_->GetMass();
			b->ApplyForceToCenter(force, true);
			body_.b2Body_->ApplyForceToCenter(-force, true);
		}
	}
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
