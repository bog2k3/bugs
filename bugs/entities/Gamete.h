/*
 * Gamete.h
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#ifndef ENTITIES_GAMETE_H_
#define ENTITIES_GAMETE_H_

#include "Entity.h"
#include "../genetics/Genome.h"
#include "../PhysicsBody.h"
#include "../serialization/objectTypes.h"

#define DEBUG_DRAW_GAMETE

class Gamete: public Entity {
public:
	Gamete(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass);
	virtual ~Gamete();

	// deserialize a Gamete from the stream and add it to the world
	static void deserialize(BinaryStream &stream);

#ifdef DEBUG_DRAW_GAMETE
	FunctionalityFlags getFunctionalityFlags() override { return FF_DRAWABLE | FF_UPDATABLE; }
#endif

	void update(float dt) override;
#ifdef DEBUG_DRAW_GAMETE
	void draw(RenderContext const& ctx) override;
#endif

	void serialize(BinaryStream &stream) override;
	SerializationObjectTypes getSerializationType() override { return SerializationObjectTypes::GAMETE; }

	const Chromosome& getChromosome() const { return chromosome_; }

protected:
	Chromosome chromosome_;
	PhysicsBody body_;
	int updateSkipCounter_ = 0;

	void onCollision(PhysicsBody* pOther, float impulse);
};

#endif /* ENTITIES_GAMETE_H_ */
