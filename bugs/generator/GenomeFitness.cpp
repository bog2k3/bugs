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
	// +3 for egglayer, +3 for mouth, +1 for each nose
	int noses = 0;
	bool hasMouth = false;
	bool hasEggLayer = false;
	auto &bparts = b.getBodyParts();
	if (bparts.size() == 0)
		return 0;
	int fatCells = 0;
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
			if (noses++ < 2) {			// +1 for first nose, +1 for second, and none for anything more
				fitness += 1;
			}
			break;
		case BodyPartType::FAT:
			fatCells++;
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
	float lowestCoef = 0.2f;	// 20%
	float highestCoef = 1.f;	// 100%
	int minPartsThreshold = 30; // up to this number there is no penalty (fitness 100%)
	int maxPartsThreshold = 100; // this is where fitness is reduced to 20%, above this it stays at 20%
	int nBodyPartsClamped = clamp((int)bparts.size(), minPartsThreshold, maxPartsThreshold);

	float coef = lowestCoef + (1 - nBodyPartsClamped / (maxPartsThreshold - minPartsThreshold)) * (highestCoef - lowestCoef);
	assert(coef >= lowestCoef*0.95 && coef <= highestCoef*1.05);

	return fitness * coef;
}
