/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include <glm/fwd.hpp>
#include <vector>
#include "../genetics/Genome.h"
#include "../updatable.h"
#include "../UpdateList.h"

class ISensor;
class IMotor;
class NeuralNet;
class Ribosome;
class Torso;
class ZygoteShell;

class Bug {
public:
	explicit Bug(Genome const &genome, float zygoteSize, glm::vec2 position);
	virtual ~Bug();

	void update(float dt);

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
	float energy_;
	float scale_;
	float scaledEnergy_;
	Torso* body_;
	ZygoteShell* zygoteShell_;
	UpdateList bodyPartsUpdateList_;

	friend class Ribosome;
};

template<> void update(Bug*& b, float dt);

#endif /* ENTITIES_BUG_H_ */
