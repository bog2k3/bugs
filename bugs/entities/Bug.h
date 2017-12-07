/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include "enttypes.h"
#include "Bug/LifetimeSensor.h"
#include "../genetics/Genome.h"
#include "../genetics/GeneDefinitions.h"
#include "../genetics/CummulativeValue.h"
#include "../serialization/objectTypes.h"

#include <boglfw/entities/Entity.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/bitFlags.h>
#include <boglfw/math/aabb.h>

#include <glm/fwd.hpp>

#include <vector>
#include <map>
#include <atomic>


class NeuralNet;
class InputSocket;
class Ribosome;
class Torso;
class ZygoteShell;
class EggLayer;
class BodyPart;

class Bug : public Entity {
public:

	struct defaultConstants {
		static constexpr float lifetimeSensor_vmsCoord = 500;
	};

	explicit Bug(Genome const &genome, float zygoteMass, glm::vec2 position, glm::vec2 velocity, unsigned generation);
	virtual ~Bug();
	FunctionalityFlags getFunctionalityFlags() const override { return
			FunctionalityFlags::UPDATABLE |
			FunctionalityFlags::DRAWABLE |
			FunctionalityFlags::SERIALIZABLE;
	}
	static constexpr EntityType entityType = EntityType::BUG;
	virtual EntityType getEntityType() const override { return entityType; }
	glm::vec3 getWorldTransform() const override;
	aabb getAABB() const override;

	// deserialize a Bug from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) override;

	void update(float dt) override;
	void draw(RenderContext const &ctx) override;
	int getSerializationType() override { return SerializationObjectTypes::BUG; }

	const Genome& getGenome() { return genome_; }
	glm::vec2 getVelocity();
	float getMass();
	unsigned getGeneration() { return generation_; }
	bool isAlive() { return isAlive_; }
	float getNeuronData(int neuronIndex);
	Torso* getBody() { return body_; }

	void kill();

	/**
	 * creates a new basic bug out of a default genome
	 */
	static Bug* newBasicBug(glm::vec2 position);
	/**
	 * creates a mutant descendant from the default bug genome
	 */
	static Bug* newBasicMutantBug(glm::vec2 position);

	static int getPopulationCount() { return population.load(std::memory_order_relaxed); }
	static int getZygotesCount() { return freeZygotes.load(std::memory_order_relaxed); }
	static int getMaxGeneration() { return maxGeneration.load(std::memory_order_relaxed); }
	uint64_t getId() { return id; }

protected:
	Bug(Bug const& orig) = delete; // no implementation because no usage

	Genome genome_;
	NeuralNet* neuralNet_;

	// motor nerves mapped by the indices representing the order in which they are created from the genome - motor line indices match these.
	// 'first' is the inputSocket of the motor, 'second' is the outputSocket of the neuron connected to it.
	std::map<int, std::pair<InputSocket*, OutputSocket*>> motorLines_;

	// default sensors (arguments represent VMS coordinates):
	LifetimeSensor lifeTimeSensor_ { defaultConstants::lifetimeSensor_vmsCoord };

	Ribosome* ribosome_;
	bool isAlive_;
	bool isDeveloping_;
	float tRibosomeStep_; // time since last ribosome step
	Torso* body_;
	ZygoteShell* zygoteShell_;
	UpdateList bodyPartsUpdateList_;
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
	static std::atomic<int> population;
	static std::atomic<int> freeZygotes;
	static std::atomic<int> maxGeneration;

	friend class Ribosome;

	void updateEmbryonicDevelopment(float dt);
	void updateDeadDecaying(float dt);
	void onFoodProcessed(float mass);
	void onMotorLinesDetached(std::vector<unsigned> const& lines);
	void fixAllGeneValues();
	static Chromosome createBasicChromosome();

private:
	static std::atomic<uint64_t> nextId;
	uint64_t id = nextId++;

	mutable aabb cachedAABB_;
	mutable glm::vec3 cachedWorldTransform_;
};

#endif /* ENTITIES_BUG_H_ */
