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

//#define DEBUG_TIME_FACTOR_SCORE

struct IterationStats {
	std::vector<float> fitness;
	unsigned newRandom = 0;
	unsigned recombinationTarget = 0;
	unsigned averageGenomeLength = 0;
	float averageFitness = 0;
	std::vector<std::pair<unsigned, unsigned>> recombinationPairs;
	std::vector<unsigned> selected;
	float duration_s = 0;
#ifdef DEBUG_TIME_FACTOR_SCORE
	std::vector<std::pair<int, float>> decodeTimeFactors;	// first: decodeTime delta percent; second: score
#endif
};

struct Fitness {
	float fenotypeFitness = 0;
	float functionalFitness = 0;
	float decodeTimeFitness = 0;
	float genomeLengthFitenss = 0;
	float relativeFitness = 0;		// this is obtained by comparing with other instances - the more comparisons it wins, the higher the relativeFitness

	bool operator<(Fitness const& x) const {
		if (fenotypeFitness < x.fenotypeFitness)
			return true;
		else if (fenotypeFitness == x.fenotypeFitness) {
			if (functionalFitness < x.functionalFitness)
				return true;
			else if (functionalFitness == x.functionalFitness) {
				if (genomeLengthFitenss < x.genomeLengthFitenss)
					return true;
				else if (genomeLengthFitenss == x.genomeLengthFitenss)
					return decodeTimeFitness < x.decodeTimeFitness;
			}
		}
		return false;
	}
	bool operator==(Fitness const& x) const {
		return fenotypeFitness == x.fenotypeFitness
				&& functionalFitness == x.functionalFitness
				&& decodeTimeFitness == x.decodeTimeFitness
				&& genomeLengthFitenss == x.genomeLengthFitenss;
	}
	bool operator>(Fitness const& x) const {
		return !operator<(x) && !operator==(x);
	}

	float overalFitness(bool scaled) const {
		return (fenotypeFitness + functionalFitness) * (scaled ? 1.f : (decodeTimeFitness * genomeLengthFitenss));
	}
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
	std::vector<std::pair<Genome, Fitness>> genomes_;
	unsigned targetPopulation_ = 20;
	float recombinationRatio_ = 0.25f;
	float renewRatio_ = 0.05f;
	unsigned motorSampleFrames_ = 500;
	unsigned randomGenomeLength_ = 200;

	std::vector<IterationStats> stats_;
	unsigned statWriteIndex_ = 0;
	unsigned fullStatsOffset_ = 0;
	bool disableStats_ = false;

	void loadGenomes();
	void fillUpPopulation();
	decltype(genomes_) doRecombination();
	void selectBest(decltype(genomes_) &out);
	unsigned biasedRandomSelect(float steepness, std::set<unsigned> exclude); // select a genome randomly, but biased by the relative fitnesses towards the best

	void printIterationStats();
};

#endif /* BUGS_GENERATOR_RESEARCHER_H_ */
