/*
 * GenomeGenerator.h
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#ifndef BUGS_GENERATOR_GENOMEGENERATOR_H_
#define BUGS_GENERATOR_GENOMEGENERATOR_H_

#include "../genetics/Genome.h"

// generates random genomes
class GenomeGenerator {
public:
	static Genome createRandom();
};

#endif /* BUGS_GENERATOR_GENOMEGENERATOR_H_ */
