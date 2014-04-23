#pragma once
#include "../search/landmarkgenerator.hpp"
#include "mdist.hpp"
#include "mdistlandmark.hpp"

class TilesMdistLandmarkGenerator : public LandmarkGenerator<TilesMdist, TilesMdistLandmark> {
public:
        TilesMdistLandmarkGenerator(TilesMdist &baseMdist, unsigned int numLandmarks) :
                baseMdist(baseMdist), numLandmarks(numLandmarks) {}

        TilesMdistLandmark getLandmark(TilesMdist &domain, TilesMdist::State &state,
				       unsigned int landmarkNum) {
                assert(landmarkNum < numLandmarks);

		unsigned int biggest = domain.Width > domain.Height ? domain.Width : domain.Height;
                unsigned int ls =  biggest / landmarkNum;

                if(numLandmarks == (landmarkNum+1))
			ls = biggest;
                return TilesMdistLandmark(domain, state, ls);
        }

        unsigned int getLandmarkCount() const {
                return numLandmarks;
        }

private:
        TilesMdist &baseMdist;
        unsigned int numLandmarks;
};

