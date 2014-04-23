#pragma once

#include "search.hpp"
#include "landmarkgenerator.hpp"
#include "astar.hpp"
#include "greedy.hpp"


template <class D, class L> struct LandmarkBasedSearch : public SearchAlgorithm<D> {

	LandmarkBasedSearch(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), landmarkGenerator(NULL) {
		for (int i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-subalgorithm") == 0) {
				i++;
				if(strcmp(argv[i], "astar") == 0) subSearch = new Astar<L>(argc, argv);
				else if(strcmp(argv[i], "greedy") == 0) subSearch = new Greedy<L>(argc, argv);
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
		std::vector< std::vector<typename D::State> > path;
		std::vector< std::vector<typename D::Oper> > operators;

		for(unsigned int i = 0; i < landmarkGenerator->getLandmarkCount(); i++) {

			L landmark = landmarkGenerator->getLandmark(domain, currentState, i);
			typename L::State startState = landmark.initialstate();

//			fprintf(stderr, "landmark %u\n", i);
//			fprintf(stderr, "heuristic %u\n", landmark.h(startState));

			subSearch->search(landmark, startState);

			path.push_back(std::vector<typename D::State>());
			for(unsigned int j = 0; j < subSearch->res.path.size(); j++) {
				path.back().push_back(landmark.toBaseDomain(subSearch->res.path[j]));
			}

			operators.push_back(std::vector<typename D::Oper>());
			for(unsigned int j = 0; j < subSearch->res.ops.size(); j++)
				operators.back().push_back(subSearch->res.ops[j]);

			this->res.expd += subSearch->res.expd;
			this->res.gend += subSearch->res.gend;
			this->res.reopnd += subSearch->res.reopnd;
			this->res.dups += subSearch->res.dups;

			currentState = path.back().front();

			if(domain.isgoal(currentState))
				break;

			subSearch->reset();
		}

		for(int i = path.size()-1; i >= 0; i--) {
			for(int j = 0; j < path[i].size()-1; j++) {
				this->res.path.push_back(path[i][j]);
			}
		}

		for(int i = operators.size()-1; i >= 0; i--) {
			for(int j = 0; j < operators[i].size(); j++) {
				this->res.ops.push_back(operators[i][j]);
			}
		}

		this->finish();
	}

	SearchAlgorithm<L> *subSearch;
	LandmarkGenerator<D,L> *landmarkGenerator;
};
