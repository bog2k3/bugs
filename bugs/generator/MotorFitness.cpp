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
//#include <boglfw/utils/log.h>

struct signalGenerator {
	float phase = 0;		// [rad]
	float freq = 0;			// [Hz]
	float amp = 0;			// [*]
	float noiseThresh = 0;	// [fraction] this is scaled by amplitude so 0.1 means 10% noise and 1.0 means 100% noise added to the signal

	signalGenerator() = default;

	signalGenerator(float freq, float amp, float noiseThresh)
		: freq(freq), amp(amp), noiseThresh(noiseThresh) {
		phase = randf() * 2*PI;
	}

	float next(float dt) {
		phase += freq * 2 * PI * dt;
		float noise = srandf() * amp * noiseThresh;
		return sinf(phase) * amp + noise;
	}
};

// Discrete Fourier Transform
/* the index of each output sample represents a signal frequency given by:
 * 		i (index) = number of periods (repetitions) of that signal that exist in the input data
 * 		given n = number of samples in the input data
 * 		and f = sample frequency (1 / duration of each sample)
 * 		we have:
 * 			Ti = n/i * 1/f		(period of component i)
 * 			fi =  i * f / n		(frequency of component i)
 * the amplitude of each component is computed as such:
 * 		amp(i) = 2 * sqrt(sqr(outR[i]) + sqr(outI[i])) / n
 * 		that is twice the modulus of the complex number from output at index i divided by the number of samples
 *
 * !! ONLY THE FIRST HALF of the output values is significant because of Nyquist limit !!
 * (the second half is a mirror of the first half)
 */
void dft(float inreal[], float outreal[], float outimag[], int n) {
    for (int k = 0; k < n; k++) {  // For each output element
        double sumreal = 0;
        double sumimag = 0;
        for (int t = 0; t < n; t++) {  // For each input element
            double angle = 2 * PI * t * k / n;
            sumreal +=  inreal[t] * cosf(angle);
            sumimag += -inreal[t] * sinf(angle);
        }
        outreal[k] = sumreal;
        outimag[k] = sumimag;
    }
}

static float computeSignalScore(std::vector<float> samples) {
	float outR[samples.size()];
	float outI[samples.size()];

	dft(&samples[0], outR, outI, samples.size());

	int nAmps = samples.size()/2; // up to the Nyquist limit
	float amps[nAmps];
	float amp_avg = 0;
	float amp_max = 0;
	for (int i=0; i<nAmps; i++) {
		// skip #0 which represents the continuous component:
		amps[i] = 2.f * sqrt(sqr(outR[i+1]) + sqr(outI[i+1])) / samples.size();
		amp_avg += amps[i];
		if (amp_max < amps[i])
			amp_max = amps[i];
	}
	amp_avg /= nAmps;

	// count how many significant components there are (amp > 5*amp_avg)
	int nSignif = 0;
	float ampRatios = 0; // cumulated ratios of significant components amplitude to max amplitude
	for (int i=0; i<nAmps; i++) {
		if (amps[i] > 5*amp_avg) {
			nSignif++;
			ampRatios += amps[i] / amp_max;
		}
	}
//	LOGLN("found " << nSignif << " significant components with total ratio: " << ampRatios);
	// max score if number of significant components is between [1, compThreshLow]
	// from (compThreshLow to compThreshHigh] score decreases
	// above compThreshHigh score is zero
	float compThreshLow = 5;
	float compThreshHigh = 9;
	float factor = nSignif == 0 ? 0 : (
			nSignif <= compThreshLow ? 1 : (
					nSignif > compThreshHigh ? 0 : (
							(compThreshHigh - nSignif) / (compThreshHigh - compThreshLow)
							)
					)
			);

	float score = ampRatios * factor;
	return score;
}

/*#include <fstream>
int testDFT() {
	float amp = 4.f;
	int signalPeriods = 20;
	unsigned nSamples = 200;
	float sigFreq = (float)signalPeriods / nSamples;
	signalGenerator g(sigFreq, amp, 0.0);
	std::vector<float> samples;
	for (int i=0; i<nSamples; i++)
		samples.push_back(g.next(1.f));
	float outR[nSamples];
	float outI[nSamples];
	dft(&samples[0], outR, outI, nSamples);

	std::ofstream f("test-dft.csv");
	f << "virtTime,signal,dft_R,dft_I\n";
	for (unsigned i=0; i<samples.size(); i++) {
		f << (float)i/nSamples << "," << samples[i] << "," << outR[i] << "," << outI[i] << "\n";
	}
	return 0;
}

int dummy = testDFT();*/

float MotorFitness::compute(Bug const& b, int nIterations, float timeStep) {
	float maxFreq = 0.5f / timeStep;	// nyquist frequency for sample rate
	float minFreq = maxFreq / 50;
	float minAmp = 0.05f;
	float maxAmp = 5.f;
	float minNoiseThresh = 0.1;
	float maxNoiseThresh = 0.4f;

//#define USE_COMPASSES
//#define USE_EYES

	// populate lists of inputs and outputs
	std::vector<std::pair<OutputSocket*, signalGenerator>> sensorOutputs;
	std::vector<std::pair<InputSocket*, std::vector<float>>> motorInputs;
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
			sensorOutputs.push_back({static_cast<const Compass*>(p)->getOutputSocket(0), signalGenerator{}});
#endif
			break;
		case BodyPartType::SENSOR_PROXIMITY:
			for (unsigned i=0; i<static_cast<const Nose*>(p)->getOutputCount(); i++)
				sensorOutputs.push_back({static_cast<const Nose*>(p)->getOutputSocket(i), signalGenerator{}});
			break;
		case BodyPartType::SENSOR_SIGHT:
#ifdef USE_EYES
			for (unsigned i=0; i<static_cast<const Eye*>(p)->getOutputCount(); i++)
				sensorOutputs.push_back({static_cast<const Eye*>(p)->getOutputSocket(i), signalGenerator{}});
#endif
			break;
		default:
			break;
		}
		return false;
	});

	if (sensorOutputs.size() == 0 || motorInputs.size() == 0)
		return 0;

	const int nSimulations = min(5, nIterations);
	nIterations /= nSimulations;
	float totalScore = 0;
	for (int iS = 0; iS<nSimulations; iS++) {
		// initialize the signal generators every simulation to get different signals
		for (auto &s : sensorOutputs) {
			s.second = signalGenerator(minFreq + randf() * (maxFreq - minFreq),
					minAmp + randf() * (maxAmp - minAmp),
					minNoiseThresh + randf() * (maxNoiseThresh - minNoiseThresh));
		}
		// now run the simulation and collect data
		for (int i=0; i<nIterations; i++) {
			for (auto &pair : sensorOutputs)
				pair.first->push_value(pair.second.next(timeStep));
			b.neuralNet()->iterate(timeStep);
			for (auto &pair : motorInputs)
				pair.second.push_back(pair.first->value);
		}

		// compute motor signal score
		//OGLN("-------------------------- motor fitness analysis START -----------------------");
		float score = 1;	// we start with something non-zero because at least there are motors and sensors
		for (auto &pair : motorInputs) {
			score += computeSignalScore(pair.second);
		}
		// average score:
		score /= motorInputs.size();
		//LOGLN("total motor score: " << score);
		//LOGLN("-------------------------- motor fitness analysis END -----------------------");
		totalScore += score;
	}
	totalScore /= nSimulations;

	// scale factor: more motors (up to a max number) give a higher score
	size_t maxMotors = 10;
	float scale = min(maxMotors, motorInputs.size());

	float fitness = totalScore * (log(scale)+1);
	return fitness;
}
