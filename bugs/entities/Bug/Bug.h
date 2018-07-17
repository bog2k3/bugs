/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include "../enttypes.h"
#include "../../genetics/Genome.h"
#include "../../genetics/GeneDefinitions.h"
#include "../../genetics/CumulativeValue.h"
#include "../../serialization/objectTypes.h"
#include "../../body-parts/BodyPartContext.h"
#include "../../neuralnet/OutputSocket.h"

#include <boglfw/entities/Entity.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/bitFlags.h>
#include <boglfw/math/aabb.h>

#include <glm/fwd.hpp>

#include <vector>
#include <map>
#include <atomic>
#include <functional>
#include <set>

class NeuralNet;
class Neuron;
class InputSocket;
class Ribosome;
class ZygoteShell;
class EggLayer;
class BodyPart;
class Joint;

class Bug : public Entity {
public:
	Bug(Genome const &genome, float zygoteMass, glm::vec2 position, glm::vec2 velocity, unsigned generation);
	virtual ~Bug() override;
	FunctionalityFlags getFunctionalityFlags() const override { return
			FunctionalityFlags::UPDATABLE |
			FunctionalityFlags::DRAWABLE |
			FunctionalityFlags::SERIALIZABLE;
	}
	static constexpr EntityType entityType = EntityType::BUG;
	virtual EntityType getEntityType() const override { return entityType; }
//	glm::vec3 getWorldTransform() const override;
	aabb getAABB(bool requirePrecise=false) const override;

	// deserialize a Bug from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) const override;

	void update(float dt) override;
	void draw(Viewport* vp) override;
	int getSerializationType() const override { return SerializationObjectTypes::BUG; }

	const Genome& getGenome() const { return genome_; }
	//glm::vec2 getVelocity() const;
	float getMass() const;
	unsigned getGeneration() const { return generation_; }
	bool isAlive() const { return isAlive_; }
	bool isInEmbryonicDevelopment() const { return isDeveloping_; }
	float getNeuronValue(int neuronIndex) const;
//	Torso* getBody() { return body_; }
	float getTotalFatMass() const;
	float getTotalMass() const;
	float getLeanMass() const { return getTotalMass() - getTotalFatMass(); }

	void consumeEnergy(float amount);
	void onFoodProcessed(float mass);

	// destroys a joint and recreates it; this is required after connected body parts get scaled
	void recreateJoint(Joint* j) { jointsToRecreate_.insert(j); }

	void kill();

	uint64_t getId() { return id; }

#ifdef DEBUG
	float getDebugValue(std::string const name);
#endif

	/**
	 * creates a new basic bug out of a default genome
	 */
	static Bug* newBasicBug(glm::vec2 position);
	/**
	 * creates a mutant from the default bug genome
	 */
	static Bug* newBasicMutantBug(glm::vec2 position);

	static int getPopulationCount() { return population.load(std::memory_order_relaxed); }
	static int getZygotesCount() { return freeZygotes.load(std::memory_order_relaxed); }
	static int getMaxGeneration() { return maxGeneration.load(std::memory_order_relaxed); }

protected:
	Bug(Bug const& orig) = delete; // no implementation because no usage

	Genome genome_;
	NeuralNet* neuralNet_;

	std::map<Joint*, std::vector<std::pair<OutputSocket*, InputSocket*>>> jointSynapses_;
	std::map<BodyPart*, std::vector<Neuron*>> bodyPartNeurons_;

	OutputSocket lifetimeOutput_;
	float lifeTime_ = 0;

	// motor nerves mapped by the indices representing the order in which they are created from the genome - motor line indices match these.
	// 'first' is the inputSocket of the motor, 'second' is the outputSocket of the neuron connected to it.
//	std::map<int, std::pair<InputSocket*, OutputSocket*>> motorLines_;

	Ribosome* ribosome_;
	bool isAlive_;
	bool isDeveloping_;
	float tRibosomeStep_; // time since last ribosome step
//	Torso* body_;
	ZygoteShell* zygoteShell_;
	std::vector<BodyPart*> bodyParts_;
	UpdateList bodyPartsUpdateList_;
	float growthMassBuffer_;	// stores processed food to be used for growth (at the speed dictated by genes)
	float maxGrowthMassBuffer_;	// max growth buffer capacity (depends on max growth speed)
	float cachedLeanMass_;		// lean body mass cached; stored here for reasons of float precision
	bool cachedMassDirty_;		// flag to signal that cachedLeanMass_ must be recomputed
	std::vector<EggLayer*> eggLayers_;
	std::vector<BodyPart*> deadBodyParts_;
	std::set<Joint*> jointsToRecreate_;
	BodyPartContext context_;

	std::map<gene_body_attribute_type, CumulativeValue*> mapBodyAttributes_;
//	CumulativeValue initialFatMassRatio_;
	CumulativeValue minFatMasRatio_;
	CumulativeValue adultLeanMass_;
	CumulativeValue growthSpeed_;
	CumulativeValue reproductiveMassRatio_;
	CumulativeValue developmentMassThreshRatio_;
	CumulativeValue eggMass_;

	uint generation_=0;  // the generation this bug represents
	static std::atomic<uint> population;
	static std::atomic<uint> freeZygotes;
	static std::atomic<uint> maxGeneration;

	friend class Ribosome;

	void updateEmbryonicDevelopment(float dt);
	void updateDeadDecaying(float dt);
	void onJointBreak(Joint* joint);
	void fixAllGeneValues();
	void killNeuron(Neuron* n);
	void breakSynapse(std::pair<OutputSocket*, InputSocket*> &syn);
	float computeNeuralNetFrameEnergy(float dt); // how much energy the neural net consumed in the time frame?

	static Chromosome createBasicChromosome();

#ifdef DEBUG
	std::map<std::string, std::function<float()>> debugValueGetters_;
	std::atomic<float> actualGrowthSpeed_ {0};
	std::atomic<float> eggGrowthSpeed_ {0};
	std::atomic<float> fatGrowthSpeed_ {0};
	std::atomic<float> frameFoodProcessed_ {0};
	std::atomic<float> frameEnergyUsed_ {0};
#endif

private:
	static std::atomic<uint64_t> nextId;
	uint64_t id = nextId++;

	mutable aabb cachedAABB_;
	mutable uint cachedAABBFramesOld_;
//	mutable glm::vec3 cachedWorldTransform_;
};

#endif /* ENTITIES_BUG_H_ */
