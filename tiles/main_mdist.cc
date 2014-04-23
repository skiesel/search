// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "mdist.hpp"
#include "mdistlandmark.hpp"
#include "mdistlandmarkgenerator.hpp"
#include "../search/main.hpp"
#include "../search/landmarkbasedsearch.hpp"
#include <cstdio>

static SearchAlgorithm<TilesMdist> *get(int, const char *[]);

void landmarkBasedSearch(TilesMdist& d, int argc, const char *argv[]);

int main(int argc, const char *argv[]) {
	dfheader(stdout);

	FILE *lvl = stdin;
	const char *lvlpath = "";
	for (int i = 0; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			lvlpath = argv[++i];
	}

	if (lvlpath[0] != '\0') {
		lvl = fopen(lvlpath, "r");
		if (!lvl)
			fatalx(errno, "Failed to open %s for reading", lvlpath);
	}

	TilesMdist d(lvl);

	if (lvlpath[0] != '\0') {
		dfpair(stdout, "level", "%s", lvlpath);
		fclose(lvl);
 	}

	if (strcmp(argv[1], "landmarkbasedsearch") == 0) {
                landmarkBasedSearch(d, argc, argv);
        } else {
		searchGet<TilesMdist>(get, d, argc, argv);
	}


	dffooter(stdout);
	return 0;
}

static SearchAlgorithm<TilesMdist> *get(int argc, const char *argv[]) {
	return getsearch<TilesMdist>(argc, argv);
}

void landmarkBasedSearch(TilesMdist& d, int argc, const char *argv[]) {
	LandmarkBasedSearch<TilesMdist, TilesMdistLandmark> *srch =
		new LandmarkBasedSearch<TilesMdist, TilesMdistLandmark>(argc, argv);

	unsigned int decompSize = d.Width;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-decompsize") == 0) {
			decompSize = strtol(argv[++i], NULL, 10);
		}
	}

        srch->setLandmarkGenerator(new TilesMdistLandmarkGenerator(d, decompSize));

	typename TilesMdist::State s0 = d.initialstate();
	dfpair(stdout, "initial heuristic", "%f", (double) d.h(s0));
	dfpair(stdout, "initial distance", "%f", (double) d.d(s0));
	dfpair(stdout, "algorithm", argv[1]);
	dfpair(stdout, "landmarks", "%u", decompSize);

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
