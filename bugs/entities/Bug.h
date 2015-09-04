/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include "Entity.h"
#include "enttypes.h"
#include "../genetics/Genome.h"
#include "../genetics/GeneDefinitions.h"
#include "../utils/UpdateList.h"
#include "../genetics/CummulativeValue.h"
#include <glm/fwd.hpp>
#include <vector>
#include <map>
#include "Bug/LifetimeSensor.h"
#include "../serialization/objectTypes.h"
#include "Bug/IMotor.h"

class ISensor;
class NeuralNet;
class Ribosome;
class Torso;
class ZygoteShell;
class EggLayer;
class BodyPart;

class Bug : public Entity {
public:
	explicit Bug(Genome const &genome, float zygoteMass, glm::vec2 position, glm::vec2 velocity, unsigned generation);
	virtual ~Bug();
	FunctionalityFlags getFunctionalityFlags() override {
		return FF_UPDATABLE | FF_DRAWABLE | FF_SERIALIZABLE;
	}
	static constexpr EntityType entityType = ENTITY_BUG;
	virtual EntityType getEntityType() override { return entityType; }

	// deserialize a Bug from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) override;

	void update(float dt) override;
	void draw(RenderContext const &ctx) override;
	SerializationObjectTypes getSerializationType() override { return SerializationObjectTypes::BUG; }

	const Genome& getGenome() { return genome_; }
	glm::vec2 getPosition();
	glm::vec2 getVelocity();
	float getMass();
	unsigned getGeneration() { return generation_; }
	bool isAlive() { return isAlive_; }

	void kill();

	/**
	 * creates a new basic bug out of a default genome
	 */
	static Bug* newBasicBug(glm::vec2 position);
	/**
	 * creates a mutant descendant from the default bug genome
	 */
	static Bug* newBasicMutantBug(glm::vec2 position);

	static unsigned getPopulationCount() { return population; }
	static unsigned getZygotesCount() { return freeZygotes; }
	static unsigned getMaxGeneration() { return maxGeneration; }

protected:
	Bug(Bug const& orig) = delete; // no implementation because no usage

	Genome genome_;
	std::vector<ISensor*> sensors_;		// holds pointers to sensors in the order they appear in the genome - sensor lines indices match these
	std::vector<IMotor*> motors_;		// holds pointers to motors in the order they appear in the genome - motor lines indices match these
	NeuralNet* neuralNet_;
	Ribosome* ribosome_;
	bool isAlive_;
	bool isDeveloping_;
	float tRibosomeStep_; // time since last ribosome step
	Torso* body_;
	ZygoteShell* zygoteShell_;
	UpdateList bodyPartsUpdateList_;
	LifetimeSensor lifeTimeSensor_;
	float growthMassBuffer_;	// stores processed food to be used for growth (at the speed dictated by genes)
	float maxGrowthMassBuffer_;	// max growth buffer capacity (depends on max growth speed)
	float cachedLeanMass_;		// lean body mass cached; stored here for reasons of float precision
	bool cachedMassDirty_;		// flag to signal that cachedLeanMass_ must be recomputed
	std::vector<EggLayer*> eggLayers_;
	std::vector<BodyPart*> deadBodyParts_;

	std::map<gene_body_attribute_type, CummulativeValue*> mapBodyAttributes_;
	CummulativeValue initialFatMassRatio_;
	CummulativeValue minFatMasRatio_;
	CummulativeValue adultLeanMass_;
	CummulativeValue growthSpeed_;
	CummulativeValue reproductiveMassRatio_;
	CummulativeValue eggMass_;

	unsigned generation_=0;  // the generation this bug represents
	static unsigned population;
	static unsigned freeZygotes;
	static unsigned maxGeneration;

	friend class Ribosome;

	void updateEmbryonicDevelopment(float dt);
	void updateDeadDecaying(float dt);
	void onFoodProcessed(float mass);
	void onMotorLinesDetached(std::vector<int> const& lines);
	void fixAllGeneValues();
	static Chromosome createBasicChromosome();
};

#endif /* ENTITIES_BUG_H_ */
