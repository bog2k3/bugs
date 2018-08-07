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
#include <set>

struct IterationStats {
	std::vector<float> fitness;
	unsigned newRandom = 0;
	unsigned recombinationTarget = 0;
	std::vector<std::pair<unsigned, unsigned>> recombinationPairs;
	std::vector<unsigned> selected;
};

// this class manages and coordinates the genome research process
class Researcher {
public:
	Researcher(std::string genomesPath);
	~Researcher() {}

	void saveGenomes();

	// load genomes, set target population and recombinationRatio - the fraction of targetPopulation that will be filled
	// renewRatio - fraction of targetPopulation that will be filled with fresh new random genomes every iteration
	// with new genomes created by recombining previous generation genomes (the rest will be prev gen genomes wich are simply mutated)
	void initialize(int targetPopulation, float recombinationRatio, float renewRatio, int motorSampleFrames, int randomGenomeLength);

	// perform a research iteration
	void iterate(float timeStep);

	// print stats from the current session
	void printStatistics();

private:
	std::string genomesPath_;
	std::vector<std::pair<Genome, float>> genomes_;
	unsigned targetPopulation_ = 20;
	float recombinationRatio_ = 0.25f;
	float renewRatio_ = 0.05f;
	unsigned motorSampleFrames_ = 500;
	unsigned randomGenomeLength_ = 200;

	std::vector<IterationStats> stats_;

	void loadGenomes();
	void fillUpPopulation();
	decltype(genomes_) doRecombination();
	void selectBest(decltype(genomes_) &out);
	unsigned biasedRandomSelect(float steepness, std::set<unsigned> exclude); // select a genome randomly, but biased by the relative fitnesses towards the best

	void printIterationStats();
};

#endif /* BUGS_GENERATOR_RESEARCHER_H_ */
