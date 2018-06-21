/*
 * ribosome.cpp
 *
 *  Created on: Jun 20, 2018
 *      Author: bog
 */

#define ASSERTDBG_ENABLE
#include <boglfw/utils/rand.h>
#include <boglfw/math/math3D.h>

#include <easyunit/test.h>
#include <sstream>

#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

template <typename T, class PredT>
int getVMSNearestObjectIndex(std::vector<std::pair<T, float>> const& nerves, float matchCoord,
		PredT validatePred, int start=0, int end=-1) {
	if (nerves.size() == 0)
		return -1;
	if (end == -1)
		end = nerves.size() - 1;
	// binary-search the nearest object that satisfies validatePred:
	assert(start <= end);
	if (start == end) {
		if (validatePred(nerves[start].first))
			return start;
		else
			return -1;
	}
	unsigned pivot = (end-start) / 2 + start;
	if (matchCoord == nerves[pivot].second && validatePred(nerves[pivot].first))
		return pivot;	// exact match!
	// if we got here, there's no perfect match
	int iBegin, iEnd, iDir;
	if (matchCoord > nerves[pivot].second) { // look into the big interval
		iBegin = pivot+1;
		iEnd = end;
		iDir = -1;
	} else {
		// look into the small interval
		iBegin = start;
		iEnd = pivot;
		iDir = +1;
	}
	int intervalMatch = getVMSNearestObjectIndex(nerves, matchCoord, validatePred, iBegin, iEnd);
	float intervalDelta = 1e20;
	if (intervalMatch != -1)
		intervalDelta = abs(matchCoord - nerves[intervalMatch].second);
	// go back from the pivot one by one and get the closest match and compare it to the intervalMatch
	int otherSearch = pivot + (iDir > 0 ? iDir : 0);
	while (otherSearch >= start && otherSearch <= end && !validatePred(nerves[otherSearch].first))
		otherSearch += iDir;
	if (otherSearch >= start && otherSearch <= end) {
		// we found something in the other interval as well, let's see which is closer
		float otherDelta = abs(matchCoord - nerves[otherSearch].second);
		if (otherDelta < intervalDelta)
			return otherSearch;
		else
			return intervalMatch;
	} else if (intervalMatch != -1)
		return intervalMatch;
	else
		return -1;
}


using namespace easyunit;

//static float flterr = 1.e-5f;	// accepted float error in comparisons

TEST(ribosome, getVMSNearestObjectIndex) {
	std::vector<std::pair<int, float>> neurons {
		{1, -20.5f},
		{1, -0.5f},
		{2, 2.f},
		{1, 2.1f},
		{1, 100.f}
	};
	auto pred = [](int const& i) { return i == 1; };
	int index = getVMSNearestObjectIndex<int>(neurons, 2.f, pred);
	ASSERT_EQUALS(3, index);

	auto pred1 = [](int const& i) { return true; };
	index = getVMSNearestObjectIndex<int>(neurons, 2.f, pred1);
	ASSERT_EQUALS(2, index);

	auto pred2 = [](int const& i) { return i%2; };
	for (int i=0; i<100; i++) {
		neurons.clear();
		for (int n=randi(200), j=0; j<n; j++) {
			neurons.emplace_back(randi(10), srandf() * 100);
		}
		std::sort(neurons.begin(), neurons.end(), [](auto &n1, auto &n2) {
			return n1.second < n2.second;
		});
		float coord = srandf() * 100;
		index = getVMSNearestObjectIndex(neurons, coord, pred2);

		// now search with a traditional method and check if the results are the same
		int kMin = -1;
		float dMin = INF;
		for (int k=0; k<neurons.size(); k++) {
			float dCrt = abs(coord - neurons[k].second);
			if (dCrt < dMin && pred2(neurons[k].first)) {
				dMin = dCrt;
				kMin = k;
			}
		}
		if (kMin != index) {
			std::cout << 'n=' << neurons.size() << "\n";
			for (int k=0; k<neurons.size(); k++)
				std::cout << "\t" << k << ": { " << neurons[k].first << ", " << neurons[k].second << " }\n";
			std::cout << "coord: " << coord << "\n";
			std::cout << "getVMSNearest: " << index << ";   kMin: " << kMin << "\n";
			index = getVMSNearestObjectIndex(neurons, coord, pred2);
		}
		ASSERT_EQUALS(kMin, index);
	}
}
