/*
 * Gamete.h
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#ifndef ENTITIES_GAMETE_H_
#define ENTITIES_GAMETE_H_

#include "enttypes.h"
#include "../genetics/Genome.h"
#include "../serialization/objectTypes.h"

#include <boglfw/entities/Entity.h>
#include <boglfw/physics/PhysicsBody.h>
#include <boglfw/utils/bitFlags.h>

#define DEBUG_DRAW_GAMETE

class Gamete: public Entity {
public:
	Gamete(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass);
	virtual ~Gamete();

	static constexpr EntityType entityType = EntityType::GAMETE;
	EntityType getEntityType() const override { return entityType; }
	glm::vec3 getWorldTransform() const override;
	aabb getAABB() const override;

	// deserialize a Gamete from the stream and add it to the world
	static void deserialize(BinaryStream &stream);

#ifdef DEBUG_DRAW_GAMETE
	FunctionalityFlags getFunctionalityFlags() const override { return
			FunctionalityFlags::DRAWABLE |
			FunctionalityFlags::UPDATABLE;
	}
#endif

	void update(float dt) override;
#ifdef DEBUG_DRAW_GAMETE
	void draw(RenderContext const& ctx) override;
#endif

	void serialize(BinaryStream &stream) const override;
	int getSerializationType() const override { return SerializationObjectTypes::GAMETE; }

	const Chromosome& getChromosome() const { return chromosome_; }

	unsigned generation_=0;  // the generation of the bug who spawned this gamete

protected:
	Chromosome chromosome_;
	PhysicsBody body_;
	int updateSkipCounter_ = 0;

	void onCollision(PhysicsBody* pOther, float impulse);

private:
	static Entity* getEntityFromGametePhysBody(PhysicsBody const& body);
};

#endif /* ENTITIES_GAMETE_H_ */
