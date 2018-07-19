/*
 * Researcher.h
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#ifndef BUGS_GENERATOR_RESEARCHER_H_
#define BUGS_GENERATOR_RESEARCHER_H_

#include "../genetics/Genome.h"

#include <string>
#include <vector>

// this class manages and coordinates the genome research process
class Researcher {
public:
	Researcher(std::string genomesPath);
	~Researcher() {}

	void saveGenomes();

	// load genomes, set target population and recombinationRatio - the fraction of targetPopulation that will be filled
	// with new genomes created by recombining previous generation genomes (the rest will be prev gen genomes wich are simply mutated)
	void initialize(int targetPopulation, float recombinationRatio, int motorSampleFrames, int randomGenomeLength);

	// perform a research iteration
	void iterate(float timeStep);

	// print stats from the current session
	void printStatistics();

private:
	std::string genomesPath_;
	std::vector<std::pair<Genome, float>> genomes_;
	uint targetPopulation_ = 20;
	float recombinationRatio_ = 0.25f;
	uint motorSampleFrames_ = 500;
	uint randomGenomeLength_ = 200;

	void loadGenomes();
	void fillUpPopulation();
	decltype(genomes_) doRecombination();
	void selectBest(decltype(genomes_) &out);
	uint biasedRandomSelect(); // select a genome randomly, but biased by the relative fitnesses towards the best

	void printIterationStats();
};

#endif /* BUGS_GENERATOR_RESEARCHER_H_ */
