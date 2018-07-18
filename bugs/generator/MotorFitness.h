/*
 * MotorFitness.h
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#ifndef BUGS_GENERATOR_MOTORFITNESS_H_
#define BUGS_GENERATOR_MOTORFITNESS_H_

class Bug;

// computes a fitness score for a motor command signal based on its characteristics and resemblance to a "real" signal
class MotorFitness {
public:
	static float compute(Bug const& b, int nIterations, float timeStep);
};

#endif /* BUGS_GENERATOR_MOTORFITNESS_H_ */
