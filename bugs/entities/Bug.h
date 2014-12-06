/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include <list>
#include "../genetics/Genome.h"

class ISensor;
class IMotor;
class NeuralNet;
class Ribosome;

class Bug {
public:
	explicit Bug(Genome const &genome, float zygoteSize);
	virtual ~Bug();

	void update(float dt);

	const Genome& getGenome() { return genome; }

	/**
	 * creates a new basic bug out of a default genome
	 */
	static Bug* newBasicBug();

protected:
	Genome genome;
	std::list<ISensor*> sensors;
	std::list<IMotor*> motors;
	NeuralNet* neuralNet;
	Ribosome* ribosome;
	bool isAlive;
	bool isDeveloping;
	float tRibosomeStep; // time since last ribosome step
	float energy;
	float scale;
	float scaledEnergy;

	friend class Ribosome;
};

#endif /* ENTITIES_BUG_H_ */
