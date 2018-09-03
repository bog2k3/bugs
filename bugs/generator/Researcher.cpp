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

static const char fullStatsFilename[] = "stats.csv";
static const char sessionStatsFilename[] = "stats.last-session.csv";

Researcher::Researcher(std::string genomesPath)
	: genomesPath_(genomesPath)
{
}

void Researcher::saveGenomes() {
	// sort genomes by decreasing fitness:
	std::sort(genomes_.begin(), genomes_.end(), [](auto &g1, auto &g2) {
		return g1.second > g2.second;
	});
	// now write them to disk:
	for (unsigned i=0; i<genomes_.size(); i++) {
		auto &g = genomes_[i];
		std::stringstream ss;
		ss << genomesPath_ + "/genome-" << i << ".dat";
		size_t data_size = dataSize(g.first) + sizeof(float);
		BinaryStream str(data_size);
		str << g.second.overalFitness(false) << g.first;
		assertDbg(str.size() == data_size);

		std::ofstream f(ss.str(), std::ios::binary);
		f.write((char*)str.getBuffer(), str.size());
		f.close();
#ifdef DEBUG
		auto fileSize = filesystem::getFileSize(ss.str());
		assertDbg(fileSize == data_size);
#endif
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

	// make sure the working directory exists:
	if (!filesystem::pathExists(genomesPath_)) {
		if (!filesystem::mkDirRecursive(genomesPath_)) {
			ERROR("!!! Could not create genomes directory: " << genomesPath_);
			ERROR("!!! Genomes and statistics will not be saved!");
			disableStats_ = true;
			return;
		}
	}

	// initialize stats file:
	std::ofstream f(genomesPath_ + "/" + sessionStatsFilename);
	f << "Iteration #,Best Fitness,Avg Fitness,Avg Genome Length,Duration\n";
	f.close();

	bool existingFullStats = false;
	if (filesystem::pathExists(genomesPath_ + "/" + fullStatsFilename)) {
		std::ifstream f(genomesPath_ + "/" + fullStatsFilename);
		std::string junk;
		while (std::getline(f, junk))
			fullStatsOffset_++;
		if (fullStatsOffset_ > 0) {
			existingFullStats = true;
			fullStatsOffset_--; // don't count the header line
		}
	}
	if (!existingFullStats) {
		filesystem::copyFile(genomesPath_ + "/" + sessionStatsFilename, genomesPath_ + "/" + fullStatsFilename);
	}
}

// perform a research iteration
void Researcher::iterate(float timeStep) {
	static unsigned iterationNumber = 1;
	LOGNP("\n");
	LOGLN("RESEARCH ITERATION " << iterationNumber << " / " << iterationNumber+fullStatsOffset_ << " -----------------------------------------------------------------------------------------------\n");
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
		unsigned decodeIterations = 0;
		while (b->isInEmbryonicDevelopment() && !b->getRibosome()->isPreFinalStep()) {
			b->update(1.f);	// use 1 second step to bypass gene decode frequency delay in ribosome
			decodeIterations++;
		}
		float decodeTime = decodeIterations;

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
	// decode time factor is 1 for lowest decode time, minTimeFactor for highest decode time, in an inverse logarithmic shape
	// if maxTime is 10 times smallTime, factor for maxTime is 0.75:
	float minDurationFactor = 1/(1+0.33f*log(maxDecodeTime/minDecodeTime)/log(10));
	assertDbg(minDurationFactor > 0 && minDurationFactor <= 1);
	float durationFactorSlope = (1.f / minDurationFactor - 1) / log(maxDecodeTime-minDecodeTime + 1);

#ifdef DEBUG_TIME_FACTOR_SCORE
	MTVector<decltype(IterationStats::decodeTimeFactors)::value_type> decodeTimeFactors(genomes_.size());
#endif

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
			[this, &updatedGenomes, timeStep, minDecodeTime, minDurationFactor, durationFactorSlope
#ifdef DEBUG_TIME_FACTOR_SCORE
			 , &decodeTimeFactors
#endif
			 ] (auto &p)
	{
		auto b = p.first;
		Fitness fitness;
		fitness.fenotypeFitness = GenomeFitness::compute(*b);
		fitness.functionalFitness = MotorFitness::compute(*b, motorSampleFrames_, timeStep);
		fitness.genomeLengthFitenss = GenomeFitness::genomeLengthFactor(*b);
		// scale fitness by decode time factor;
		float decodeTimeFactor = 1.f / (1 + durationFactorSlope * log(p.second - minDecodeTime + 1));
		assertDbg(decodeTimeFactor >= minDurationFactor * 0.95 && decodeTimeFactor <= 1.05);
		fitness.decodeTimeFitness = decodeTimeFactor;
#ifdef DEBUG_TIME_FACTOR_SCORE
		decodeTimeFactors.push_back({(p.second-minDecodeTime) / minDecodeTime * 100, decodeTimeFactor});	// first: percentage of minDecodeTime spent above minDecodeTime
#endif
		updatedGenomes.push_back({b->getGenome(), fitness});
		// kill bug
		b->kill();
	});
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0); // performed queued die events and stuff

	for (auto b : bugs) {
		stats_.back().averageGenomeLength += b.first->getGenome().first.genes.size();
		b.first->destroy();
	}
	stats_.back().averageGenomeLength /= bugs.size();
	bugs.clear();
	// allow world to clean up entities
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);

	// copy back the genomes with the updated fitness scores
	genomes_.clear();
	for (auto &g : updatedGenomes)
		genomes_.push_back(g);

	// compute relative fitnesses (by comparing all with all)
	// this is O(N^2), not very nice, but simplifies the randomSelect a lot
	for (unsigned i=0; i<genomes_.size(); i++)
		for (unsigned j=i+1; j<genomes_.size(); j++) {
			if (genomes_[i].second > genomes_[j].second)
				genomes_[i].second.relativeFitness++;
			else if (genomes_[j].second > genomes_[i].second)
				genomes_[j].second.relativeFitness++;
		}

	// sort by decreasing fitness
	std::sort(genomes_.begin(), genomes_.end(), [](auto &g1, auto &g2) {
		return g1.second.relativeFitness > g2.second.relativeFitness;
	});

	for (auto &g : genomes_)
		stats_.back().fitness.push_back(g.second.overalFitness(false));

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

#ifdef DEBUG_TIME_FACTOR_SCORE
	stats_.back().decodeTimeFactors.reserve(decodeTimeFactors.size());
	std::copy(decodeTimeFactors.begin(), decodeTimeFactors.end(), std::back_inserter(stats_.back().decodeTimeFactors));
	std::sort(stats_.back().decodeTimeFactors.begin(), stats_.back().decodeTimeFactors.end(), [](auto &x ,auto &y) {
		return x.second > y.second;
	});
#endif

	printIterationStats();

	const int autosaveInterval = 20; // iterations
	if (!(iterationNumber % autosaveInterval)) {
		LOGLN("Autosaving (interval=" << autosaveInterval << ") ...");
		saveGenomes();
		LOGLN("Writing stats to file...");
		printStatistics();
		statWriteIndex_ = stats_.size();
		LOGLN("Autosave done.");
	}

	iterationNumber++;
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

		genomes_.push_back({g, {fitness, 0, 0, 0}});

		nLoaded++;

		if (genomes_.size() == targetPopulation_)
			break;
	}
	LOGLN("Loaded " << nLoaded << " genomes.");
}

void Researcher::fillUpPopulation() {
	// generate random genomes until reaching the target population
	while (genomes_.size() < targetPopulation_) {
		int length = max(randomGenomeLength_, stats_.size() ? stats_.back().averageGenomeLength : 1);
		genomes_.push_back({GenomeGenerator::createRandom(length), {}});
	}
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
			Genome g{c1, c2};
			newGenomes.push_back({g, {}});

			stats_.back().recombinationPairs.push_back({i1, i2});
		}
	}

	return newGenomes;
}

void Researcher::selectBest(decltype(genomes_) &out) {
	std::set<unsigned> exclude;
	unsigned roomForNew = max(1.f, renewRatio_ * targetPopulation_);
	while (out.size() < targetPopulation_ - roomForNew) {
		unsigned index = biasedRandomSelect(2.f, exclude);
		exclude.insert(index);
		out.push_back(genomes_[index]);
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
			total += pow(genomes_[i].second.relativeFitness, steepness);
	}
	double dice = randd() * total;
	double floor = 0;
	unsigned selected = genomes_.size();
	for (unsigned i=0; i<genomes_.size(); i++) {
		if (exclude.find(i) != exclude.end())
			continue;
		auto &g = genomes_[i];
		if (dice - floor <= pow(g.second.relativeFitness, steepness)) {
			selected = i;
			break;
		}
		floor += pow(g.second.relativeFitness, steepness);
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
	LOG("Selected genomes for next generation (" << stats_.back().selected.size() << "): ");
	for (auto i : stats_.back().selected)
		LOGNP(i << " (f: " << stats_.back().fitness[i] << "); ");
	LOGNP("\n");
	LOGLN("Fresh new genomes for next generation: " << stats_.back().newRandom);

	stats_.back().averageFitness = std::accumulate(stats_.back().fitness.begin(), stats_.back().fitness.end(), 0.f);
	stats_.back().averageFitness /= stats_.back().fitness.size();
	LOGLN("Average iteration fitness: " << stats_.back().averageFitness);
	LOGLN("Average iteration genome length: " << stats_.back().averageGenomeLength);
#ifdef DEBUG_TIME_FACTOR_SCORE
	LOG("DecodeTime scores: ");
	for (unsigned i=0; i<3 && i<stats_.back().decodeTimeFactors.size(); i++) {
		LOGNP("" << stats_.back().decodeTimeFactors[i].second << " (+" << stats_.back().decodeTimeFactors[i].first << "%), ");
	}
	if (stats_.back().decodeTimeFactors.size() > 6)
		LOGNP("....., ");
	for (unsigned i=stats_.back().decodeTimeFactors.size()-3; i<stats_.back().decodeTimeFactors.size(); i++) {
		LOGNP("" << stats_.back().decodeTimeFactors[i].second << " (+" << stats_.back().decodeTimeFactors[i].first << "%), ");
	}
	LOGNP("END\n");
#endif
	LOGLN("Iteration duration: " << stats_.back().duration_s << " [s]");
}

void Researcher::printStatistics() {
	if (disableStats_) {
		ERROR("Stats cannot be written to file!!!");
		return;
	}
	std::ofstream ff(genomesPath_ + "/" + fullStatsFilename, std::ios_base::app);
	std::ofstream fs(genomesPath_ + "/" + sessionStatsFilename, std::ios_base::app);
	for (unsigned i=statWriteIndex_; i<stats_.size(); i++) {
		fs << i+1 << "," << stats_[i].fitness[0]
			   << "," << stats_[i].averageFitness
			   << "," << stats_[i].averageGenomeLength
			   << "," << stats_[i].duration_s << "\n";
		ff << i+1+fullStatsOffset_ << "," << stats_[i].fitness[0]
			   << "," << stats_[i].averageFitness
			   << "," << stats_[i].averageGenomeLength
			   << "," << stats_[i].duration_s << "\n";
	}
	fs.close();
	ff.close();
}
