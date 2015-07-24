#pragma once

#include "search.hpp"
#include "landmarkgenerator.hpp"
#include "astar.hpp"

template <class D, class L> struct LandmarkBasedSearch : public SearchAlgorithm<D> {

	LandmarkBasedSearch(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), landmarkGenerator(NULL) {
		for (int i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-subalgorithm") == 0) {
				i++;
				if(strcmp(argv[i], "astar") == 0) subSearch = new Astar<L>(argc, argv);
				else fatal("unrecognized subalgorithm: %s", argv[i]);
			}
		}
	}

	~LandmarkBasedSearch() {}

	void setLandmarkGenerator(LandmarkGenerator<D,L> *lg) {
		landmarkGenerator = lg;
	}

	void search(D &domain, typename D::State &start) {
		assert(landmarkGenerator != NULL);

		this->start();

		typename D::State currentState = start;

		for(unsigned int i = 0; i < landmarkGenerator->getLandmarkCount(); i++) {

			L landmark = landmarkGenerator->getLandmark(currentState, i);
			typename L::State startState = landmark.initialstate();

			fprintf(stderr, "landmark %u\n", i);
			fprintf(stderr, "heuristic %u\n", landmark.h(startState));

			subSearch->search(landmark, startState);
			currentState = landmark.toBaseDomain(startState);
			domain.dumpstate(stdout, currentState);
			landmark.dumpstate(stdout, startState);
			currentState = landmark.toBaseDomain(subSearch->res.path.front());
			domain.dumpstate(stdout, currentState);

			if(domain.isgoal(currentState))
				break;

			subSearch->reset();
		}

		this->finish();
	}

	SearchAlgorithm<L> *subSearch;
	LandmarkGenerator<D,L> *landmarkGenerator;
};
