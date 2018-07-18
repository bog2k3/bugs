/*
 * GenomeFitness.h
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#ifndef BUGS_GENERATOR_GENOMEFITNESS_H_
#define BUGS_GENERATOR_GENOMEFITNESS_H_

class Bug;

// computes the fitness of a genome based on characteristics of its phenotype expression, such as the presence of important body parts
class GenomeFitness {
public:
	static float compute(Bug const& b);
};

#endif /* BUGS_GENERATOR_GENOMEFITNESS_H_ */
