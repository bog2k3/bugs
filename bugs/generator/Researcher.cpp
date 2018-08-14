/*
 * Researcher.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "Researcher.h"
#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyConst.h"
#include "../genetics/Ribosome.h"
#include "../serialization/GenomeSerialization.h"
#include "GenomeGenerator.h"
#include "GenomeFitness.h"
#include "MotorFitness.h"

#include <boglfw/World.h>
#include <boglfw/utils/parallel.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/utils/filesystem.h>
#include <boglfw/serialization/BinaryStream.h>

#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>

Researcher::Researcher(std::string genomesPath)
	: genomesPath_(genomesPath)
{
}

void Researcher::saveGenomes() {
	for (unsigned i=0; i<genomes_.size(); i++) {
		auto &g = genomes_[i];
		std::stringstream ss;
		ss << genomesPath_ + "/genome-" << i << ".dat";
		BinaryStream str(sizeof(g));
		str << g.second << g.first;

		std::ofstream f(ss.str(), std::ios::binary);
		f.write((char*)str.getBuffer(), str.getSize());
	}
}

// load genomes, set target population and recombinationRatio - the fraction of targetPopulation that will be filled
// with new genomes created by recombining previous generation genomes (the rest will be prev gen genomes which are simply mutated)
void Researcher::initialize(int targetPopulation, float recombinationRatio, float renewRatio, int motorSampleFrames, int randomGenomeLength) {
	targetPopulation_ = targetPopulation;
	recombinationRatio_ = recombinationRatio;
	renewRatio_ = renewRatio;
	motorSampleFrames_ = motorSampleFrames;
	randomGenomeLength_ = randomGenomeLength;
	loadGenomes();
	fillUpPopulation();
}

// perform a research iteration
void Researcher::iterate(float timeStep) {
	static unsigned iterationNumber = 1;
	LOGNP("\n");
	LOGLN("RESEARCH ITERATION " << iterationNumber++ << " -----------------------------------------------------------------------------------------------\n");
	std::chrono::time_point<std::chrono::high_resolution_clock> itStartTime(std::chrono::high_resolution_clock::now());
	stats_.push_back({});
	MTVector<std::pair<Bug*, float>> bugs(genomes_.size());

//#define SINGLE_THREAD

#ifdef SINGLE_THREAD
	std::for_each(
#else
	parallel_for(
#endif
			genomes_.begin(), genomes_.end(),
#ifndef SINGLE_THREAD
			Infrastructure::getThreadPool(),
#endif
			[this, timeStep, &bugs](auto &gp)
	{
		Genome& g = gp.first;
		Bug* b = new Bug(g, BodyConst::initialEggMass*2, {0,0}, {0,0}, 0);
		b->getRibosome()->setResearchModeOn();
		std::chrono::time_point<std::chrono::high_resolution_clock> decodeGenomeStart(std::chrono::high_resolution_clock::now());
		while (b->isInEmbryonicDevelopment() && !b->getRibosome()->isPreFinalStep())
			b->update(1.f);	// use 1 second step to bypass gene decode frequency delay in ribosome
		std::chrono::time_point<std::chrono::high_resolution_clock> decodeGenomeEnd(std::chrono::high_resolution_clock::now());
		float decodeTime = std::chrono::nanoseconds(decodeGenomeEnd - decodeGenomeStart).count();

		bugs.push_back({b, decodeTime});

	});
	// execute deferred tasks
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);
	for (auto p : bugs)
		p.first->update(1.f); // last update to finalize ribosome stuff
	// another deferred tasks execution:
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);

	// compute min/max genome decode times to use later for fitness scaling
	float maxDecodeTime = 0;
	float minDecodeTime = 1e20;
	for (auto &p : bugs) {
		if (p.second > maxDecodeTime)
			maxDecodeTime = p.second;
		if (p.second < minDecodeTime)
			minDecodeTime = p.second;
	}

	// compute fitnesses
	MTVector<decltype(genomes_)::value_type> updatedGenomes(genomes_.size());
#ifdef SINGLE_THREAD
	std::for_each(
#else
	parallel_for(
#endif
			bugs.begin(), bugs.end(),
#ifndef SINGLE_THREAD
			Infrastructure::getThreadPool(),
#endif
			[this, &updatedGenomes, &timeStep, &minDecodeTime, &maxDecodeTime] (auto &p)
	{
		auto b = p.first;
		float fitness = 0;
		fitness += GenomeFitness::compute(*b);
		fitness += MotorFitness::compute(*b, motorSampleFrames_, timeStep);
		fitness *= GenomeFitness::genomeLengthFactor(*b);
		// scale fitness by decode time factor;
		// decode time factor is 1 for lowest decode time, minTimeFactor for highest decode time, in an inverse logarithmic shape
		float minTimeFactor = 0.5f;
		float r = 1 / log(maxDecodeTime-minDecodeTime + 1);
		float decodeTimeFactor = 1.f / (1 + r * log(p.second - minDecodeTime + 1));
		assertDbg(decodeTimeFactor >= minTimeFactor * 0.95 && decodeTimeFactor <= 1.05);
		fitness *= decodeTimeFactor;
		updatedGenomes.push_back({b->getGenome(), fitness});
		// kill bug
		b->kill();
	});
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0); // performed queued die events and stuff

	for (auto b : bugs) {
		b.first->destroy();
	}
	bugs.clear();
	// allow world to clean up entities
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);

	// copy back the genomes with the updated fitness scores
	genomes_.clear();
	for (auto &g : updatedGenomes)
		genomes_.push_back(g);

	// sort by decreasing fitness
	std::sort(genomes_.begin(), genomes_.end(), [](auto &g1, auto &g2) {
		return g1.second > g2.second;
	});

	for (auto &g : genomes_)
		stats_.back().fitness.push_back(g.second);

	// try to recombine recombinationRatio_ * targetPopulation_ matching genome pairs
	auto newGenomes = doRecombination();

	// select best genomes until targetPopulation_ is reached again
	selectBest(newGenomes);

	genomes_.swap(newGenomes);

	stats_.back().newRandom = targetPopulation_ - genomes_.size();
	// refresh population by adding some new random genomes
	fillUpPopulation();

	std::chrono::time_point<std::chrono::high_resolution_clock> itEndTime(std::chrono::high_resolution_clock::now());
	stats_.back().duration_s = std::chrono::nanoseconds(itEndTime - itStartTime).count() * 1.e-9;

	printIterationStats();

	const int autosaveInterval = 20; // iterations
	if (!(iterationNumber % autosaveInterval)) {
		LOGLN("Autosaving (interval=" << autosaveInterval << ") ...");
		saveGenomes();
		LOGLN("Autosave done.");
	}
}

void Researcher::loadGenomes() {
	// load as many genomes as needed or available from genomesPath_ directory
	LOGLN("Loading genomes from: " << genomesPath_ << " ...");
	int nLoaded = 0;
	for (unsigned i=0; i<targetPopulation_; i++) {
		std::stringstream ss;
		ss << genomesPath_ + "/genome-" << i << ".dat";
		auto fn = ss.str();
		if (!filesystem::pathExists(fn))
			continue;
		// load from file fn
		std::ifstream f(fn, std::ios::binary);
		BinaryStream str(f);
		float fitness;
		Genome g;
		str >> fitness >> g;

		genomes_.push_back({g, fitness});

		nLoaded++;

		if (genomes_.size() == targetPopulation_)
			break;
	}
	LOGLN("Loaded " << nLoaded << " genomes.");
}

void Researcher::fillUpPopulation() {
	// generate random genomes until reaching the target population
	while (genomes_.size() < targetPopulation_)
		genomes_.push_back({GenomeGenerator::createRandom(randomGenomeLength_), 0.f});
}

decltype(Researcher::genomes_) Researcher::doRecombination() {
	decltype(genomes_) newGenomes;
	unsigned required = max(1.f, recombinationRatio_ * targetPopulation_);
	unsigned tries = 0;
	unsigned maxTries = genomes_.size();

	stats_.back().recombinationTarget = required;

	while (tries++ <= maxTries && newGenomes.size() < required) {
		unsigned i1 = biasedRandomSelect(1.f, {});
		unsigned i2 = biasedRandomSelect(1.f, {});

		Chromosome c1 = GeneticOperations::meyosis(genomes_[i1].first);
		Chromosome c2 = GeneticOperations::meyosis(genomes_[i2].first);

		if (c1.isGeneticallyCompatible(c2)) {
			newGenomes.push_back({Genome{c1, c2}, 0});

			stats_.back().recombinationPairs.push_back({i1, i2});
		}
	}

	return newGenomes;
}

void Researcher::selectBest(decltype(genomes_) &out) {
	std::set<unsigned> selected;
	unsigned roomForNew = max(1.f, renewRatio_ * targetPopulation_);
	while (out.size() < targetPopulation_ - roomForNew) {
		unsigned index = biasedRandomSelect(1.f, selected);
		selected.insert(index);
		out.push_back(genomes_[index]);
		genomes_[index].second = 0; // in order to avoid duplicates
		// perform mutations on half of the genomes:
		if (randf() < 0.5) {
			GeneticOperations::alterChromosome(out.back().first.first);
			GeneticOperations::alterChromosome(out.back().first.second);
		}

		stats_.back().selected.push_back(index);
	}
}

// smaller than 1.0 values are more likely to also select from the ones with smaller fitnesses
// greater than 1.0 values are more likely to only select the highest fitnesses
unsigned Researcher::biasedRandomSelect(float steepness, std::set<unsigned> exclude) {
	// each genome gets a chance to be selected proportional to its fitness raised to "steepness" power
	// normalize chances to make them sum up to 1.0
	double total = 0;
	for (unsigned i=0; i<genomes_.size(); i++) {
		if (exclude.find(i) == exclude.end())
			total += pow(genomes_[i].second, steepness);
	}
	double dice = randd() * total;
	double floor = 0;
	unsigned selected = genomes_.size();
	for (unsigned i=0; i<genomes_.size(); i++) {
		if (exclude.find(i) != exclude.end())
			continue;
		auto &g = genomes_[i];
		if (dice - floor <= pow(g.second, steepness)) {
			selected = i;
			break;
		}
		floor += pow(g.second, steepness);
	}
	assertDbg(selected < genomes_.size());
	return selected;
}

void Researcher::printIterationStats() {
	LOG("Iteration best fitnesses: ");
	unsigned n=10;
	unsigned printed = 0;
	for (unsigned i=0; i<n && i<stats_.back().fitness.size(); i++) {
		if (stats_.back().fitness[i] == 0)
			break;
		printed++;
		LOGNP(stats_.back().fitness[i] << ",   ");
	}
	if (!printed)
		LOGNP("nothing yet...");
	LOGNP("\n");
	LOGLN("Recombined " << stats_.back().recombinationPairs.size() << " out of " << stats_.back().recombinationTarget << " pairs of genomes:");
	LOG("\t");
	for (auto &p : stats_.back().recombinationPairs) {
		LOGNP(p.first << " (f: " << stats_.back().fitness[p.first] << ") x " <<
				p.second << " (f: " << stats_.back().fitness[p.second] << ");  ");
	}
	LOGNP("\n");
	LOG("Selected genomes for next generation: ");
	for (auto i : stats_.back().selected)
		LOGNP(i << " (f: " << stats_.back().fitness[i] << "); ");
	LOGNP("\n");
	LOGLN("Fresh new genomes for next generation: " << stats_.back().newRandom);

	float avgFitness = std::accumulate(stats_.back().fitness.begin(), stats_.back().fitness.end(), 0.f);
	avgFitness /= stats_.back().fitness.size();
	LOGLN("Average iteration fitness: " << avgFitness);
	LOGLN("Iteration duration: " << stats_.back().duration_s << " [s]");
}

void Researcher::printStatistics() {
	// TODO ...
}
