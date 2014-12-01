/*
 * Bug.h
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#ifndef ENTITIES_BUG_H_
#define ENTITIES_BUG_H_

#include "../genetics/Chromosome.h"

#include <list>

class ISensor;
class IMotor;
class NeuralNet;

class Bug {
public:
	explicit Bug(Genome genome);
	virtual ~Bug();

	const Genome& getGenome() { return genome; }

protected:
	Genome genome;
	std::list<ISensor*> sensors;
	std::list<IMotor*> motors;
	NeuralNet* neuralNet;

	friend class Ribosome;
};

#endif /* ENTITIES_BUG_H_ */
