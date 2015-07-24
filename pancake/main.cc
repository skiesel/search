// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "pancake.hpp"
#include "pancakelandmark.hpp"
#include "pancakelandmarkgenerator.hpp"
#include "../search/main.hpp"
#include "../search/landmarkbasedsearch.hpp"
#include <cstdio>

void landmarkBasedSearch(Pancake& d, int argc, const char *argv[]);

int main(int argc, const char *argv[]) {
	dfheader(stdout);
	Pancake d(stdin);
	if (strcmp(argv[1], "landmarkbasedsearch") == 0) {
		landmarkBasedSearch(d, argc, argv);
	} else {
		search<Pancake>(d, argc, argv);
	}
	dffooter(stdout);
	return 0;
}

void landmarkBasedSearch(Pancake& d, int argc, const char *argv[]) {
	LandmarkBasedSearch<Pancake, PancakeLandmark> *srch =
		new LandmarkBasedSearch<Pancake, PancakeLandmark>(argc, argv);

	unsigned int decompSize = d.Ncakes;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-decompsize") == 0) {
			decompSize = strtol(argv[++i], NULL, 10);
		}
	}

	srch->setLandmarkGenerator(new PancakeLandmarkGenerator(d, decompSize));

	typename Pancake::State s0 = d.initialstate();
	dfpair(stdout, "initial heuristic", "%f", (double) d.h(s0));
	dfpair(stdout, "initial distance", "%f", (double) d.d(s0));
	dfpair(stdout, "algorithm", argv[1]);

	try {
		srch->search(d, s0);
	} catch (std::bad_alloc&) {
		dfpair(stdout, "out of memory", "%s", "true");
		srch->res.path.clear();
		srch->res.ops.clear();
		srch->finish();
	}
	if (srch->res.path.size() > 0) {
		dfpair(stdout, "final sol cost", "%f",
			(double) d.pathcost(srch->res.path, srch->res.ops));
	} else {
		dfpair(stdout, "final sol cost", "%f", -1.0);
	}
	srch->output(stdout);

	delete srch;
}

