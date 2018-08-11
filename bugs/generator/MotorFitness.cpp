/*
 * MotorFitness.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "MotorFitness.h"
#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyPart.h"
#include "../body-parts/Gripper.h"
#include "../body-parts/Muscle.h"
#include "../body-parts/sensors/Nose.h"
#include "../body-parts/sensors/Eye.h"
#include "../body-parts/sensors/Compass.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/InputSocket.h"
#include "../neuralnet/OutputSocket.h"

#include <boglfw/utils/rand.h>

struct signalGenerator {
	float phase = 0;		// [rad]
	float freq = 0;			// [Hz]
	float amp = 0;			// [*]
	float noiseThresh = 0;	// [fraction] this is scaled by amplitude so 0.1 means 10% noise and 1.0 means 100% noise added to the signal

	signalGenerator(float freq, float amp, float noiseThresh)
		: freq(freq), amp(amp), noiseThresh(noiseThresh) {
	}

	float next(float dt) {
		phase += freq * 2 * PI * dt;
		float noise = srandf() * amp * noiseThresh;
		return sinf(phase) * amp + noise;
	}
};

static float computeSignalScore(std::vector<float> samples) {
	return 0;
}

float MotorFitness::compute(Bug const& b, int nIterations, float timeStep) {
	std::vector<std::pair<OutputSocket*, signalGenerator>> sensorOutputs;
	std::vector<std::pair<InputSocket*, std::vector<float>>> motorInputs;

	float maxFreq = 0.5f / timeStep;	// nyquist frequency for sample rate
	float minFreq = maxFreq / 50;
	float minAmp = 0.05f;
	float maxAmp = 5.f;
	float minNoiseThresh = 0.1;
	float maxNoiseThresh = 0.7f;

//#define USE_COMPASSES
//#define USE_EYES

	// populate lists of inputs and outputs
	b.getBodyParts()[0]->applyPredicateGraph([&](auto p) {
		switch (p->getType()) {
		case BodyPartType::GRIPPER:
			motorInputs.push_back({static_cast<const Gripper*>(p)->getInputSocket(0), {}});
			break;
		case BodyPartType::MUSCLE:
			motorInputs.push_back({static_cast<const Muscle*>(p)->getInputSocket(0), {}});
			break;
		case BodyPartType::SENSOR_COMPASS:
#ifdef USE_COMPASSES
			sensorOutputs.push_back({static_cast<const Compass*>(p)->getOutputSocket(0),
				signalGenerator(minFreq + randf() * (maxFreq - minFreq),
							minAmp + randf() * (maxAmp - minAmp),
							minNoiseThresh + randf() * (maxNoiseThresh - minNoiseThresh))
				});
#endif
			break;
		case BodyPartType::SENSOR_PROXIMITY:
			for (unsigned i=0; i<static_cast<const Nose*>(p)->getOutputCount(); i++)
				sensorOutputs.push_back({static_cast<const Nose*>(p)->getOutputSocket(i),
					signalGenerator(minFreq + randf() * (maxFreq - minFreq),
							minAmp + randf() * (maxAmp - minAmp),
							minNoiseThresh + randf() * (maxNoiseThresh - minNoiseThresh))
				});
			break;
		case BodyPartType::SENSOR_SIGHT:
#ifdef USE_EYES
			for (unsigned i=0; i<static_cast<const Eye*>(p)->getOutputCount(); i++)
				sensorOutputs.push_back({static_cast<const Eye*>(p)->getOutputSocket(i)
					signalGenerator(minFreq + randf() * (maxFreq - minFreq),
							minAmp + randf() * (maxAmp - minAmp),
							minNoiseThresh + randf() * (maxNoiseThresh - minNoiseThresh))
				});
#endif
			break;
		default:
			break;
		}
		return false;
	});

	// now run the simulation and collect data
	for (int i=0; i<nIterations; i++) {
		for (auto &pair : sensorOutputs)
			pair.first->push_value(pair.second.next(timeStep));
		b.neuralNet()->iterate(timeStep);
		for (auto &pair : motorInputs)
			pair.second.push_back(pair.first->value);
	}

	// compute motor signal score
	float score = 0;
	for (auto &pair : motorInputs) {
		score += computeSignalScore(pair.second);
	}
	// average score:
	score /= motorInputs.size();

	// scale factor: more motors (up to a max number) give a higher score
	size_t maxMotors = 10;
	int scale = min(maxMotors, motorInputs.size());

	float fitness = score * scale;
	return fitness;
}
