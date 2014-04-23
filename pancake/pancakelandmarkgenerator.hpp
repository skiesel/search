#pragma once
#include "../search/landmarkgenerator.hpp"
#include "pancake.hpp"
#include "pancakelandmark.hpp"

class PancakeLandmarkGenerator : public LandmarkGenerator<Pancake, PancakeLandmark> {
public:
	PancakeLandmarkGenerator(Pancake &basePancake, unsigned int numLandmarks) :
		basePancake(basePancake), numLandmarks(numLandmarks) {}

	PancakeLandmark getLandmark(Pancake &domain, Pancake::State &state, unsigned int landmarkNum) {
		assert(landmarkNum < numLandmarks);

		unsigned int cakes = (basePancake.Ncakes / numLandmarks) * (landmarkNum + 1);

		if(numLandmarks == (landmarkNum+1))
			cakes = basePancake.Ncakes;
		return PancakeLandmark(state, basePancake.Ncakes - cakes);
	}

	unsigned int getLandmarkCount() const {
		return numLandmarks;
	}

private:
	Pancake &basePancake;
	unsigned int numLandmarks;
};
