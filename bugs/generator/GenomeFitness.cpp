/*
 * GenomeFitness.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "GenomeFitness.h"
#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyPart.h"

float GenomeFitness::compute(Bug const& b) {
	float fitness = 0;
	// +3 for first egglayer,
	// +3 for first mouth,
	// +1 for each of first 2 noses,
	// +1 for each of first 2 bones
	// +1 for each of first 2 pivot joints
	// +1 for each of first 2 grippers
	int noses = 0;
	int bones = 0;
	int joints = 0;
	int grippers = 0;
	bool hasMouth = false;
	bool hasEggLayer = false;
	auto &bparts = b.getBodyParts();
	if (bparts.size() == 0)
		return 0;
	bparts[0]->applyPredicateGraph([&](auto b) {
		switch (b->getType()) {
		case BodyPartType::EGGLAYER:
			if (!hasEggLayer) {
				fitness += 3;
				hasEggLayer = true;
			}
			break;
		case BodyPartType::MOUTH:
			if (!hasMouth) {
				fitness += 3;
				hasMouth = true;
			}
			break;
		case BodyPartType::SENSOR_PROXIMITY:
			if (noses++ < 2)			// +1 for first nose, +1 for second, and none for anything more
				fitness += 1;
			break;
		case BodyPartType::BONE:
			if (bones++ < 2)
				fitness += 1;
			break;
		case BodyPartType::JOINT_PIVOT:
			if (joints++ < 2)
				fitness += 1;
			break;
		case BodyPartType::GRIPPER:
			if (grippers++ < 2)
				fitness += 1;
			break;
		default:
			break;
		}
		return false;
	});
	float fatRatio = b.getTotalFatMass() / b.getTotalMass();
	float idealFatRatio = 0.33f;
	float fatFitnessScore = 0;
	// max score when fatRatio is ideal, less if it's greater or smaller
	if (fatRatio > idealFatRatio)
		fatFitnessScore = 1.f - sqr((fatRatio - idealFatRatio) / (1.f - idealFatRatio));
	else
		fatFitnessScore = sqrt(fatRatio / idealFatRatio);

	fitness += fatFitnessScore;

	// fitness decreases when bug has too many body parts, as few as possible that do the job is best
	float lowestPartScore = -1.f;
	float highestPartsScore = 1.f;
	int highScoreThreshold = 30; // up to this number score is highest
	int zeroScoreThreshold = 100; // at this number score is zero
	int lowScoreThreshold = 200; // at this number and above score is lowest

	int nBodyParts = bparts.size();
	float partsScore;
	if (nBodyParts <= highScoreThreshold)
		partsScore = highestPartsScore;
	else if (nBodyParts <= zeroScoreThreshold)
		partsScore = highestPartsScore * (float)(zeroScoreThreshold - nBodyParts) / (zeroScoreThreshold - highScoreThreshold);
	else if (nBodyParts <= lowScoreThreshold)
		partsScore = lowestPartScore * (float)(nBodyParts - zeroScoreThreshold) / (lowScoreThreshold - zeroScoreThreshold);
	else
		partsScore = lowestPartScore;

	assert(partsScore >= lowestPartScore - 0.05 && partsScore <= highestPartsScore + 0.05);

	return fitness + partsScore;
}

float GenomeFitness::genomeLengthFactor(Bug const& b) {
	// fitness decreases when bug has too many genes, as few as possible that do the job is best
	float lowestGCoef = 0.5f;
	float highestGCoef = 1.f;
	int minLengthThresh = 250; // up to this number of genes there's no penalty
	int maxLengthThresh = 1000; // where coef is reduced to lowestGCoef, above this no further penalty
	int nGenes1 = clamp((int)b.getGenome().first.genes.size(), minLengthThresh, maxLengthThresh);
	int nGenes2 = clamp((int)b.getGenome().second.genes.size(), minLengthThresh, maxLengthThresh);

	float geneCoef1 = lowestGCoef + (1 - (float)(nGenes1-minLengthThresh) / (maxLengthThresh - minLengthThresh)) * (highestGCoef - lowestGCoef);
	float geneCoef2 = lowestGCoef + (1 - (float)(nGenes2-minLengthThresh) / (maxLengthThresh - minLengthThresh)) * (highestGCoef - lowestGCoef);
	float geneCoef = (geneCoef1 + geneCoef2) / 2;
	assert(geneCoef >= lowestGCoef*0.95 && geneCoef <= highestGCoef*1.05);

	return geneCoef;
}
