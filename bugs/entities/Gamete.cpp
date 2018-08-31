/*
 * Gamete.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#include "Gamete.h"
#include "WorldConst.h"
#include "../genetics/Gene.h"
#include "../body-parts/BodyConst.h"
#include "../ObjectTypesAndFlags.h"
#include "Bug/Bug.h"

#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/World.h>
#include <boglfw/utils/log.h>
#include <boglfw/perf/marker.h>

#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(0.1f, 0.4f, 1.f);
static const int UPDATE_PERIOD = 10; // [frames]

Gamete::Gamete(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass)
	: chromosome_(ch)
	, body_(ObjectTypes::GAMETE, this,
			EventCategoryFlags::GAMETE | EventCategoryFlags::FOOD,	// TODO handle mouth collision properly (from Mouth)
			EventCategoryFlags::GAMETE)
{
	World::getInstance().queueDeferredAction([this, pos, speed, mass]() {
		PhysicsProperties props(pos, 0, true, speed, 0);
		body_.create(props);
		body_.getEntityFunc_ = &getEntityFromGametePhysBody;

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
	});
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
	if (!chromosome_.isGeneticallyCompatible(other->getChromosome()))
		return;
	LOGLN("gametes fused -> new bug embryo !!!");
	Genome g;
	// combine the two chromosomes into a single genome
	g.first = chromosome_;
	g.second = other->chromosome_;
	// compute resultant velocity for new zygote:
	b2Vec2 velocity = (body_.b2Body_->GetMass() * body_.b2Body_->GetLinearVelocity()
			+ other->body_.b2Body_->GetMass() * other->body_.b2Body_->GetLinearVelocity());
	velocity *= 1.f / (body_.b2Body_->GetMass() + other->body_.b2Body_->GetMass());
	// now create the zygote:
	std::unique_ptr<Bug> newlySpawnedBug(new Bug(g,
			body_.b2Body_->GetMass() + other->body_.b2Body_->GetMass(),
			(body_.getPosition() + other->body_.getPosition()) * 0.5f, b2g(velocity),
			std::max(generation_, other->generation_)));
	World::getInstance().takeOwnershipOf(std::move(newlySpawnedBug));
	// destroy these gamettes:
	destroy();
	other->destroy();
}

void Gamete::update(float dt) {
	PERF_MARKER_FUNC;
	if (isZombie())
		return;
	if (++updateSkipCounter_ < UPDATE_PERIOD)
		return;
	updateSkipCounter_ = 0;
	// attract other gamettes
	std::vector<b2Body*> bodies;
	World::getInstance().getBodiesInArea(body_.getPosition(), WorldConst::GameteAttractRadius, true, bodies);
	for (auto b : bodies) {
		if (!b->GetUserData() || b->GetType() != b2_dynamicBody)
			continue;
		if (((PhysicsBody*)b->GetUserData())->userObjectType_ == ObjectTypes::GAMETE) {
			// found another gamete, let's attract each other
			// p.s.: only attract and fuse if they don't differ by more than N in the number of genes - N is the same as the new gene recorded history
			Gamete* other = (Gamete*)((PhysicsBody*)b->GetUserData())->userPointer_;
			if (!chromosome_.isGeneticallyCompatible(other->getChromosome()))
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
void Gamete::draw(Viewport* vp) {
	static constexpr int nSeg = 7;
	static constexpr float lengthRatio = 0.8f;
	static constexpr float widthRatio = 0.2f;
	float radius = sqrtf(body_.b2Body_->GetMass() * BodyConst::ZygoteDensityInv * PI_INV);
	glm::vec3 origin { body_.getPosition(), 0 };
	glm::vec3 i { glm::rotate(glm::vec2(1,0), body_.b2Body_->GetAngle()), 0 };
	glm::vec3 j { glm::rotate(glm::vec2(0,1), body_.b2Body_->GetAngle()), 0 };
	float width = widthRatio * radius;
	float segHeight = lengthRatio * radius * 2.f / nSeg;
	float offset = nSeg * 0.5f * segHeight;
	glm::vec3 v[nSeg+1];
	v[0] = origin -width*i + offset*j;
	for (int k=0; k<nSeg; k++) {
		float side = k%2 ? -1 : +1;
		v[k+1] = origin + width * side * i + (offset-(k+1)*segHeight) * j;
	}
	Shape3D::get()->drawLineStrip(v, nSeg+1, debug_color);
}
#endif

void Gamete::serialize(BinaryStream &stream) const {
	//TODO...
}

void Gamete::deserialize(BinaryStream &stream) {
	//TODO...
}

//glm::vec3 Gamete::getWorldTransform() const {
//	if (body_.b2Body_) {
//		auto pos = body_.b2Body_->GetPosition();
//		return glm::vec3(b2g(pos), body_.b2Body_->GetAngle());
//	} else
//		return glm::vec3(0);
//}

Entity* Gamete::getEntityFromGametePhysBody(PhysicsBody const& body) {
	Gamete* pGamete = static_cast<Gamete*>(body.userPointer_);
	assertDbg(pGamete);
	return pGamete;
}

aabb Gamete::getAABB(bool requirePrecise) const {
	return body_.getAABB();
}
