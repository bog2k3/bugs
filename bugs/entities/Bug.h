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

	const Genome& getGenome() { return genome; }

	/**
	 * creates a new basic bug out of a default genome
	 */
	static Bug* newBasicBug(glm::vec2 position);

protected:
	Genome genome;
	std::vector<ISensor*> sensors;
	std::vector<IMotor*> motors;
	NeuralNet* neuralNet;
	Ribosome* ribosome;
	bool isAlive;
	bool isDeveloping;
	float tRibosomeStep; // time since last ribosome step
	float energy;
	float scale;
	float scaledEnergy;
	Torso* body;
	ZygoteShell* zygoteShell;

	friend class Ribosome;
};

#endif /* ENTITIES_BUG_H_ */
