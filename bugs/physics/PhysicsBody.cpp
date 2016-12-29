/*
 * PhysicsBody.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#include "PhysicsBody.h"
#include "../World.h"
#include "../math/box2glm.h"
#include "../math/aabb.h"
#include <Box2D/Box2D.h>
#include <cmath>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

PhysicsBody::PhysicsBody(ObjectTypes userObjType, void* userPtr, EventCategoryFlags::type categFlags, EventCategoryFlags::type collisionMask)
	: b2Body_(nullptr)
	, userObjectType_(userObjType)
	, userPointer_(userPtr)
	, categoryFlags_(categFlags)
	, collisionEventMask_(collisionMask)
{
}

void PhysicsBody::create(const PhysicsProperties& props) {
	assertDbg(b2Body_==nullptr);
	assertDbg(userPointer_ != nullptr);
	assertDbg(userObjectType_ != ObjectTypes::UNDEFINED);
	assertDbg(!std::isnan(props.angle));
	assertDbg(!std::isnan(props.angularVelocity));
	assertDbg(!std::isnan(props.position.x));
	assertDbg(!std::isnan(props.position.y));
	assertDbg(!std::isnan(props.velocity.x));
	assertDbg(!std::isnan(props.velocity.y));

	b2BodyDef def;
	def.angle = props.angle;
	def.position.Set(props.position.x, props.position.y);
	def.type = props.dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.3f;
	def.angularVelocity = props.angularVelocity;
	def.linearVelocity = g2b(props.velocity);

	World::getInstance()->queueDeferredAction([this, def] {
		b2Body_ = World::getInstance()->getPhysics()->CreateBody(&def);
	});
}

PhysicsBody::~PhysicsBody() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	onDestroy.trigger(this);
	if (b2Body_) {
		auto bodyPtr = b2Body_;
		bodyPtr->GetWorld()->DestroyBody(bodyPtr);
	}
}

PhysicsBody* PhysicsBody::getForB2Body(b2Body* body) {
	if (!body->GetUserData())
		return nullptr;
	return (PhysicsBody*)body->GetUserData();
}

aabb PhysicsBody::getAABB() const {
	aabb x;
	if (b2Body_) {
		for (b2Fixture* pFix = b2Body_->GetFixtureList(); pFix; pFix = pFix->GetNext()) {
			x = x.reunion(pFix->GetAABB(0));
		}
	}
	return x;
}
