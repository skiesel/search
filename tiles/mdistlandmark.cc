// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "mdistlandmark.hpp"
#include <cstdlib>
#include <limits>
#include <vector>
#include <iostream>
TilesMdistLandmark::TilesMdistLandmark(TilesMdist& d, TilesMdist::State &s,
					 unsigned int howManyLs) : howManyLs(howManyLs) {

	int minRow = Width - howManyLs;
	if(minRow < 0) {
		minRow = 0;
	}

	int minCol = Height - howManyLs;
	if(minCol < 0) {
		minCol = 0;
	}

	for(unsigned int i = 0; i < Ntiles; i++) {
		init[i] = s.ts[i];
		goalpos[i] = d.goalpos[i];
		for(unsigned int j = 0; j < Ntiles; j++) {
			md[i][j] = d.md[i][j];
		}
		unsigned int row = i / Width;
		unsigned int col = i % Width;
		if(row >= minRow || (row < minRow && col >= minCol)) {
			tilesToConsider[i] = true;
		} else {
			tilesToConsider[i] = false;
		}
	}
}

TilesMdistLandmark::State TilesMdistLandmark::initialstate() {
	State s;
	s.h = 0;
	for (unsigned int i = 0; i < Ntiles; i++) {
		if (init[i] == 0)
			s.b = i;
		else if(tilesToConsider[init[i]])
			s.h += md[init[i]][i];
		s.ts[i] = init[i];
	}
	return s;
}

TilesMdistLandmark::Cost TilesMdistLandmark::computeMdist(const State& state) const {
	Cost h = 0;
	for (unsigned int i = 0; i < Ntiles; i++) {
		if (state.ts[i] != 0 && tilesToConsider[state.ts[i]]) {
			h += md[state.ts[i]][i];
		}
	}
	return h;
}

TilesMdistLandmark::Cost TilesMdistLandmark::pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {
	State state = initialstate();
	Cost cost(0);

	if (ops.size() > (unsigned int) std::numeric_limits<int>::max())
		fatal("Too many actions");

	for (int i = ops.size() - 1; i >= 0; i--) {
		State copy(state);
		Edge e(*this, copy, ops[i]);
		cost += e.cost;
		assert(e.state == path[i]);
		state = e.state;
	}
	assert (isgoal(state));
	return cost;
}
