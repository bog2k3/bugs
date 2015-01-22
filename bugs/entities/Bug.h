/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include "Entity.h"
#include "../genetics/Genome.h"
#include "../genetics/GeneDefinitions.h"
#include "../utils/UpdateList.h"
#include "../genetics/CummulativeValue.h"
#include <glm/fwd.hpp>
#include <vector>
#include <map>
#include "Bug/LifetimeSensor.h"

class ISensor;
class IMotor;
class NeuralNet;
class Ribosome;
class Torso;
class ZygoteShell;

class Bug : public Entity {
public:
	explicit Bug(Genome const &genome, float zygoteSize, glm::vec2 position);
	virtual ~Bug();
	FunctionalityFlags getFunctionalityFlags() override {
		return FF_UPDATABLE | FF_DRAWABLE;
	}

	void update(float dt) override;
	void draw(RenderContext const &ctx) override;

	const Genome& getGenome() { return genome_; }

	/**
	 * creates a new basic bug out of a default genome
	 */
	static Bug* newBasicBug(glm::vec2 position);

protected:
	Genome genome_;
	std::vector<ISensor*> sensors_;
	std::vector<IMotor*> motors_;
	NeuralNet* neuralNet_;
	Ribosome* ribosome_;
	bool isAlive_;
	bool isDeveloping_;
	float tRibosomeStep_; // time since last ribosome step
	Torso* body_;
	ZygoteShell* zygoteShell_;
	UpdateList bodyPartsUpdateList_;
	LifetimeSensor lifeTimeSensor_;

	std::map<gene_body_attribute_type, CummulativeValue*> mapBodyAttributes_;
	CummulativeValue initialFatMassRatio_;
	CummulativeValue minFatMasRatio_;
	CummulativeValue adultLeanMass_;

	friend class Ribosome;

	void updateEmbryonicDevelopment(float dt);
	void updateDeadDecaying(float dt);
};

#endif /* ENTITIES_BUG_H_ */
